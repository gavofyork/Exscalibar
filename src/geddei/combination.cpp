/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <qdatetime.h>
#include "qtask.h"
#include "rdtsc.h"

#include "domprocessor.h"
#include "combination.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

Combination::Combination(SubProcessor *x, SubProcessor *y) : SubProcessor(x->theType + "&" + y->theType), theX(x), theY(y), theResident(0)
{
}

Combination::~Combination()
{
	delete theX;
	delete theY;
}

void Combination::processChunks(const BufferDatas &in, BufferDatas &out, uint) const
{
	uint xc = (in[0].samples() - theX->theIn) / theX->theStep + 1;
	uint samplesNeeded = xc * theX->theOut;
	uint yc = (samplesNeeded - theY->theIn) / theY->theStep + 1;
	if (!theResident || theResident->samples() < samplesNeeded)
	{
		delete theResident;
		theResident = new BufferData(samplesNeeded * theInterScope, theInterScope);
	}
	BufferDatas d(1);
	d.setData(0, theResident);

	unsigned long long s = rdtsc();
	theX->processChunks(in, theY->isInplace() ? out : d, xc);
	m_totalTimeX += rdtscElapsed(s);

	s = rdtsc();
	theY->processChunks(theY->isInplace() ? out : d, out, yc);
	m_totalTimeY += rdtscElapsed(s);
	d.setData(0, 0);
}

void Combination::processOwnChunks(const BufferDatas &in, BufferDatas &out, uint)
{
	if (out[0].isNull())
		return;
	uint yc = out[0].samples() / theY->theOut;
	uint interSamples = theY->theIn + theY->theStep * (yc - 1);
	uint cachedSamples = theY->theIn - theY->theStep;
	float cache[cachedSamples * theInterScope];

	if (theResident)
	{
		assert(m_residentSamples > cachedSamples);

		// copy the last cachedSamples samples to the cache.
		theResident->mid(m_residentSamples - cachedSamples, cachedSamples).copyTo(cache, cachedSamples * theInterScope);

		if (theResident->samples() < interSamples)
		{
			delete theResident;
			theResident = 0;
		}
	}
	else
		cachedSamples = 0;
	if (!theResident)
		theResident = new BufferData(interSamples * theInterScope, theInterScope);
	theResident->leftSamples(cachedSamples).copyFrom(cache, cachedSamples * theInterScope);

	BufferDatas d(1);

	uint xc = (interSamples - cachedSamples) / theX->theOut;

	d.copyData(0, theResident->samples(cachedSamples));
	assert(in[0].samples() == (xc - 1) * theX->theStep + theX->theIn);
	BufferDatas aIn = in.rightSamples((xc - 1) * theX->theStep + theX->theIn);

	unsigned long long s = rdtsc();
	theX->processOwnChunks(in, theY->isInplace() ? out : d, xc);
	m_totalTimeX += rdtscElapsed(s);

	d.copyData(0, *theResident);

	s = rdtsc();
	theY->processOwnChunks(theY->isInplace() ? out : d.leftSamples(interSamples), out, yc);
	m_totalTimeY += rdtscElapsed(s);
	m_residentSamples = theY->theIn + (yc - 1) * theY->theStep;
}

PropertiesInfo Combination::specifyProperties() const
{
	return theX->specifyProperties() + theY->specifyProperties().stashed();
}

void Combination::initFromProperties(const Properties& _p)
{
	Properties p = _p;
	theY->initFromProperties(p.unstash());
	theX->initFromProperties(p);
	setupIO(theX->theNumInputs, theY->theNumOutputs);
}

void Combination::updateFromProperties(const Properties& _p)
{
	Properties p = _p;
	theY->updateFromProperties(p.unstash());
	theX->updateFromProperties(p);
}

bool Combination::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	resetTime();
	Types r(1);
	if (theX->proxyVSTypes(inTypes, r) && r.populated(0))
	{
		theInterScope = r[0]->size();
		if (!theY->proxyVSTypes(r, outTypes))
			return false;
		if (theY->theIn >= theX->theOut && theY->theStep >= theX->theOut && !(theY->theIn % theX->theOut) && !(theY->theStep % theX->theOut) && theX->theNumOutputs == 1 && theY->theNumInputs == 1)
		{
			setupSamplesIO(theX->theIn + theX->theStep * (theY->theIn / theX->theOut - 1), theX->theStep * theY->theStep / theX->theOut, theY->theOut);
			setFlag(SubInplace, (theX->isInplace() || !theY->isInplace()));
			if (MESSAGES) qDebug("Setting up IO: %d->%d, %d/%d => %d", theNumInputs, theNumOutputs, theIn, theStep, theOut);
			delete theResident;
			theResident = 0;
			return true;
		}
		else
		{
			qDebug("WARNING: Could not initialise - incompatible SubProcessors: %s(%d, %d, %d) === %s(%d, %d, %d).", qPrintable(theX->theType), theX->theIn, theX->theStep, theX->theOut, qPrintable(theY->theType), theY->theIn, theY->theStep, theY->theOut);
			return false;
		}
	}
	else
		return false;
}

}

#undef MESSAGES

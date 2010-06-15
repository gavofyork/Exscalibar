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

#include "qfactoryexporter.h"

#include "transmissiontype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

class DLLEXPORT Add: public HeavyProcessor
{
	int count;
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties(const Properties &properties);
	virtual QString simpleText() const { return "+"; }
public:
	Add(): HeavyProcessor("Add") {}
};

void Add::processor()
{
	while (true)
	{	input(0).waitForSamples();
		input(1).waitForSamples();
		int samples = min(min(input(0).samplesReady(), input(1).samplesReady()), output(0).maximumScratchSamples());
		// TODO: allow pushing of greater than buffer-size of elements (with foreign BDs)

		const BufferData i0 = input(0).readSamples(samples);
		const BufferData i1 = input(1).readSamples(samples);
		BufferData o = output(0).makeScratchSamples(samples);
		for (uint i = 0; i < i0.elements(); i++)
			o[i] = i0[i] + i1[i];
		output(0).push(o);
	}
}

bool Add::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	if (!inTypes.allSame()) return false;
	outTypes[0] = inTypes[0];
	return true;
}

void Add::initFromProperties(const Properties &)
{
	setupIO(2, 1);
}

EXPORT_CLASS(Add, 0,1,0, Processor);

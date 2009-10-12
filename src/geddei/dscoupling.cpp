/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include "domprocessor.h"
#include "dscoupling.h"

#define MESSAGES 0

namespace Geddei
{

DSCoupling::DSCoupling(DomProcessor *dom, SubProcessor *subProc) : DxCoupling(dom), xSCoupling(subProc), m_isReady(true)
{
	dom->ratify(this);
}

DSCoupling::~DSCoupling()
{
	assert(m_isReady);
}

int DSCoupling::doWork()
{
	if (!m_isReady)
	{
		theSubProc->processChunks(m_ins, m_outs, m_chunks);
		m_ins.nullify();
		m_outs.nullify();
		m_isReady = true;
		return DidWork;
	}
	return NoWork;
}

void DSCoupling::go()
{
	QTask::start();
}

void DSCoupling::stop()
{
	QTask::stop();
}

void DSCoupling::processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks)
{
	m_ins = _ins;
	m_outs = _outs;
	m_chunks = _chunks;
	m_isReady = false;
}

bool DSCoupling::isReady()
{
	return m_isReady;
}

void DSCoupling::specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes)
{
	SignalTypeRefs dummyOutTypes(outTypes.count());

	if (!theSubProc->proxyVSTypes(inTypes, dummyOutTypes))
	{	qDebug("*** CRITICAL: SubProcessor does not verify previously validated types.");
	}
	theSubProc->theOutTypes = outTypes;
	// TODO: check outTypes == dummyOutTypes, taking into account MultiOut
}

void DSCoupling::initFromProperties(const Properties &p)
{
	theSubProc->initFromProperties(p);
}

void DSCoupling::defineIO(uint inputs, uint outputs)
{
	theSubProc->defineIO(inputs, outputs);
}

}

#undef MESSAGES

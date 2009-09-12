/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

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

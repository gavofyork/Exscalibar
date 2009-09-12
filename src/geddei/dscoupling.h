/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _GEDDEI_DSCOUPLING_H
#define _GEDDEI_DSCOUPLING_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qtask.h"
#include "dxcoupling.h"
#include "xscoupling.h"
#else
#include <qtextra/qtask.h>
#include <geddei/dxcoupling.h>
#include <geddei/xscoupling.h>
#endif
using namespace QtExtra;

namespace Geddei
{

class DomProcessor;

/** @internal @ingroup Geddei
 * @brief Embodiment of a direct coupling between DomProcessor and SubProcessor.
 * @author Gav Wood <gav@kde.org>
 */
class DSCoupling: public DxCoupling, public xSCoupling, public QTask
{
	friend class DomProcessor;

public:
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks);
	virtual bool isReady();
	virtual void specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &p);
	virtual void go();
	virtual void stop();
	virtual void defineIO(uint numInputs, uint numOutputs);

private:
	DSCoupling(DomProcessor *dom, SubProcessor *subProc);
	~DSCoupling();

	virtual int doWork();
	virtual void onStopped() {}

	bool m_isReady;
	BufferDatas m_ins;
	BufferDatas m_outs;
	uint m_chunks;
};


}

#endif

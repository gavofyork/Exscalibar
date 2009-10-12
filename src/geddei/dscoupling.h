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

#pragma once

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

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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "transmissiontype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Log : public SubProcessor
{
	uint m_arity;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);

public:
	Log() : SubProcessor("Log") {}
};

void Log::processChunk(const BufferDatas &ins, BufferDatas &outs) const
{
	for (uint i = 0; i < m_arity; i++)
		outs[0][i] = log(ins[0][i]);
}

bool Log::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	outTypes[0] = inTypes[0];
	m_arity = inTypes[0].arity();
	return true;
}

EXPORT_CLASS(Log, 0,2,0, SubProcessor);

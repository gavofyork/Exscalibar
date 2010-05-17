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

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
#include "buffer.h"
using namespace Geddei;

class Mean : public SubProcessor
{
	uint theSize;

	virtual void processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual QString simpleText() const { return QChar(0x2A0F); }
public:
	Mean() : SubProcessor("Mean", In) {}
};

void Mean::processChunks(const BufferDatas &ins, BufferDatas &outs, uint chunks) const
{
	for (uint c = 0; c < chunks; c++)
		for (uint j = 0; j < theSize; j++)
			outs[0](c, j) = 0.;
	for (uint i = 0; i < multiplicity(); i++)
		for (uint c = 0; c < chunks; c++)
			for (uint j = 0; j < theSize; j++)
				outs[0](c, j) += ins[i](c, j) / float(multiplicity());
}

bool Mean::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes.count())
		return false;
	outTypes[0] = inTypes[0];
	theSize = inTypes[0].scope();
	return true;
}

EXPORT_CLASS(Mean, 0,3,0, SubProcessor);

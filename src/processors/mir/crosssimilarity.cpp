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

#include <cmath>
using namespace std;

#include "qfactoryexporter.h"

#include "signaltype.h"
#include "bufferdata.h"
#include "subprocessor.h"
using namespace Geddei;

#include "matrix.h"
using namespace SignalTypes;

class CrossSimilarity: public SubProcessor
{
	int theArity, theCount;

	virtual void processChunk(const BufferDatas &in, BufferDatas &out) const;
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);

public:
	CrossSimilarity(): SubProcessor("CrossSimilarity", In) {}
};

void CrossSimilarity::processChunk(const BufferDatas &in, BufferDatas &out) const
{
	for (int i = 0; i < theCount; i++)
		for (int ii = 0; ii < theCount; ii++)
		{	out[0][i * theCount + ii] = 0.;
			for (int j = 0; j < theArity; j++)
				out[0][i * theCount + ii] += (1. - abs(in[i][j] - in[ii][j])) / float(theArity);
		}
}

bool CrossSimilarity::verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes)
{
	if (!inTypes[0].isA<SquareMatrix>())
		return false;
	theArity = inTypes[0].asA<TransmissionType>().arity();
	theCount = multiplicity();
	outTypes[0] = SquareMatrix(theCount, inTypes[0].asA<SquareMatrix>().frequency());
	return true;
}

EXPORT_CLASS(CrossSimilarity, 0,1,0, SubProcessor);

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

#include "qfactoryexporter.h"

#include "transmissiontype.h"
#include "bufferdata.h"
#include "processor.h"
#include "buffer.h"
using namespace Geddei;

class Split : public HeavyProcessor
{
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual void initFromProperties(const Properties &properties);
public:
	Split(): HeavyProcessor("Split", Out, Guarded) {}
};

void Split::processor()
{
	while (thereIsInputForProcessing(1))
	{	// Get our maximum amount of samples to transfer that everyone can agree on
		uint m = input(0).samplesReady();
		for (uint i = 0; i < numOutputs(); i++) m = min(m, output(i).maximumScratchSamples());
		// Read them
		BufferData d = input(0).readSamples(m);
		// Push them
		// TODO: allow pushing of greater than buffer-size of elements (with foreign BDs), so no maxSS() call neccessary
		for (uint i = 0; i < numOutputs(); i++)
			output(i).push(d);
	}
}

bool Split::verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes)
{
	outTypes = inTypes[0];
	return true;
}

void Split::initFromProperties(const Properties &)
{
	setupIO(1, Undefined);
}

EXPORT_CLASS(Split, 0,1,0, Processor);

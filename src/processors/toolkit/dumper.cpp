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

#include <qfile.h>
#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "coretypes.h"
using namespace Geddei;

class Dumper: public HeavyProcessor
{
	QFile theOut;
	bool theFloats;
	bool theText;
	bool theCommas;

	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Commas", false, "Separate values by commas")("Output", "/tmp/data", "The filename in to which data should be written.")("Binary", true, "Write data in binary (false for text)")("Floats", false, "Use binary floats instead of bytes."); }
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const Types &, Types &);
	virtual void processor();
	virtual void processorStopped();

public:
	Dumper();
};

Dumper::Dumper(): HeavyProcessor("Dumper", In, Guarded)
{
}

void Dumper::initFromProperties(const Properties &p)
{
	theOut.setFileName(p["Output"].toString());
	theText = !p["Binary"].toBool();
	theCommas = p["Commas"].toBool();
	theFloats = p["Floats"].toBool();

	setupIO(Undefined, 0);
}

bool Dumper::verifyAndSpecifyTypes(const Types &, Types &)
{
	return true;
}

void Dumper::processor()
{
	theOut.open(QIODevice::Truncate|QIODevice::WriteOnly);
	QTextStream ts;
	if (theText)
		ts.setDevice(&theOut);
	while (thereIsInputForProcessing(1))
	{
		for (uint i = 0; i < numInputs(); i++)
		{	const BufferData d = input(i).readSample();
			for (uint j = 0; j < d.elements(); j++)
				if (theText)
					ts << d[j] << (theCommas ? "," : " ");
				else if (theFloats)
				{
					unsigned char* dc = (unsigned char*)&(d[j]);
					theOut.putChar(dc[0]);
					theOut.putChar(dc[1]);
					theOut.putChar(dc[2]);
					theOut.putChar(dc[3]);
				}
				else
					theOut.putChar(int(min(max(0.f, d[j]), 1.f) * 255));
			if (theText)
				ts << "\t";
		}
		if (theText)
			ts << endl;
	}
}

void Dumper::processorStopped()
{
	theOut.close();
}

EXPORT_CLASS(Dumper, 0,1,1, Processor);

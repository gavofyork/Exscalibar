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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
using namespace std;

#include <qfile.h>
#include "qfactoryexporter.h"

#include "geddei.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

class Dumper: public Processor
{
	QFile theOut;
	bool theFloats;

	virtual PropertiesInfo specifyProperties() const { return PropertiesInfo("Floats", false, "Use binary floats instead of bytes."); }
	virtual void initFromProperties(const Properties &p);
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &);
	virtual void processor();
	virtual void processorStopped();

public:
	Dumper();
};

Dumper::Dumper(): Processor("Dumper", In, Guarded)
{
}

void Dumper::initFromProperties(const Properties &p)
{
	theOut.setFileName(p["Output"].toString());
	theFloats = p["Floats"].toBool();

	setupIO(Undefined, 0);
}

bool Dumper::verifyAndSpecifyTypes(const SignalTypeRefs &, SignalTypeRefs &)
{
	return true;
}

void Dumper::processor()
{
	theOut.open(QIODevice::Truncate|QIODevice::WriteOnly);
	while (thereIsInputForProcessing(1))
		for (uint i = 0; i < numInputs(); i++)
		{	const BufferData d = input(i).readSample();
			for (uint j = 0; j < d.elements(); j++)
				if (theFloats)
				{
					unsigned char* dc = (unsigned char*)&(d[j]);
					theOut.putChar(dc[0]);
					theOut.putChar(dc[1]);
					theOut.putChar(dc[2]);
					theOut.putChar(dc[3]);
				}
				else
					theOut.putChar(int(min(max(0.f, d[j]), 1.f) * 255));
		}
}

void Dumper::processorStopped()
{
	theOut.close();
}

EXPORT_CLASS(Dumper, 0,1,1, Processor);

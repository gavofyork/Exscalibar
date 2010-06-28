/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include <QFile>
#include "qfactoryexporter.h"

#include <Plugin>
using namespace Geddei;

class Dumper: public CoProcessor
{
public:
	Dumper(): CoProcessor("Dumper", In) {}

private:
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual bool verifyAndSpecifyTypes(Types const&, Types&) { return true; }
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();

	bool m_floats;
	bool m_binary;
	bool m_commas;
	QString m_output;
	DECLARE_4_PROPERTIES(Dumper, m_floats, m_binary, m_commas, m_output);

	QFile m_out;
	QTextStream m_ts;
};

PropertiesInfo Dumper::specifyProperties() const
{
	return PropertiesInfo
			("Commas", false, "Separate values by commas", false, ",", AVbool)
			("Output", "/tmp/data", "The filename in to which data should be written.", false)
			("Binary", true, "Write data in binary (false for text)", false, QChar(0x2325), AVbool)
			("Floats", false, "Use binary floats instead of bytes.", false, QChar(0x211D), AVbool);
}

void Dumper::initFromProperties()
{
	m_out.setFileName(m_output);
	setupIO(Undefined, 0);
}

bool Dumper::processorStarted()
{
	m_out.open(QIODevice::Truncate|QIODevice::WriteOnly);
	if (!m_binary)
		m_ts.setDevice(&m_out);
	return m_out.isOpen();
}

int Dumper::process()
{
	for (uint i = 0; i < numInputs(); i++)
	{	const BufferData d = input(i).readSample();
		Type t = input(i).readType();
		if (t.isA<Mark>())
		{
			double ts = Mark::timestamp(d);
			if (!m_binary)
				m_ts << ts << (m_commas ? "," : " ");
			else
			{
				unsigned char* dc = (unsigned char*)&(ts);
				for  (uint i = 0; i < sizeof(double); i++)
					m_out.putChar(dc[i]);
			}
		}
		for (uint j = 0; j < t.arity(); j++)
			if (!m_binary)
				m_ts << d[j] << (m_commas ? "," : " ");
			else if (m_floats)
			{
				unsigned char* dc = (unsigned char*)&(d[j]);
				m_out.putChar(dc[0]);
				m_out.putChar(dc[1]);
				m_out.putChar(dc[2]);
				m_out.putChar(dc[3]);
			}
			else
				m_out.putChar(int(min(max(0.f, d[j]), 1.f) * 255));
		if (!m_binary)
			m_ts << "\t";
	}
	if (!m_binary)
		m_ts << endl;
	return DidWork;
}

void Dumper::processorStopped()
{
	m_ts.flush();
	m_ts.setDevice(0);
	m_out.close();
}

EXPORT_CLASS(Dumper, 0,1,1, Processor);


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

#include <Plugin>
using namespace Geddei;

class Generator : public CoProcessor
{
public:
	Generator(): CoProcessor("Generator") {}

	double m_secs;

private:
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(Properties const& _p);
	virtual void updateFromProperties(Properties const& _p);
	virtual QString simpleText() const { return QChar(0x21FB); }
	virtual double secondsPassed() const;
};

PropertiesInfo Generator::specifyProperties() const
{
	return PropertiesInfo
			("Example", false, "Example description. { Units }");
}

void Generator::initFromProperties(Properties const& _p)
{
	updateFromProperties(_p);
	setupIO(0, 1);
}

void Generator::updateFromProperties(Properties const&)
{
}

bool Generator::verifyAndSpecifyTypes(Types const&, Types& _outTypes)
{
	_outTypes = Mark(0);
	qDebug() << _outTypes[0].arity();
	return true;
}

bool Generator::processorStarted()
{
	m_secs = 0.0;
	return true;
}

double Generator::secondsPassed() const
{
	return m_secs;
}

int Generator::process()
{
	output(0) << output(0).makeScratchSample();
	m_secs+=1.0;
	return -1000;
}

void Generator::processorStopped()
{
}

EXPORT_CLASS(Generator, 0,1,0, Processor);


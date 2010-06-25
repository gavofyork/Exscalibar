
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

#include <Plugin>
using namespace Geddei;

class Pauser : public CoProcessor
{
public:
	Pauser();

private:
	virtual QString simpleText() const { return ""; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(240, 255, 255); }
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual int process();

	bool m_paused;
	DECLARE_1_PROPERTY(Pauser, m_paused);

	QVector<float> m_last;
};

Pauser::Pauser(): CoProcessor("Pauser")
{
}

PropertiesInfo Pauser::specifyProperties() const
{
	return PropertiesInfo
		("Paused", false, "Paused.", true, "P", AVbool);
}

void Pauser::initFromProperties()
{
	setupIO(1, 1);
	setupVisual(0, 0, 1, 0, 0, false);
}

bool Pauser::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	_outTypes[0] = _inTypes[0];
	m_last.resize(_outTypes[0].arity());
	return true;
}

int Pauser::process()
{
	const BufferData in = input(0).readSample();
	BufferData out = output(0).makeScratchSample();
	if (m_paused)
	{
		out.copyFrom(m_last);
	}
	else
	{
		in.copyTo(m_last);
		out.copyFrom(in);
	}
	output(0).push(out);
	return DidWork;
}

EXPORT_CLASS(Pauser, 0,1,0, Processor);


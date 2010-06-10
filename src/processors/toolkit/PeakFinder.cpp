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

class PeakFinder : public CoProcessor
{
public:
	PeakFinder();

private:
	virtual bool processorStarted();
	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(Properties const& _p);
	virtual void updateFromProperties(Properties const& _p);
	virtual QString simpleText() const { return QChar(0x21FB); }

	bool m_goingUp;
};

PeakFinder::PeakFinder(): CoProcessor("PeakFinder")
{
}

bool PeakFinder::processorStarted()
{
	return true;
}

int PeakFinder::process()
{
	{
		const BufferData in = input(0).peekSamples(3);
		if ((in(0, 0) < in(1, 0) && in(1, 0) <= in(2, 0)) || (in(0, 0) <= in(1, 0) && in(1, 0) < in(2, 0)))
			m_goingUp = true;
		else if ((in(0, 0) < in(1, 0) && in(1, 0) > in(2, 0)) || m_goingUp == true)
		{
			BufferData out = output(0).makeScratchSample();
			out[0] = in[1];
			out[1] = in[1] + max(in[0], in[2]);
			output(0) << out;
			m_goingUp = false;
		}
	}
	input(0).readSample();
	return DidWork;
}

void PeakFinder::processorStopped()
{
}

bool PeakFinder::verifyAndSpecifyTypes(Types const& _inTypes, Types& _outTypes)
{
	if (!_inTypes[0].isA<Contiguous>() || _inTypes[0].asA<Contiguous>().arity() != 1)
		return false;
	_outTypes[0] = Mark(2);
	return true;
}

void PeakFinder::initFromProperties(Properties const& _p)
{
	updateFromProperties(_p);
	setupIO(1, 1);
}

void PeakFinder::updateFromProperties(Properties const&)
{
}

PropertiesInfo PeakFinder::specifyProperties() const
{
	return PropertiesInfo();
}

EXPORT_CLASS(PeakFinder, 0,1,0, Processor);


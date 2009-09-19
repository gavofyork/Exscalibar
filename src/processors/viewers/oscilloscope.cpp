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

#include "qfactoryexporter.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "wave.h"
using namespace SignalTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Oscilloscope: public CoProcessor
{
	QVector<float> m_last;
	float m_gain;

	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual void updateFromProperties(Properties const& _p);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = m_last.count(); }

public:
	Oscilloscope(): CoProcessor("Oscilloscope", NotMulti) {}
};

int Oscilloscope::process()
{
	BufferData d = input(0).readSamples(m_last.count());
	d.copyTo(m_last.data());
	return DidWork;
}

bool Oscilloscope::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0, _s.height() / 2);
	_p.scale(1, _s.height() / 2);
	_p.setPen(QPen(QColor(0, 0, 0, 64), 0));
	_p.drawLine(0, 0, _s.width(), 0);
	_p.drawLine(0, .5f, _s.width(), .5f);
	_p.drawLine(0, -.5f, _s.width(), -.5f);
	_p.setPen(QPen(Qt::black, 0));
	if (isRunning())
		for (int i = 2; i < _s.width(); i+=2)
			_p.drawLine(QLineF(i - 2, m_last[(i - 2) * m_last.size() / (int)_s.width()] * m_gain, i, m_last[i * m_last.size() / (int)_s.width()] * m_gain));
	return true;
}

void Oscilloscope::processorStopped()
{
}

bool Oscilloscope::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	return _inTypes.count() == 1 && _inTypes[0].isA<Wave>();
}

void Oscilloscope::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	setupVisual(32, 20, 1000 / max(1, _p["Refresh Frequency"].toInt()));
	m_last.resize(_p["Size"].toInt());
	updateFromProperties(_p);
}

void Oscilloscope::updateFromProperties(Properties const& _p)
{
	m_gain = _p["Gain"].toDouble();
}

PropertiesInfo Oscilloscope::specifyProperties() const
{
	return PropertiesInfo	("Size", 1024, "The size of the window's width (in samples).")
							("Refresh Frequency", 30, "The number of times to redraw the wave each second (in Hz).")
							("Gain", 1.f, "Gain for input signal (x).");
}

EXPORT_CLASS(Oscilloscope, 0,1,0, Processor);

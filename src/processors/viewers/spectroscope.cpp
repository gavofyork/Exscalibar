/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org			                                               *
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

#include "spectrum.h"
using namespace SignalTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Spectroscope: public Processor
{
	QVector<float> m_last;
	float m_minAmp;
	float m_deltaAmp;

	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;

public:
	Spectroscope(): Processor("Spectroscope", NotMulti, Cooperative) {}
};

int Spectroscope::process()
{
	input(0).readSample().copyTo(m_last.data());
	return DidWork;
}

bool Spectroscope::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	_p.setRenderHint(QPainter::Antialiasing, false);
	_p.translate(0, _s.height());
	_p.setPen(QPen(QColor(0, 0, 0, 64), 0));
	_p.drawLine(0, 0, _s.width(), 0);
	if (isRunning())
	{
		_p.scale(_s.width() / m_last.size(), 1);
		_p.setPen(QPen(Qt::black, _s.width() > m_last.size() ? 1 : 0));
		for (int i = 0; i < m_last.size(); i++)
			_p.drawLine(QLineF(i, 0, i, -(m_last[i] - m_minAmp) * m_deltaAmp * _s.height()));
	}
	return true;
}

void Spectroscope::processorStopped()
{
}

bool Spectroscope::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	if (_inTypes.count() != 1 || !_inTypes[0].isA<Spectrum>())
		return false;
	m_minAmp = _inTypes[0].asA<Spectrum>().minAmplitude();
	m_deltaAmp = _inTypes[0].asA<Spectrum>().maxAmplitude() - m_minAmp;
	m_last.resize(_inTypes[0].asA<Spectrum>().size());
	return true;
}

void Spectroscope::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	this->setupVisual(32, 20, 1000 / max(1, _p["Refresh Frequency"].toInt()));
}

PropertiesInfo Spectroscope::specifyProperties() const
{
	return PropertiesInfo("Refresh Frequency", 30, "The number of times to redraw the wave each second (Hz).");
}

EXPORT_CLASS(Spectroscope, 0,1,0, Processor);



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

#include <QRgb>

#include "qfactoryexporter.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "matrix.h"
using namespace SignalTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Matrigraph: public CoProcessor
{
	QImage m_display;

	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = 1; }

public:
	Matrigraph(): CoProcessor("Matrigraph", NotMulti) {}
};

int Matrigraph::process()
{
	BufferData d = input(0).readSample();
	for (int y = 0; y < m_display.height(); y++)
		for (int x = 0; x < m_display.width(); x++)
			m_display.setPixel(QPoint(x, y), (uchar)(d[x + y * m_display.height()] * 255));
	return DidWork;
}

bool Matrigraph::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	if (isRunning())
	{
		_p.setRenderHint(QPainter::Antialiasing, false);
		_p.scale(_s.width() / m_display.width(), _s.height() / m_display.height());
		_p.drawImage(0, 0, m_display);
	}
	return true;
}

void Matrigraph::processorStopped()
{
}

bool Matrigraph::verifyAndSpecifyTypes(const SignalTypeRefs& _inTypes, SignalTypeRefs&)
{
	if (_inTypes.count() != 1 || !_inTypes[0].isA<Matrix>())
		return false;
	m_display = QImage(QSize(_inTypes[0].asA<Matrix>().width(), _inTypes[0].asA<Matrix>().height()), QImage::Format_Indexed8);
	for (int i = 0; i < 256; i++)
		m_display.setColor(i, qRgb(i, i, i));
	return true;
}

void Matrigraph::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	this->setupVisual(128, 128, 1000 / max(1, _p["Refresh Frequency"].toInt()));
}

PropertiesInfo Matrigraph::specifyProperties() const
{
	return PropertiesInfo("Refresh Frequency", 30, "The number of times to redraw the wave each second (Hz).");
}

EXPORT_CLASS(Matrigraph, 0,1,0, Processor);

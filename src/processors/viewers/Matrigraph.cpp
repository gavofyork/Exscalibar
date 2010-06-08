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

#include <QRgb>

#include "qfactoryexporter.h"
#include "buffer.h"
#include "processor.h"
using namespace Geddei;

#include "matrix.h"
using namespace TransmissionTypes;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class Matrigraph: public CoProcessor
{
	mutable QPixmap m_display;
	QVector<float> m_last;
	mutable bool m_isNew;

	virtual int process();
	virtual void processorStopped();
	virtual bool verifyAndSpecifyTypes(const Types &inTypes, Types &outTypes);
	virtual PropertiesInfo specifyProperties() const;
	virtual void initFromProperties(const Properties &properties);
	virtual bool paintProcessor(QPainter& _p, QSizeF const& _s) const;
	virtual void specifyInputSpace(QVector<uint>& _s) { _s[0] = 1; }
	virtual QColor specifyOutlineColour() const { return QColor::fromHsv(120, 96, 160); }

public:
	Matrigraph(): CoProcessor("Matrigraph", NotMulti) {}
};

int Matrigraph::process()
{
	input(0).readSample().copyTo(m_last.data());
	m_isNew = true;
	return DidWork;
}

bool Matrigraph::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	if (isRunning())
	{
		if (m_isNew)
		{
			QImage im(m_display.size(), QImage::Format_Indexed8);
			for (int i = 0; i < 256; i++)
				im.setColor(i, qRgb(i, i, i));
			for (int y = 0; y < im.height(); y++)
				for (int x = 0; x < im.width(); x++)
					im.setPixel(QPoint(x, im.height() - 1 - y), (uchar)(m_last[x + y * im.height()] * 255));
			m_display = QPixmap::fromImage(im);
			m_isNew = false;
		}
		_p.setRenderHint(QPainter::Antialiasing, false);
		_p.scale(_s.width() / m_display.width(), _s.height() / m_display.height());
		_p.drawPixmap(0, 0, m_display);
	}
	return true;
}

void Matrigraph::processorStopped()
{
}

bool Matrigraph::verifyAndSpecifyTypes(const Types& _inTypes, Types&)
{
	if (_inTypes.count() != 1 || !_inTypes[0].isA<Matrix>())
		return false;
	m_last.resize(_inTypes[0].asA<Matrix>().arity());
	m_display = QPixmap(_inTypes[0].asA<Matrix>().width(), _inTypes[0].asA<Matrix>().height());
	m_display.fill(Qt::white);
	setupVisual(m_display.width(), m_display.height(), redrawPeriod());
	return true;
}

void Matrigraph::initFromProperties(Properties const& _p)
{
	setupIO(1, 0);
	setupVisual(128, 128, _p["Refresh Frequency"].toInt() >= 1 ? 1000 / _p["Refresh Frequency"].toInt() : 0);
}

PropertiesInfo Matrigraph::specifyProperties() const
{
	return PropertiesInfo("Refresh Frequency", 30, "The number of times to redraw the wave each second (Hz).");
}

EXPORT_CLASS(Matrigraph, 0,1,0, Processor);

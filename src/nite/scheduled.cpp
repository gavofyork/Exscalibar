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

#include <QtGui>

#include "qtask.h"
#include "qscheduler.h"
#include "qworker.h"
using namespace QtExtra;

#include "GeddeiNite.h"
#include "scheduled.h"

Scheduled::Scheduled(QWidget *parent) :
    QWidget(parent)
{
}

void Scheduled::startUpdating()
{
	m_timer = startTimer(5000);
}

void Scheduled::stopUpdating()
{
	killTimer(m_timer);
}

void Scheduled::paintEvent(QPaintEvent*)
{
	QPainter p(this);
	GeddeiNite* gn = qobject_cast<GeddeiNite*>(topLevelWidget());

	QList<QWorker*> ws = QScheduler::get()->workers();
	float duration = .001f;
	p.scale(width() / duration, height() / float(ws.count()));
	int y = 0;
	double now = 0.0;
	foreach (QWorker* w, ws)
	{
		QRing<QWorker::TimeSlice> const& tss = w->timeSlices();
		if (tss.count())
		{
			if (now == 0.0)
				now = tss[tss.count() - 1].stop;
			else
				now = max(tss[tss.count() - 1].stop, now);
		}
	}
	if (now != 0.0)
		foreach (QWorker* w, ws)
		{
			QRing<QWorker::TimeSlice> const& tss = w->timeSlices();
			for (int i = tss.count() - 1; i > 0; i--)
				if (now - tss[i].start > duration)
					break;
				else
					p.fillRect(QRectF(now - tss[i].start, y, tss[i].stop - tss[i].start, 1.f), gn->myColour(tss[i].task));
			y++;
		}
}

void Scheduled::timerEvent(QTimerEvent*)
{
	update();
}

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

#include <QDateTime>

#include <exscalibar.h>

#include "qtask.h"
#include "qworker.h"
using namespace QtExtra;

QWorker::QWorker(QScheduler* _boss, uint _index):
	m_boss				(_boss),
	m_index				(_index),
	m_sinceLastWorked	(0),
	m_waitPeriod		(0),
	m_timeSlices		(10000)
{
	QThread::start();
}

QWorker::~QWorker()
{
	QThread::terminate();
	QThread::wait();
}

void QWorker::beginAgain()
{
	m_timeSlices.clear();
}

void QWorker::run()
{
	for (QTask* t = 0;;)
	{
		t = m_boss->nextTask(t, this);
		double s = QScheduler::currentTime();
		t->attemptProcess();
		double e = QScheduler::currentTime();
		m_timeSlices.shift(TimeSlice(t, s, e));
	}
}

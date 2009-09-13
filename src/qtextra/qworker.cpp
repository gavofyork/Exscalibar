/***************************************************************************
 *   Copyright (C) 2009 by Gav Wood                                        *
 *   gav@kde.org                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <QDateTime>

#include <exscalibar.h>

#include "qworker.h"
using namespace QtExtra;

QWorker::QWorker(QScheduler* _boss, uint _index):
	m_boss				(_boss),
	m_index				(_index),
	m_sinceLastWorked	(0),
	m_waitPeriod		(0)
{
	QThread::start();
}

QWorker::~QWorker()
{
	QThread::terminate();
	QThread::wait();
}

void QWorker::run()
{
	for (QTask* t = 0;;)
	{
		t = m_boss->nextTask(t, this);
		QTime timer = QTime::currentTime();
		t->attemptProcess();
//		if (t->m_lastStatus >= 0)
//			qDebug("WORKER %d: TIMESLICED TO %s; %d ms, %s", m_index, qPrintable(t->taskName()), timer.elapsed(), t->m_lastStatus >= 0 ? "WORKED" : "NO WORK");
	}
}

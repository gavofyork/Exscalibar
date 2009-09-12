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

#include <exscalibar.h>

#include "qscheduler.h"
#include "qworker.h"
#include "qtask.h"
using namespace QtExtra;

QTask::QTask():
	m_scheduler(0),
	m_lastStatus(WillNeverWork)
{
}

QTask::~QTask()
{
	if (m_scheduler)
		m_scheduler->inDestructor(this);
}

void QTask::start()
{
	QScheduler::get()->registerTask(this);
}

void QTask::stop()
{
	m_scheduler->unregisterTask(this);
}

void QTask::wait() const
{
	while (m_scheduler)
		QWorker::msleep(1);
}

void QTask::attemptProcess()
{
	if (l_execution.tryLock())
	{
		m_lastStatus = doWork();
		l_execution.unlock();
	}
	else
		m_lastStatus = NoWork;
}

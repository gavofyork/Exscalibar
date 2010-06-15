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

#include <sys/time.h>

#include "qscheduler.h"
#include "qworker.h"
#include "qtask.h"
#include "rdtsc.h"
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
	m_taskCount = 0;
	m_totalTime = 0.0;
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
	unsigned long long s = rdtsc();
	m_lastStatus = doWork();
	m_totalTime += rdtscElapsed(s);
	m_taskCount++;
}

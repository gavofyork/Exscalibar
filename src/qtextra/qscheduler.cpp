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

#include <QThread>

#include <exscalibar.h>

#include "qtask.h"
#include "qworker.h"
#include "qscheduler.h"
using namespace QtExtra;

QScheduler* QScheduler::s_this = 0;

QScheduler::QScheduler(int _workers):
	m_robin(0)
{
	setWorkers(_workers);
}

QScheduler::~QScheduler()
{
	setWorkers(0);
	clearTasks();
}

void QScheduler::registerTask(QTask* _p)
{
	QFastMutexLocker l(&l_tasks);
	m_tasks.append(_p);
	_p->m_scheduler = this;
	_p->m_lastStatus = QTask::DidWork;
}

void QScheduler::unregisterTask(QTask* _p)
{
	QFastMutexLocker l(&l_tasks);
	_p->guaranteeStopped();
	m_tasks.removeAll(_p);
	_p->m_scheduler = 0;
	_p->releaseGuarantee();
	_p->onStopped();
}

void QScheduler::inDestructor(QTask* _p)
{
	unregisterTask(_p);
}

void QScheduler::clearTasks()
{
	QFastMutexLocker l(&l_tasks);
	while (m_tasks.count())
	{
		m_tasks.last()->guaranteeStopped();
		QTask* t = m_tasks.takeLast();
		t->m_scheduler = 0;
		t->releaseGuarantee();
		t->onStopped();
	}
}

void QScheduler::setWorkers(int _n)
{
	if (_n == -1)
		_n = QThread::idealThreadCount() - 1;

	while (m_workers.count() > _n)
		delete m_workers.takeLast();
	while (m_workers.count() < _n)
		m_workers.append(new QWorker(this));
}

QTask* QScheduler::nextTask(QTask* _last)
{
	l_tasks.lock();

	int noWork = false;

	if (m_tasks.contains(_last))
	{
		if (_last->m_lastStatus == QTask::WillNeverWork)
		{
			_last->guaranteeStopped();
			m_tasks.removeAll(_last);
			_last->m_scheduler = 0;
			_last->releaseGuarantee();
			_last->onStopped();
		}
		else if (_last->m_lastStatus == QTask::NoWork)
		{
			int redundancy = 0;
			foreach (QTask* t, m_tasks)
				if (t->m_lastStatus < 0)
					redundancy++;
			noWork = (redundancy == m_tasks.count());
		}
		else
			m_sinceLastWorked = 0;
	}

	QTask* ret = 0;
	while (!ret)
	{
		// Always runs from one a QWorker thread.
		if (m_tasks.count() == 0)
		{
			l_tasks.unlock();
			QWorker::setTerminationEnabled(true);
			QThread::msleep(100);
			QWorker::setTerminationEnabled(false);
			l_tasks.lock();
		}
		else if (noWork)
		{
			l_tasks.unlock();
			QWorker::setTerminationEnabled(true);
			// TODO Configurable by the processors.
			QThread::msleep(10);
			QWorker::setTerminationEnabled(false);
			l_tasks.lock();
			m_sinceLastWorked = 0;
		}
		else
		{
			m_robin = (m_robin + 1) % m_tasks.count();
			ret = m_tasks[m_robin];
		}
	}
	l_tasks.unlock();
	return ret;
}

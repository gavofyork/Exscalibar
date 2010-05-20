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
	if (m_tasks.isEmpty())
	{
		m_startRdtsc = rdtsc();
		QFastMutexLocker l(&l_workers);
		foreach (QWorker* w, m_workers)
			w->beginAgain();
	}
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
		_n = QThread::idealThreadCount();

	while (m_workers.count() > _n)
		delete m_workers.takeLast();
	while (m_workers.count() < _n)
		m_workers.append(new QWorker(this, m_workers.count()));
}

QTask* QScheduler::nextTask(QTask* _last, QWorker* _w)
{
	// Always runs from a QWorker thread.

	int ls = 0;
	if (m_tasks.contains(_last))
	{
		_last->l_execution.unlock();
		ls = _last->m_lastStatus;
	}

	l_tasks.lock();

	if (m_tasks.contains(_last))
	{
		if (ls == QTask::WillNeverWork)
		{
			_last->guaranteeStopped();
			m_tasks.removeAll(_last);
			_last->m_scheduler = 0;
			_last->releaseGuarantee();
			_last->onStopped();
		}
		else if (ls < 0)
		{
			_w->m_sinceLastWorked++;
			_w->m_waitPeriod = qMin((int)_w->m_waitPeriod, -ls);
			if (_w->m_sinceLastWorked >= m_tasks.count())
			{
				uint wp = _w->m_waitPeriod;
				foreach (QWorker* w, m_workers)
					if (w->m_sinceLastWorked < m_tasks.count())
					{
						wp = 1;
						break;
					}
					else
						wp = qMin(wp, w->m_waitPeriod);
				l_tasks.unlock();
				QWorker::setTerminationEnabled(true);
//				qDebug("WORKER %d: SLEEPING FOR %d ms", _w->m_index, wp);
				QThread::msleep(wp);
				QWorker::setTerminationEnabled(false);
				l_tasks.lock();
				_w->m_waitPeriod = (int)-QTask::NoWork;
				_w->m_sinceLastWorked = 0;
			}
		}
		else
		{
			_w->m_waitPeriod = (int)-QTask::NoWork;
			_w->m_sinceLastWorked = 0;
		}
	}

	QTask* ret = 0;
	while (!ret)
	{
		if (m_tasks.count() == 0)
		{
			l_tasks.unlock();
			QWorker::setTerminationEnabled(true);
			QThread::msleep(100);
			QWorker::setTerminationEnabled(false);
			l_tasks.lock();
		}
		else
		{
			ret = m_tasks[(m_tasks.indexOf(_last) + 1) % m_tasks.count()];
			if (!ret->l_execution.tryLock())
			{
				_last = ret;
				ret = 0;
			}
		}
	}
	l_tasks.unlock();
	return ret;
}

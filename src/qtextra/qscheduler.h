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

#pragma once

#include <QString>
#include <QList>

#include "qfastwaitcondition.h"
#include "rdtsc.h"

#include <exscalibar.h>

namespace QtExtra
{

class QTask;
class QWorker;

class DLLEXPORT QScheduler
{
public:
	QScheduler(int _workers = -1);
	~QScheduler();

	static QScheduler* get() { if (!s_this) s_this = new QScheduler; return s_this; }

	inline static double currentTime() { return rdtscElapsed(get()->m_startRdtsc); }

	QTask* nextTask(QTask* _last, QWorker* _w);

	void setWorkers(int _n = -1);

	void registerTask(QTask* _p);
	void unregisterTask(QTask* _p);
	void clearTasks();
	void inDestructor(QTask* _p);

	QList<QTask*> tasks() const { QList<QTask*> ret; l_tasks.lock(); foreach (QTask* t, m_tasks) ret << t; l_tasks.unlock(); return ret; }
	QList<QWorker*> workers() const { QList<QWorker*> ret; l_workers.lock(); foreach (QWorker* w, m_workers) ret << w; l_workers.unlock(); return ret; }

private:
	mutable QFastMutex l_tasks;
	QList<QTask*> m_tasks;
	mutable QFastMutex l_workers;
	QList<QWorker*> m_workers;

	int m_robin;
	int m_sinceLastWorked;

	realTime m_startRdtsc;

	static QScheduler* s_this;
};

}

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

#pragma once

#include <QString>

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

	QTask* nextTask(QTask* _last, QWorker* _w);

	void setWorkers(int _n = -1);

	void registerTask(QTask* _p);
	void unregisterTask(QTask* _p);
	void clearTasks();
	void inDestructor(QTask* _p);

private:
	QFastMutex l_tasks;
	QList<QTask*> m_tasks;
	QFastMutex l_workers;
	QList<QWorker*> m_workers;

	int m_robin;
	int m_sinceLastWorked;

	static QScheduler* s_this;
};

}

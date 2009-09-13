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
#include <QThread>

#include <exscalibar.h>

namespace QtExtra
{

class QScheduler;

class DLLEXPORT QWorker: private QThread
{
	friend class QScheduler;

public:
	static void sleep(uint _s) { QThread::msleep(_s); }
	static void msleep(uint _ms) { QThread::msleep(_ms); }
	static void usleep(uint _us) { QThread::msleep(_us); }

	uint index() const { return m_index; }

private:
	QWorker(QScheduler* _boss, uint _index);
	~QWorker();

	static void setTerminationEnabled(bool _e) { QThread::setTerminationEnabled(_e); }

	virtual void run();

	QScheduler* m_boss;
	uint m_index;
	int m_sinceLastWorked;
	uint m_waitPeriod;
};

}

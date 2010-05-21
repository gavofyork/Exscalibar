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

#pragma once

#include <QString>
#include <QThread>

#include "qring.h"

#include <exscalibar.h>

namespace QtExtra
{

class QScheduler;

class DLLEXPORT QWorker: private QThread
{
	friend class QScheduler;

public:
	struct TimeSlice { QTask* task; double start; double stop; TimeSlice(QTask* t = 0, double s = 0.0, double e = 0.0): task(t), start(s), stop(e){} };

	static void sleep(uint _s) { QThread::msleep(_s); }
	static void msleep(uint _ms) { QThread::msleep(_ms); }
	static void usleep(uint _us) { QThread::msleep(_us); }

	uint index() const { return m_index; }

	QRing<TimeSlice> const& timeSlices() const { return m_timeSlices; }
	void beginAgain();

private:
	QWorker(QScheduler* _boss, uint _index);
	~QWorker();

	static void setTerminationEnabled(bool _e) { QThread::setTerminationEnabled(_e); }

	virtual void run();

	QScheduler* m_boss;
	uint m_index;
	int m_sinceLastWorked;
	uint m_waitPeriod;

	QRing<TimeSlice> m_timeSlices;
};

}

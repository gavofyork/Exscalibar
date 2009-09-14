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
#include <QMutex>

#include <exscalibar.h>
#include "qfastwaitcondition.h"

namespace QtExtra
{

class QScheduler;

class DLLEXPORT QTask
{
	friend class QScheduler;	// For lifetime reg/dereg.
	friend class QWorker;		// For access to attemptProcess()

public:
	QTask();
	~QTask();

	// +ve for how many more cycles of work there is to do.
	// -ve for how long (in us) we will be ignored.
	// WillNeverWork and we will be ignored forever.
	// NoWork and we will be ignored until something else completes work (or 15ms).
	enum { WillNeverWork = INT_MIN, NoWork = -15, ImminentWork = -1, DidWork = 0, NoMoreWork = 0, CanWork = 1, CanStillWork = 1 };

	void start();
	void stop();
	void wait() const;
	bool isRunning() const { return m_scheduler; }

	virtual int doWork() { return WillNeverWork; }
	virtual void onStopped() {}
	virtual QString taskName() const { return QString::number((uint)this); }

private:
	virtual void attemptProcess();
	void guaranteeStopped() { l_execution.lock(); }
	void releaseGuarantee() { l_execution.unlock(); }

	QFastMutex l_execution;
	QScheduler* m_scheduler;

	int m_lastStatus;
};

}

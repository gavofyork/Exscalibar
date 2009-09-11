/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
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

#ifndef _QT_FASTWAITCONDITION_H
#define _QT_FASTWAITCONDITION_H

#define HAVE_LINUX

#ifdef HAVE_LINUX

#include <climits>

#include <qmutex.h>

#include <sched.h>

#include <exscalibar.h>

#define SINGLE_THREADED 1

#ifdef SINGLE_THREADED

class DLLEXPORT QFastMutex
{
public:
	enum { NonRecursive, Recursive };
	void lock() {}
	bool tryLock() { return true; }
	void unlock() {}
	bool isLocked() { return true; }
	QFastMutex(int = NonRecursive) {}
};

class DLLEXPORT QFastMutexLocker
{
public:
	QFastMutexLocker(QFastMutex*) {}
};

#else

typedef QMutex QFastMutex;
typedef QMutexLocker QFastMutexLocker;

#endif

class DLLEXPORT QFastWaitCondition
{
public:
	bool wait(unsigned long  = ULONG_MAX) { sched_yield(); return true; }
	bool wait(QFastMutex * mutex, unsigned long  = ULONG_MAX) { mutex->unlock(); sched_yield(); mutex->lock(); return true; }
	void wakeOne() {}
	void wakeAll() {}
};

#else

#include <qwaitcondition.h>

typedef QWaitCondition QFastWaitCondition;

#endif

#endif

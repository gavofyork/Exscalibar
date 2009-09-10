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

#ifndef _QT_COUNTER_H
#define _QT_COUNTER_H

#undef UTILISE_PTHREAD

#ifdef UTILISE_PTHREAD
#include <pthread.h>
#else
#include <qmutex.h>
#endif

#include <exscalibar.h>

/** @internal @ingroup QtExtra
 * @brief Thread-safe integer counter object.
 * @author Gav Wood <gav@kde.org>
 *
 * This class is a simple thread-safe integer counter similar in ways to some
 * of the functionality of a semaphore.
 */
class DLLEXPORT QCounter
{
	int theCount;
#ifdef UTILISE_PTHREAD
	pthread_mutex_t theMutex;
#else
	QMutex theMutex;
#endif
public:
	/**
	 * Prefix increment operator. Atomically increment the counter by one and
	 * return the new value.
	 *
	 * @return The new value of the counter.
	 */
	int operator++();

	/**
	 * Prefix increment operator. Atomically increment the counter by one and
	 * return the new value.
	 *
	 * @return The new value of the counter.
	 */
	int operator++(int);

	/**
	 * Prefix decrement operator. Atomically decrement the counter by one and
	 * return the new value.
	 *
	 * @return The new value of the counter.
	 */
	int operator--();

	/**
	 * Prefix decrement operator. Atomically decrement the counter by one and
	 * return the new value.
	 *
	 * @return The new value of the counter.
	 */
	int operator--(int);

	/**
	 * Atomic shorthand operator. Atomically increment the counter by @a i and
	 * return the result.
	 *
	 * @param i The amount to increment the counter by.
	 * @return The new value of the counter.
	 */
	int operator+=(int i);

	/**
	 * Atomic shorthand operator. Atomically decrement the counter by @a i and
	 * return the result.
	 *
	 * @param i The amount to decrement the counter by.
	 * @return The new value of the counter.
	 */
	int operator-=(int i);

	/**
	 * Assignment operator for an int.
	 *
	 * @param i The new value for the counter.
	 * @return The counter's value, this is guaranteed to be @a i.
	 */
	int operator=(int i);

	/**
	 * Get the "current" value of the counter as an integer cast.
	 *
	 * @note This provides no guarantees as to when this value is correct,
	 * since synchronisation makes no sense with an atomic read.
	 *
	 * @return The value of the counter as some point between call start and
	 * end.
	 */
	operator int() const;

	/**
	 * Basic constructor.
	 *
	 * @param i The initial value of the counter.
	 */
	QCounter(int i = 0);

	/**
	 * Basic destructor.
	 */
	~QCounter();
};

#ifdef UTILISE_PTHREAD
inline int QCounter::operator++() { pthread_mutex_lock(&theMutex); int ret = (++theCount); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator++(int) { pthread_mutex_lock(&theMutex); int ret = (theCount++); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator--() { pthread_mutex_lock(&theMutex); int ret = (--theCount); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator--(int) { pthread_mutex_lock(&theMutex); int ret = (theCount--); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator+=(int i) { pthread_mutex_lock(&theMutex); int ret = (theCount += i); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator-=(int i) { pthread_mutex_lock(&theMutex); int ret = (theCount -= i); pthread_mutex_unlock(&theMutex); return ret; }
inline int QCounter::operator=(int i) { pthread_mutex_lock(&theMutex); int ret = (theCount = i); pthread_mutex_unlock(&theMutex); return ret; }
inline QCounter::operator const int() const { return theCount; }
inline QCounter::QCounter(int i) : theCount(i) { pthread_mutex_init(&theMutex, NULL); }
inline QCounter::~QCounter() { pthread_mutex_destroy(&theMutex); }
#else
inline int QCounter::operator++() { QMutexLocker lock(&theMutex); return ++theCount; }
inline int QCounter::operator++(int) { QMutexLocker lock(&theMutex); return theCount++; }
inline int QCounter::operator--() { QMutexLocker lock(&theMutex); return --theCount; }
inline int QCounter::operator--(int) { QMutexLocker lock(&theMutex); return theCount--; }
inline int QCounter::operator+=(int i) { QMutexLocker lock(&theMutex); return theCount += i; }
inline int QCounter::operator-=(int i) { QMutexLocker lock(&theMutex); return theCount -= i; }
inline int QCounter::operator=(int i) { QMutexLocker lock(&theMutex); return theCount = i; }
inline QCounter::operator int() const { return theCount; }
inline QCounter::QCounter(int i) : theCount(i) {}
inline QCounter::~QCounter() {}
#endif

#endif

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

#ifndef _QT_CLEANER_H
#define _QT_CLEANER_H

#include <QThread>
#include <QMutex>

#include "qfastwaitcondition.h"

#include <exscalibar.h>

template<class X>
class DLLEXPORT QCleaner: public QThread, private QFastMutex
{
	QList<X*> theRow;
	int theInterval;

	void purgeAll()
	{
		QFastMutexLocker lock(this);
		while (theRow.size())
			delete theRow.takeLast();
	}

	void run()
	{
		while (1)
		{	usleep(theInterval * 1000000);
			purgeAll();
		}
	}

public:
	void deleteObject(X *theObject)
	{
		QFastMutexLocker lock(this);
		theRow.append(theObject);
	}

	QCleaner(int interval = 300): QThread()
	{
		theInterval = interval;
		start();
	}

	~QCleaner()
	{
		terminate();
		wait();
		purgeAll();
	}
};

#endif

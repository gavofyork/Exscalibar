/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
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
#ifndef __WATCHPROCESSOR_H
#define __WATCHPROCESSOR_H

#define __GEDDEI_BUILD

#include <qobject.h>
#include <q3picture.h>
#include <qpixmap.h>
#include <qmutex.h>
//Added by qt3to4:
#include <QLabel>

#include "processor.h"
#include "signaltype.h"
using namespace Geddei;

#include "signaltypes.h"
using namespace SignalTypes;

/**
@author Gav Wood <gav@cs.york.ac.uk>
*/

class QDockWidget;
class QLabel;
class QTimer;

class GeddeiNite;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class WatchProcessor: public Processor
{
	Q_OBJECT

	GeddeiNite *theGeddeiNite;
	QDockWidget *theDockWatch;
	QTimer *theTimer;
	QLabel *theLabel;

	QMutex theDrawing;

	uint theWatcher, theScope;
	float theFrequency;
	uint theWidth, theHeight;
	uint theThroughput, theDrawingTime, theReadTime, theOutTime;

	void watchWave();
	void watchGraph();
	void watchSpectrum();
	void watchSpectrograph();
	void watchMatrix();

	virtual void processor();
	virtual bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &) { setupIO(1, 1); }
	virtual void specifyInputSpace(Q3ValueVector<uint> &samples);
	virtual void specifyOutputSpace(Q3ValueVector<uint> &samples) { specifyInputSpace(samples); }

private slots:
	void repaint();

public:
	WatchProcessor(GeddeiNite *gn);
	~WatchProcessor();
};

#endif

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
#ifndef __PLAYER_H
#define __PLAYER_H

#define __GEDDEI_BUILD

#include "processor.h"
using namespace Geddei;

#ifdef HAVE_SNDFILE
#include <sndfile.h>
#endif

class HardBob;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class Player: public Processor
{
	friend class HardBob;

	QString thePath;
#ifdef HAVE_SNDFILE
	SNDFILE *theFile;
#endif
	uint theChannels, theRate, thePosition, theLength, theBlockSize;

	virtual void paintProcessor(QPainter &p);
	virtual void processor();
	virtual void processorStopped();
	virtual const bool verifyAndSpecifyTypes(const SignalTypeRefs &inTypes, SignalTypeRefs &outTypes);
	virtual void initFromProperties(const Properties &);
	virtual void specifyOutputSpace(Q3ValueVector<uint> &sizes);

public:
	Player(const QString &path);
};

#endif

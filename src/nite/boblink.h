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

#ifndef BOBLINK_H
#define BOBLINK_H

#define __GEDDEI_BUILD

#include <q3canvas.h>

#include "refresher.h"

class QDomElement;
class QDomDocument;
class BobPort;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class BobLink: public Q3CanvasLine, public Refresher
{
	friend class BobPort;
	BobPort *theFrom, *theTo;
	bool theConnected;
	uint theBufferSize, theProximity;

public:
	static int RTTI;
	virtual int rtti() const { return 1002; }

	void refresh();
	bool connected() { return theConnected; }
	void setConnected(bool c) { theConnected = c; }
	BobPort *destination() { return theTo; }

	void saveYourself(QDomElement &element, QDomDocument &doc);
	void loadYourself(QDomElement &element);

	void setBufferSize(const uint bufferSize) { theBufferSize = bufferSize; }
	const uint bufferSize() { return theBufferSize; }
	void setProximity(const uint proximity) { theProximity = proximity; }
	const uint proximity() { return theProximity; }

	BobLink(BobPort *from, BobPort *to);
	~BobLink();
};

#endif

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
#ifndef FLOATINGLINK_H
#define FLOATINGLINK_H

#define __GEDDEI_BUILD

#include <qpoint.h>
#include <q3canvas.h>

class BobPort;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class FloatingLink: public Q3CanvasLine
{
	BobPort *theFrom;
	QPoint theTo;

public:
	void update();
	void setFrom(BobPort *from);
	void setTo(QPoint to) { theTo = to; update(); }

	FloatingLink(Q3Canvas *c);
};

#endif

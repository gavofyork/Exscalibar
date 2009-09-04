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
#ifndef __HARDBOB_H
#define __HARDBOB_H

#define __GEDDEI_BUILD

#include "bob.h"

class QDomElement;
class QDomDocument;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class HardBob: public Bob
{
	virtual void saveYourself(QDomElement &element, QDomDocument &doc);

public:
	static int RTTI;
	virtual int rtti() const { return 1004; }

	HardBob(int x, int y, const QString &name, Q3Canvas *c, Processor *p);
	~HardBob();
};


#endif

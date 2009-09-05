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
#ifndef SOFTBOB_H
#define SOFTBOB_H

#define __GEDDEI_BUILD

#include "properties.h"
using namespace Geddei;

#include <qstring.h>

#include <bob.h>

class Q3Canvas;
class QDomElement;
class QDomDocument;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class SoftBob : public Bob
{
	friend class GeddeiNite;

	virtual void saveYourself(QDomElement &element, QDomDocument &doc);
	virtual void loadYourselfPre(QDomElement &element);

protected:
	QString theType;
	Properties theProperties;

	/**
	 * Rebuilds theProcessor object.
	 * This deletes the current, and creates a new one.
	 */
	virtual void rebuild();

public:
	void propertiesChanged();

public:
	const QString &type() { return theType; }
	void setName(const QString &name);

	static int RTTI;
	virtual int rtti() const { return 1005; }

	SoftBob(const QString &name, Q3Canvas *c);
	SoftBob(int x, int y, const QString &type, const QString &name, Q3Canvas *c);
	virtual ~SoftBob();
};

#endif

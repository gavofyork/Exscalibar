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

#define __GEDDEI_BUILD

#include "domsoftbob.h"

#include "domprocessor.h"
#include "subprocessorfactory.h"
#include "processorgroup.h"
using namespace Geddei;

#include <qdom.h>
#include <qtimer.h>

#include "geddeinite.h"
#include "bobport.h"
#include "boblink.h"

DomSoftBob::DomSoftBob(int x, int y, const QString &type, const QString &name, Q3Canvas *c) : SoftBob(name, c), theDom(*((DomProcessor **)&theProcessor))
{
	theType = type;

	if (theName == "") theName = geddeiNite()->makeUniqueName(type);
	theDom = SubProcessorFactory::createDom(type);

	init(x, y);
	theProperties = Properties(theDom->properties());
}

DomSoftBob::~DomSoftBob()
{
}

void DomSoftBob::rebuild()
{
	delete theDom;
	theDom = SubProcessorFactory::createDom(theType);
}


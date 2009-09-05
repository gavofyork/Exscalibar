//Added by qt3to4:
#include <Q3PtrList>
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

#ifndef __CAPTURER_H
#define __CAPTURER_H

#ifdef HAVE_ALSA

#define __GEDDEI_BUILD

#include "processor.h"
using namespace Geddei;

/** @internal
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class Capturer: public Processor
{
	virtual void processor();
	virtual bool verifyAndSpecifyTypes(Q3PtrList<SignalType> &, Q3PtrList<SignalType> &outTypes);
	virtual void initFromProperties(const Properties &) { setupIO(0, 2); }

public:
	Capturer() : Processor("Capturer") {}
};

#endif

#endif

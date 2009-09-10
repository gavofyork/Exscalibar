/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "domprocessor.h"
#include "dscoupling.h"

#define MESSAGES 0

namespace Geddei
{

DSCoupling::DSCoupling(DomProcessor *dom, SubProcessor *subProc) : DxCoupling(dom), xSCoupling(subProc)
{
	dom->ratify(this);
}

}

#undef MESSAGES

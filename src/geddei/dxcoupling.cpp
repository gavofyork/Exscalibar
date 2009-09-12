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

#include <cmath>
using namespace std;

#include "domprocessor.h"
#include "bufferreader.h"
#include "dxcoupling.h"

#define MESSAGES 0

namespace Geddei
{

DxCoupling::DxCoupling(DomProcessor *dom): theDomProcessor(dom)
{
}

DxCoupling::~DxCoupling()
{
}

}

#undef MESSAGES

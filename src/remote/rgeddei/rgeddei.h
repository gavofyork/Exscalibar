/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef __RGEDDEI_H
#define __RGEDDEI_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "abstractprocessor.h"
#include "abstractprocessorgroup.h"
#include "hostprocessorforwarder.h"
#include "sessionserver.h"
#include "localdomprocessor.h"
#include "localprocessor.h"
#include "localsession.h"
#include "remotedomprocessor.h"
#include "remoteprocessor.h"
#include "remotesession.h"
#else
#include <rgeddei/abstractprocessor.h>
#include <rgeddei/abstractprocessorgroup.h>
#include <rgeddei/hostprocessorforwarder.h>
#include <rgeddei/sessionserver.h>
#include <rgeddei/localdomprocessor.h>
#include <rgeddei/localprocessor.h>
#include <rgeddei/localsession.h>
#include <rgeddei/remotedomprocessor.h>
#include <rgeddei/remoteprocessor.h>
#include <rgeddei/remotesession.h>
#endif

/**
  * @defgroup rGeddei rGeddei's core classes.
  * @brief Module containing classes for remote controlling Geddei.
  * @author Gav Wood <gav@kde.org>
  *
  * Any classes used from this module are in namespace rGeddei. Make sure
  * you either manually specify it with rGeddei::ClassName or use it at
 * the start of your code with:
  *
  * @code
  * using namespace rGeddei;
  * @endcode
 */


#endif


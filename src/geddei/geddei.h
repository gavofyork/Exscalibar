/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef __GEDDEI_H
#define __GEDDEI_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "multiprocessorcreator.h"
#include "multiprocessor.h"
#include "combination.h"
#include "subprocessor.h"
#include "processor.h"
#include "bufferdatas.h"
#include "bufferdata.h"
#include "domprocessor.h"
#include "processorfactory.h"
#include "subprocessorfactory.h"
#include "lxconnection.h"
#include "xlconnection.h"
#include "processorgroup.h"
#include "processorport.h"
#include "properties.h"
#include "signaltype.h"
#include "signaltyperef.h"
#include "signaltyperefs.h"
#else
#include <geddei/multiprocessorcreator.h>
#include <geddei/multiprocessor.h>
#include <geddei/subprocessor.h>
#include <geddei/combination.h>
#include <geddei/processor.h>
#include <geddei/bufferdatas.h>
#include <geddei/bufferdata.h>
#include <geddei/domprocessor.h>
#include <geddei/processorfactory.h>
#include <geddei/subprocessorfactory.h>
#include <geddei/lxconnection.h>
#include <geddei/xlconnection.h>
#include <geddei/processorgroup.h>
#include <geddei/processorport.h>
#include <geddei/properties.h>
#include <geddei/signaltype.h>
#include <geddei/signaltyperef.h>
#include <geddei/signaltyperefs.h>
#endif

/**
 * @defgroup Geddei Geddei's core classes.
 * @brief Module containing classes fundamental to Geddei.
 * @author Gav Wood <gav@kde.org>
 *
 * Any classes used from this module are in namespace Geddei. Make sure
 * you either manually specify it with Geddei::ClassName or use it at
 * the start of your code with:
 *
 * @code
 * using namespace Geddei;
 * @endcode
 */



#endif

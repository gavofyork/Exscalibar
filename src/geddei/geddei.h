/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
 *
 * This file is part of Exscalibar.
 *
 * Exscalibar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Exscalibar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Exscalibar.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "multiprocessorcreator.h"
#include "multiprocessor.h"
#include "multisource.h"
#include "multisink.h"
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
#include "transmissiontype.h"
#include "type.h"
#include "types.h"
#include "contiguous.h"
#include "mark.h"
#else
#include <geddei/multiprocessorcreator.h>
#include <geddei/multiprocessor.h>
#include <geddei/multisource.h>
#include <geddei/multisink.h>
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
#include <geddei/transmissiontype.h>
#include <geddei/type.h>
#include <geddei/types.h>
#include <geddei/contiguous.h>
#include <geddei/mark.h>
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

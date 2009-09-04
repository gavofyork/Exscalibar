/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef __SIGNALTYPES_H
#define __SIGNALTYPES_H

/**
 * @defgroup SignalTypes Classes derived from SignalType.
 * @brief Module containing classes for describing different types of Signal.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 * Any classes used from this module are in namespace SignalTypes. Make sure
 * you either manually specify it with SignalTypes::ClassName or use it at
 * the start of your code with:
 *
 * @code
 * using namespace SignalTypes;
 * @endcode
 */

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "value.h"
#include "wave.h"
#include "matrix.h"
#include "spectrum.h"
#else
#include <geddei/value.h>
#include <geddei/wave.h>
#include <geddei/matrix.h>
#include <geddei/spectrum.h>
#endif

#endif

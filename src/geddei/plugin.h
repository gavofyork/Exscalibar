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

/**
 * @defgroup SignalTypes Classes derived from TransmissionType.
 * @brief Module containing classes for describing different types of Contiguous.
 * @author Gav Wood <gav@kde.org>
 *
 * Any classes used from this module are in namespace Geddei. Make sure
 * you either manually specify it with SignalTypes::ClassName or use it at
 * the start of your code with:
 *
 * @code
 * using namespace Geddei;
 * @endcode
 */

#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "qfactoryexporter.h"
#include "processor.h"
#include "subprocessor.h"
#include "bufferdata.h"
#include "coretypes.h"
#else
#include <qtextra/qfactoryexporter.h>
#include <geddei/processor.h>
#include <geddei/subprocessor.h>
#include <geddei/bufferdata.h>
#include <geddei/coretypes.h>
#endif

/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef __QTEXTRA_H
#define __QTEXTRA_H

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qcleaner.h"
#include "qcounter.h"
#include "qfactory.h"
#include "qfactoryexporter.h"
#include "qfactorymanager.h"
#include "qfastwaitcondition.h"
#include "qkohonennet.h"
#include "qpca.h"
#include "qsocketsession.h"
#else
#include <qtextra/qcleaner.h>
#include <qtextra/qcounter.h>
#include <qtextra/qfactory.h>
#include <qtextra/qfactoryexporter.h>
#include <qtextra/qfactorymanager.h>
#include <qtextra/qfastwaitcondition.h>
#include <qtextra/qkohonennet.h>
#include <qtextra/qpca.h>
#include <qtextra/qsocketsession.h>
#endif

#endif

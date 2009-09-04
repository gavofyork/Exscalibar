/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_RSCOUPLING_H
#define _GEDDEI_RSCOUPLING_H

#include <qthread.h>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD

#include "qsocketsession.h"
#include "xscoupling.h"
#else
#include <qtextra/qsocketsession.h>
#include <geddei/xscoupling.h>
#endif
using namespace Geddei;

namespace Geddei
{

/** @internal @ingroup Geddei
 * @brief Embodiment of Coupling between remote socket and local SubProcessor object.
 * @author Gav Wood <gav@cs.york.ac.uk>
 *
 */
class RSCoupling : public xSCoupling, protected QThread
{
	QSocketSession theSession;
	bool theBeingDeleted;

	//* Reimplementation from QThread.
	virtual void run();

	friend class ProcessorForwarder;

	/**
	 * Simple constructor.
	 */
	RSCoupling(Q3SocketDevice *dev, SubProcessor *sub);

	/**
	 * Simple destructor.
	 */
	virtual ~RSCoupling();
};

};

#endif

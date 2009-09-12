/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#pragma once

#include <QThread>

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
 * @author Gav Wood <gav@kde.org>
 *
 */
class RSCoupling: public xSCoupling, protected QThread
{
	friend class ProcessorForwarder;

	/**
	 * Simple constructor.
	 */
	RSCoupling(QTcpSocket *dev, SubProcessor *sub);

	/**
	 * Simple destructor.
	 */
	virtual ~RSCoupling();

	//* Reimplementation from QThread.
	virtual void run();

	QSocketSession theSession;
	bool theBeingDeleted;
};

}

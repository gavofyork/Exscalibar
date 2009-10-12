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

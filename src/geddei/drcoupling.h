/* Copyright 2003, 2004, 2005, 2007, 2009, 2010 Gavin Wood <gav@kde.org>
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

#include <QMutex>

#include <exscalibar.h>
#ifdef __GEDDEI_BUILD
#include "qsocketsession.h"
#include "dxcoupling.h"
#include "qfastwaitcondition.h"
#else
#include <qtextra/qfastwaitcondition.h>
#include <qtextra/qsocketsession.h>
#include <geddei/dxcoupling.h>
#endif
using namespace Geddei;

class QSocketSession;

namespace Geddei
{

class BufferReader;

/** @internal @ingroup Geddei
 * @brief Embodiment of Coupling between DomProcessor and remote socket.
 * @author Gav Wood <gav@kde.org>
 *
 * This class connects to the DomProcessor directly. The right side is
 * implemented as a remote connection.
 *
 * This class represents the left side of a remote DRCoupling.
 * All overrided commands are essentially just passed down the line
 * with arguments serialised as neccessary.
 */
class DRCoupling: virtual public DxCoupling
{
	//* Reimplementations from xxCoupling (used by DxCoupling)
	virtual void go() {}
	virtual void stop() {}
	virtual void specifyTypes(const Types &inTypes, const Types &outTypes);
	virtual void initFromProperties(const Properties &p);
	virtual void defineIO(uint inputs, uint outputs);
	virtual void processChunks(BufferDatas const& _ins, BufferDatas& _outs, uint _chunks);
	virtual bool isReady();

	mutable QFastMutex theComm;
	mutable QSocketSession theRemote;
	QString theRemoteHost;
	uint theRemoteKey, theRemoteSubProcessorKey;
	mutable BufferDatas m_outs;
	bool m_isReady;

public:
	/**
	 * Sets up the essential information about the SubProc on the remote end.
	 * Needed for eventual remote deletion.
	 */
	void setCredentials(const QString &remoteHost, uint remoteKey, uint remoteSubProcessorKey);

	/**
	 * Basic constructor.
	 */
	DRCoupling(DomProcessor *dom, QTcpSocket *sink);

	/**
	 * Default destructor.
	 */
	virtual ~DRCoupling();
};

}

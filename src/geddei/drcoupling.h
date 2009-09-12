/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef _GEDDEI_DRCOUPLING_H
#define _GEDDEI_DRCOUPLING_H

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
	virtual void specifyTypes(const SignalTypeRefs &inTypes, const SignalTypeRefs &outTypes);
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


};

#endif

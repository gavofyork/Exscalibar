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
#include <QThread>
#include <QString>
#include <QStringList>

#include "qfastwaitcondition.h"
#include <exscalibar.h>

#ifdef __GEDDEI_BUILD
#include "properties.h"
#include "commcodes.h"
#else
#include <geddei/properties.h>
#include <rgeddei/commcodes.h>
#endif
using namespace Geddei;

class QSocketSession;

namespace rGeddei
{

/** @ingroup rGeddei
 * @brief Client for a remote Geddei session.
 * @author Gav Wood <gav@kde.org>
 *
 * This class partners with SessionServer in order to allow controlling of
 * Geddei objects on one machine in one program by another program perhaps on a
 * different machine.
 *
 * The RemoteSession object acts as an intermediary base off which
 * RemoteProcessor objects and the like may determine session-dependant data
 * such as keys and state.
 *
 * To the developer this is simply the first stop to using another host for
 * Geddei.
 */
class DLLEXPORT RemoteSession
{
  class Keeper: public QThread
  {	RemoteSession *theOwner;
  virtual void run();
	public:
	  Keeper(RemoteSession *owner): theOwner(owner) {}
  } theKeeper;
  friend class Keeper;

  QFastMutex theCalling;
  uint theKey, theProcessorCount;
  int theLastError;
  QString theHost;
  bool theTerminating;
  QStringList theCompatibleProcessors;

  QSocketSession *theSession;

public:
  friend class RemoteProcessor;
  friend class LocalProcessor;
  friend class RemoteDomProcessor;
  friend class LocalDomProcessor;
  bool newProcessor(const QString &type, const QString &name);
  void deleteProcessor(const QString &name);
  bool newDomProcessor(const QString &subType, const QString &name);
  void deleteDomProcessor(const QString &name);
  void processorInit(const QString &name, const Properties &p, const QString &newName);
  bool processorGo(const QString &name);
  int processorWaitUntilGoing(const QString &name, int &errorData);
  void processorWaitUntilDone(const QString &name);
  void processorStop(const QString &name);
  void processorReset(const QString &name);
  bool processorConnect(const QString &name, uint bufferSize, uint output, const QString &destName, uint destInput);
  bool processorConnect(const QString &name, uint bufferSize, uint output, const QString &destHost, uint destKey, const QString &destName, uint destInput);
  void processorDisconnect(const QString &name, uint output);
  void processorDisconnectAll(const QString &name);
  void processorSplit(const QString &name, uint output);
  void processorShare(const QString &name, uint output);
  bool domProcessorCreateAndAdd(const QString &name);
  bool domProcessorCreateAndAdd(const QString &name, const QString &host, uint hostKey);
  bool typeAvailable(const QString &type);
  int typeVersion(const QString &type);
  bool typeSubAvailable(const QString &type);
  int typeSubVersion(const QString &type);
  void keepAlive();

  const QString makeUniqueProcessorName() { return "!_" + QString().setNum(++theProcessorCount); }

public:
	/**
	 * Determine if the remote host has a particular type of Processor
	 * available for creation.
	 *
	 * @param type The class name of the Processor-derived type to query.
	 * @return true iff the Geddei system on the remote side is able to create
	 * a Processor-derived class @a type .
	 */
	bool available(const QString &type);

	/**
	 * Determine if the remote host has a particular type of SubProcessor
	 * available for creation.
	 *
	 * @param type The class name of the SubProcessor-derived type to query.
	 * @return true iff the Geddei system on the remote side is able to create
	 * a SubProcessor-derived class @a type .
	 */
	bool subAvailable(const QString &type);

	/**
	 * Determine the latest version of a given Processor type available on the
	 * remote host.
	 *
	 * @param type The class name of the Processor-derived type to query.
	 * @return The latest available version of Processor-derived class
	 * @a type .
	 */
	int version(const QString &type);

	/**
	 * Determine the latest version of a given SubProcessor type available on
	 * the remote host.
	 *
	 * @param type The class name of the SubProcessor-derived type to query.
	 * @return The latest available version of SubProcessor-derived class
	 * @a type .
	 */
	int subVersion(const QString &type);

	/**
	 * Check whether the session is established. This should be verified after
	 * instantiation of this class.
	 *
	 * @return true if this session is valid and ready.
	 */
	bool isValid();

	/**
	 * Simple constructor. Creates a new session on host @a host . This machine
	 * must be running a compatible copy of nodeserver, and both control and
	 * data ports should by unemcumbered for proper operation.
	 *
	 * @param host The fully qualified host name of the remote machine to
	 * connect to.
	 * @param port The post of the remote machine to connect to. Default is the
	 * default Remote Geddei port, 16671.
	 */
	RemoteSession(const QString &host, uint port = RGEDDEI_PORT);

	/**
	 * Default destructor.
	 */
	~RemoteSession();
};

}

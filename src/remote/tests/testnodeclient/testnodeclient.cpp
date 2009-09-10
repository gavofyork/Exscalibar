/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include "properties.h"
using namespace Geddei;

#include "remotesession.h"
using namespace rGeddei;

int main(int /*argc*/, char **/*argv*/)
{
	int i;
	qDebug("Openning session...");
	RemoteSession local("127.0.0.1");
	if (!local.isValid()) qFatal("Couldn't open session on 127.0.0.1");
	qDebug("Creating O...");
	if (!local.newProcessor("WaveGen", "O")) qFatal("Couldn't create processor O");
	qDebug("Creating W...");
	if (!local.newProcessor("Monitor", "W")) qFatal("Couldn't create processor W");
	qDebug("Initing O...");
	local.processorInit("O", Properties("Stop After", 5.), "O");
	qDebug("Initing W...");
	local.processorInit("W", Properties(), "W");
	qDebug("Connecting...");
	if (!local.processorConnect("O", 1, 0, "W", 0)) qFatal("Couldn't connect O to W");
	qDebug("Starting O...");
	if (!local.processorGo("O")) qFatal("Couldn't start O");
	qDebug("Starting W...");
	if (!local.processorGo("W")) qFatal("Couldn't start W");
	qDebug("Waiting for O...");
	local.processorWaitUntilGoing("O", i);
	qDebug("Waiting for W...");
	local.processorWaitUntilGoing("W", i);
	qDebug("Stopping O...");
	local.processorStop("O");
	qDebug("Stopping W...");
	local.processorStop("W");
	qDebug("Deleting O...");
	local.deleteProcessor("O");
	qDebug("Deleting W...");
	local.deleteProcessor("W");
	qDebug("Closing session...");
}


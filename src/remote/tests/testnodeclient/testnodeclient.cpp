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


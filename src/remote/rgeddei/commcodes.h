/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#ifndef _RGEDDEI_COMMCODES_H
#define _RGEDDEI_COMMCODES_H

#define RGEDDEI_PORT 16671

enum CodeComm
{
	Nop,
	NewProcessor,
	DeleteProcessor,
	NewDomProcessor,
	DeleteDomProcessor,
	ProcessorInit,
	ProcessorGo,
	ProcessorWaitUntilGoing,
	ProcessorWaitUntilDone,
	ProcessorStop,
	ProcessorReset,
	ProcessorConnectL,
	ProcessorConnectR,
	ProcessorDisconnect,
	ProcessorDisconnectAll,
	ProcessorSplit,
	ProcessorShare,
	DomProcessorCreateAndAddL,
	DomProcessorCreateAndAddR,
	TypeAvailable,
	TypeVersion,
	TypeSubAvailable,
	TypeSubVersion,
	EndSession
};

#endif

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

enum Command
{
	NOP = 0,
	Ack,
	Nak,
	SetType,
	ResetType,
	Transfer,
	BufferWaitForFree,
	BufferWaitForFreeElements,
	BufferSpaceFree,
	WaitUntilReady,
	IsReadyYet,
	AppendPlunger,
	StartPlungers,
	PlungerSent,
	NoMorePlungers,
	EnforceMinimum,
	Close,
	SpecifyTypes,
	InitFromProperties,
	Go,
	Stop,
	Transact,
	DeliverResults,
	ProcessChunks,
	Stopping,
	Stopped,
	DefineIO
};


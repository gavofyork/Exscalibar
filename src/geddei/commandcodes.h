/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

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
	Stopping,
	Stopped,
	DefineIO
};


/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#define __GEDDEI_BUILD

#include <qdatetime.h>

#include "domprocessor.h"
#include "subprocessor.h"
using namespace Geddei;

#define MESSAGES 0

namespace Geddei
{

SubProcessor::SubProcessor(const QString &type, const MultiplicityType &multi) : theMulti(multi)
{
	theType = type;
	theLoaded = false;
	theReturned = true;
	theStopping = false;
	theNoMoreTransactions = false;
	thePrimaryOf = 0;
	theTimeTaken = 0;
	theNumInputs = theNumOutputs = theIn = theStep = theOut = 1;
}

void SubProcessor::setupIO(uint numInputs, uint numOutputs, uint samplesIn, uint samplesStep, uint samplesOut)
{
	theNumInputs = numInputs;
	theNumOutputs = numOutputs;
	setupSamplesIO(samplesIn, samplesStep, samplesOut);
}

void SubProcessor::setupSamplesIO(uint samplesIn, uint samplesStep, uint samplesOut)
{
	if (samplesIn < samplesStep)
	{	theIn = samplesStep;
		qDebug(	"*** WARNING: setupIO/setupSamplesIO(): samplesIn is less than samplesStep. Increasing samplesIn\n"
				"             to match samplesStep. Read subprocessor.h or the API docs for more\n"
				"             info.");
	}
	else
		theIn = samplesIn;
	theStep = samplesStep;
	theOut = samplesOut;
}

void SubProcessor::go()
{
	if (MESSAGES) qDebug("SubProc[%p]: go(): Initialising state...", theCoupling);
	theLoaded = false;
	theReturned = true;
	theStopping = false;
	theNoMoreTransactions = false;
	if (MESSAGES) qDebug("SubProc[%p]: go(): Starting... L: %d R: %d NMT: %d", theCoupling, theLoaded, theReturned, theNoMoreTransactions);
	start();
}

void SubProcessor::stop()
{
	if (MESSAGES) qDebug("SubProc[%p]: stop(): Stopping...", theCoupling);
	{	QMutexLocker lock(&theDataInUse);
		theStopping = true;
		theDataChanged.wakeAll();
	}
	wait();
	theStopping = false;
	if (MESSAGES)
	{	qDebug("SubProc[%p]: stop(): In BDs left: %d", this, theCurrentIn.size());
		for (uint i = 0; i < theCurrentIn.size(); i++)
			if (theCurrentIn.theData[i]) qDebug("%p", theCurrentIn[i].identity());
		qDebug("SubProc[%p]: stop(): Out BDs left: %d", this, theCurrentOut.size());
		for (uint i = 0; i < theCurrentOut.size(); i++)
			if (theCurrentOut.theData[i]) qDebug("%p", theCurrentOut[i].identity());
	}

	theCurrentIn.nullify();
	theCurrentIn.resize(0);
	theCurrentOut.nullify();
	theCurrentOut.resize(0);
}

void SubProcessor::setupVisual(uint width, uint height, uint redrawPeriod)
{
	if (thePrimaryOf)
		thePrimaryOf->setupVisual(width, height, redrawPeriod);
}

bool SubProcessor::paintProcessor(QPainter& _p, QSizeF const& _s) const
{
	if (!thePrimaryOf)
	{
		qWarning("WARNING: paintProcessor called on a non-primary Sub!");
		return false;
	}
	QRectF area(QPointF(0, 0), _s);
	_p.setPen(QColor(132, 132, 132));
	_p.setBrush(QColor(224, 224, 224));
	_p.drawRect(area);
	_p.setPen(Qt::black);
	_p.setFont(QFont("sans", _s.height(), 900, false));
	_p.drawText(area, Qt::AlignCenter, "?");
	return true;
}

PropertiesInfo SubProcessor::specifyProperties() const
{
	return PropertiesInfo();
}

void SubProcessor::processChunk(const BufferDatas &, BufferDatas &) const
{
	qFatal("*** FATAL: Missing SubProcessor processChunk implementation.");
}

void SubProcessor::processChunks(const BufferDatas &in, BufferDatas &out, uint chunks) const
{
	for (uint i = 0; i < chunks; i++)
	{	const BufferDatas ini = in.samples(i * theStep, theIn);
		BufferDatas outi = out.samples(i * theOut, theOut);
		processChunk(ini, outi);
	}
}

void SubProcessor::defineIO(uint numInputs, uint numOutputs)
{
	theNumInputs = numInputs;
	theNumOutputs = numOutputs;
}

void SubProcessor::run()
{
	QMutexLocker lock(&theDataInUse);
	while (!theStopping)
	{
		if (MESSAGES) qDebug("SubProc[%p]: SubProc running...", theCoupling);
		while (!theLoaded && !theStopping)
		{	//if (MESSAGES) qDebug("SubProc[%p]: Waiting for new transaction: L: %d R: %d", theCoupling, theLoaded, theReturned);
			theDataChanged.wait(&theDataInUse);
		}
		if (theStopping) { if (MESSAGES) qDebug("SubProc[%p]: Exiting...", theCoupling); return; }

		if (theChunks)
		{
			if (MESSAGES) qDebug("SubProc[%p]: Processing data of size %d samples (%d chunks)", theCoupling, theCurrentIn[0].samples(), theChunks);
			// Optimise by taking out into class
			QTime theTimer;
			theTimer.start();
			theCurrentOut.resize(theNumOutputs);
			for (uint i = 0; i < theNumOutputs; i++)
				theCurrentOut.setData(i, new BufferData(theOut * theChunks * theOutTypes.ptrAt(i)->scope(), theOutTypes.ptrAt(i)->scope()));
			processChunks(theCurrentIn, theCurrentOut, theChunks);
			if (MESSAGES) qDebug("SubProc[%p]: Data processed OK.", theCoupling);
			theTimeTaken = theTimer.elapsed();
		}
		else
		{	if (MESSAGES) qDebug("SubProc[%p]: Processing plunger", theCoupling);
			// We can assume that this transaction is merely a plunger. We'll just resize
			// the output so it's recognised as such.
			theCurrentOut.resize(0);
		}
		if (MESSAGES) qDebug("SubProc[%p]: Nullifying our input to free the data...", theCoupling);
		theCurrentIn.nullify();

		if (MESSAGES) qDebug("SubProc[%p]: Nullified. Changing state and iterating...", theCoupling);
		theLoaded = false;
		theReturned = false;
		theDataChanged.wakeAll();
		if (MESSAGES) qDebug("SubProc[%p]: New stuff ready! (L:%d, R:%d)", theCoupling, theLoaded, theReturned);
	}
}

void SubProcessor::transact(const BufferDatas &i, uint chunks)
{
	if (MESSAGES)
	{	if (chunks)
			qDebug("SubProcT[%p]: Transacting! (samples=%d)", theCoupling, i[0].samples());
		else
			qDebug("SubProcT[%p]: Transacting plunger!", theCoupling);
	}
	QMutexLocker lock(&theDataInUse);
	while (!(!theLoaded && theReturned) && !theNoMoreTransactions)
	{	if (MESSAGES) qDebug("SubProcT[%p]: Waiting for my transaction space: L: %d R: %d NMT: %d", theCoupling, theLoaded, theReturned, theNoMoreTransactions);
		theDataChanged.wait(&theDataInUse);
	}
	if (theNoMoreTransactions)
	{	if (MESSAGES) qDebug("SubProcT[%p]: Exiting - No more transactions allowed.", theCoupling);
		return;
	}
	if (MESSAGES) qDebug("SubProcT[%p]: Transferring transaction... L: %d R: %d NMT: %d", theCoupling, theLoaded, theReturned, theNoMoreTransactions);
	theLoaded = true;
	theReturned = false;
	theCurrentIn = i;
	theChunks = chunks;
	theDataChanged.wakeAll();
	if (MESSAGES) qDebug("SubProcT[%p]: All done.", theCoupling);
}

BufferDatas SubProcessor::deliverResults(uint *timeTaken)
{
	if (MESSAGES) qDebug("SubProcRR[%p]: Results wanted!", theCoupling);
	QMutexLocker lock(&theDataInUse);
	while (!(!theLoaded && !theReturned) && !theNoMoreTransactions)
	{	//if (MESSAGES) qDebug("SubProcRR[%p]: Waiting for some results: L: %d R: %d", theCoupling, theLoaded, theReturned);
		theDataChanged.wait(&theDataInUse);
	}
	if (theNoMoreTransactions)
	{	if (MESSAGES) qDebug("SubProcRR[%p]: Exiting - No more transactions allowed.", theCoupling);
		return BufferDatas();
	}
	if (MESSAGES)
	{	if (theCurrentOut.size())
			qDebug("SubProcRR[%p]: Results received! (samples=%d)", theCoupling, theCurrentOut[0].samples());
		else
			qDebug("SubProcRR[%p]: Result is plunger!", theCoupling);
	}
	theLoaded = false;
	theReturned = true;
//	if (thePlunged) qDebug("SubProcRR: Plunger returning...");
	theDataChanged.wakeAll();
	BufferDatas ret = theCurrentOut;
	theCurrentOut.nullify();
	if (MESSAGES) qDebug("SubProcRR[%p]: All done.", theCoupling);
	if (timeTaken) *timeTaken = theTimeTaken;
	return ret;
}

}

#undef MESSAGES

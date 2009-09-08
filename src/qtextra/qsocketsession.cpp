/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/

#include <cstdlib>

#include <QThread>

#include "qsocketsession.h"

#define MESSAGES 0

QSocketSession::QSocketSession(QTcpSocket *sd): theSD(sd)
{
	if (MESSAGES) qDebug("New QSocketSession...");
	theClosed = false;
	if (MESSAGES) qDebug("Done.");
}

QSocketSession::~QSocketSession()
{
	delete theSD;
}

void QSocketSession::close()
{
	if (theClosed) return;
	theClosed = true;
	if (theSD->isValid())
		theSD->close();
}

void QSocketSession::handshake()
{
	if (MESSAGES) qDebug("Handshaking... (isOpen()=%d)", isOpen());
	ack();
	if (!waitForAck(1000) || !isOpen())
	{	qWarning("*** ERROR: Handshake failed: Didn't receive ack. Closing link.");
		close();
		return;
	}
	if (MESSAGES) qDebug("Got ack!");
	for (bool know = false; !know;)
	{	uchar mi = rand() & 255;
		sendByte(mi);
		uchar i;
		if (!receiveChunk((uchar *)&i, 1, 1000) || !isOpen())
		{	qWarning("*** ERROR: Handshake failed: Didn't receive random. Closing link.");
			close();
			return;
		}
		if (mi != i)
		{	theIsMaster = mi > i;
			know = true;
		}
		else
		{	if (MESSAGES) qDebug("Doh! Number clash (%d = %d)", mi, i);
		}
	}
	findByteOrder();
}

void QSocketSession::handshake(bool opposite)
{

	if (MESSAGES) qDebug("Handshaking... (isOpen()=%d)", isOpen());
	ack();
	if (!waitForAck(1000) || !isOpen())
	{	qWarning("*** ERROR: Handshake failed: Didn't receive ack. Closing link.");
		close();
		return;
	}
	if (MESSAGES) qDebug("Got ack!");
	theIsMaster = opposite;
	findByteOrder();
}

void QSocketSession::findByteOrder()
{
	float i = 3.1415;
	if (theIsMaster)
	{	sendChunk((uchar *)&i, 4);
		if (!waitForAck(1000, &theSameByteOrder) || !isOpen())
		{	qWarning("*** ERROR: Handshake failed: Didn't receive ack. Closing link.");
			close();
			return;
		}
	}
	else
	{	float remotei;
		if (!receiveChunk((uchar *)&remotei, 4, 1000) || !isOpen())
		{	qWarning("*** ERROR: Handshake failed: Didn't receive remote bit pattern. Closing link.");
			close();
			return;
		}
		float rremotei;
		uint &t = *((uint *)&rremotei);
		t = bswap_32(*((uint *)&remotei));

		if (remotei == i)
			theSameByteOrder = true;
		else if (rremotei == i)
			theSameByteOrder = false;
		else
			qWarning("*** CRITICAL: Two hosts have neither same nor opposite byte order. (%f or %f should be %f)", remotei, rremotei, i);
		ack(theSameByteOrder);
	}
}

void QSocketSession::receiveChunk(uchar *buffer, uint size)
{
	int r = 1;
	uint read = 0;
	while (r > 0 && read < size && isOpen() && theSD->waitForReadyRead())
	{	r = theSD->read((char *)(buffer + read), size - read);
		read += r;
	}
	if (r <= 0 || read != size)
	{	qWarning("*** INFO: Socket receive error (read %d of %d). Closing connection.", read, size);
		close();
	}
}

void QSocketSession::sendChunk(const uchar *buffer, uint size)
{
	int r = 1;
	uint sent = 0;
	while (r > 0 && sent < size && isOpen())
	{	r = theSD->write((const char *)(buffer + sent), size - sent);
		theSD->waitForBytesWritten();
		sent += r;
	}
	if (sent != size)
	{	qWarning("*** INFO: Couldn't transmit data. Attempted to send %d bytes, sent %d."
				 "          Closing connection.", size, sent);
		close();
	}
}

bool QSocketSession::receiveChunk(uchar *buffer, uint size, uint timeOut)
{
	if (!isOpen()) return false;
	int r = 1;
	uint read = 0;
	bool timedOut = false;
	while (isOpen() && read < size && !timedOut)
	{	r = theSD->read((char *)(buffer + read), size - read);
		if (r <= 0)
		{	if (!theSD->waitForReadyRead(timeOut))
			{	if (MESSAGES) qDebug("Connection closed abruptly.");
				timedOut = true;
				close();
				break;
			}
		}
		else
			read += r;
	}

	if (timedOut)
		if (MESSAGES) qWarning("*** INFO: Timeout on socket receive (given %d, read %d of %d).\n"
							  "          Not closing.", timeOut, read, size);
	return !timedOut;
}

bool QSocketSession::waitForAck(uint timeOut, bool *ackType)
{
	uchar c = 0;
	while (isOpen() && c != 1 && c != 2)
		if (!receiveChunk(&c, 1, timeOut))
		{	if (MESSAGES) qWarning("*** INFO: Connection error - Timeout while waiting for Ack.");
			return false;
		}

	if (c != 1 && c != 2) return false;
	if (ackType) *ackType = c;
	return true;
}

bool QSocketSession::waitForAck(bool *ackType)
{
	uchar c = 0;
	while (isOpen() && c != 1 && c != 2)
		receiveChunk(&c, 1);

	if (c != 1 && c != 2) return false;
	if (ackType) *ackType = c;
	return true;
}

template<>
float QSocketSession::safeReceiveWord()
{
	union { int32_t i; float t; uchar c[4]; } d;
	receiveChunk(d.c, 4);
	if (!theSameByteOrder) d.i = bswap_32(d.i);
	return d.t;
}

template<>
int32_t QSocketSession::safeReceiveWord()
{
	union { int32_t i; uchar c[4]; } d;
	receiveChunk(d.c, 4);
	if (!theSameByteOrder) d.i = bswap_32(d.i);
	return d.i;
}

template<>
uint32_t QSocketSession::safeReceiveWord()
{
	union { int32_t i; uint32_t t; uchar c[4]; } d;
	receiveChunk(d.c, 4);
	if (!theSameByteOrder) d.i = bswap_32(d.i);
	return d.t;
}

template<>
void QSocketSession::safeReceiveWordArray(float *t, uint32_t size)
{
	int32_t *array = (int32_t *)t;
	receiveChunk((uchar *)array, 4 * size);
	if (!theSameByteOrder)
		for (uint32_t i = 0; i < size; i++)
			array[i] = bswap_32(array[i]);
}

template<>
void QSocketSession::safeReceiveWordArray(int32_t *t, uint32_t size)
{
	receiveChunk((uchar *)t, 4 * size);
	if (!theSameByteOrder)
		for (uint32_t i = 0; i < size; i++)
			t[i] = bswap_32(t[i]);
}

template<>
void QSocketSession::safeReceiveWordArray(uint32_t *t, uint32_t size)
{
	int32_t *array = (int32_t *)t;
	receiveChunk((uchar *)array, 4 * size);
	if (!theSameByteOrder)
		for (uint32_t i = 0; i < size; i++)
			array[i] = bswap_32(array[i]);
}

QByteArray QSocketSession::receiveString()
{
	int l = safeReceiveWord<uint32_t>();
	QByteArray s(l, ' ');
	receiveChunk((uchar *)s.data(), l);
	return s;
}

void QSocketSession::sendString(const QByteArray &s)
{
	safeSendWord((uint32_t)s.length());
	sendChunk((const uchar *)s.data(), s.length());
}


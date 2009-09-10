/***************************************************************************
 *   Copyright (C) 2003 by Gav Wood                                        *
 *   gav@kde.org                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 ***************************************************************************/
#ifndef __QSOCKETSESSION_H
#define __QSOCKETSESSION_H

// TODO: may need compatibility fixes.
#include <byteswap.h>
#include <stdint.h>

#include <QtNetwork>

#include <exscalibar.h>

/** @ingroup QtExtra
 * @brief Base utility class for dealing with Geddei socket connections.
 * @author Gav Wood <gav@kde.org>
 *
 * QSocketSession provides a suitable interface to TCP sockets for more
 * high-level uses. Through handshaking, it can determine if both hosts share
 * the same byte ordering for words, and through its "safe" methods can alter
 * transmissions accordingly transparently to the developer.
 *
 * It also provides other methods to listen on timeouts and and send higher
 * level data such as QCStrings.
 *
 * Communication methods do not return a status; to check if the data was sent
 * or received correctly, just check the value of isOpen() after the relevant
 * call.
 *
 * This is not thread-safe or reentrant.
 */
class DLLEXPORT QSocketSession
{
	bool theIsMaster, theSameByteOrder, theClosed;
	QTcpSocket *theSD;

	void findByteOrder();

public:
	/**
	 * @name Basic methods for use anytime.
	 *
	 * These methods are basic and take no notice of the word byte-ordering
	 * differences between hosts. they may be used before (and after)
	 * handshaking.
	 */
	//@{

	QTcpSocket* sd() const { return theSD; }

	/**
	 * Determine if the current connection is open.
	 *
	 * @return true if the current connection is open (i.e. connected).
	 */
	bool isOpen()
	{
		if (!theSD->isValid())
			theClosed = true;
		return !theClosed;
	}

	/**
	 * Close the current connection.
	 */
	void close();

	/**
	 * Send an acknowledgement byte through the connection. This matches the
	 * waitForAck() method.
	 *
	 * @param sign The sign of the acknowledgement. This is returned from the
	 * matching waitForAck() method on the other side of the connection.
	 *
	 * @sa waitForAck()
	 */
	void ack(bool sign = true) { sendByte(sign ? 1 : 2); }

	/**
	 * Sends a byte value down the connection. Matches receiveByte().
	 *
	 * @param c The value of the byte to be sent.
	 *
	 * @sa receiveByte()
	 */
	void sendByte(uchar c)
	{
		if (theSD->write((char *)&c, 1) == 1)
			return;
		qWarning("*** ERROR: Socket send error. Unable to send a byte.");
		close();
	}

	/**
	 * Send a number of bytes down the connection as one single entity. Matches
	 * receiveChunk().
	 *
	 * @param buffer An array of bytes to be sent.
	 * @param size The size of the array.
	 *
	 * @sa receiveChunk()
	 */
	void sendChunk(const uchar *buffer, uint size);

	/**
	 * Receive a single byte from the connection. Matches sendByte().
	 *
	 * This will block until either the connection fails or the byte is
	 * received.
	 *
	 * @return A byte read from the connection. 0 if the connection is
	 * not open by the time this returns.
	 *
	 * @sa sendByte()
	 */
	uchar receiveByte()
	{
		uchar c;
		if (theSD->read((char *)&c, 1) == 1)
			return c;
		qWarning("***  ERROR: Socket receive error. Unable to read a byte.");
		close();
		return 0;
	}

	/**
	 * Receive some number of bytes from the connection. The number received is
	 * stated in @a size.
	 *
	 * This will block until either the connection fails or the number of bytes
	 * is received.
	 *
	 * @param buffer The array of bytes that the data is to be written into.
	 * Must be at least @a size big.
	 * @param size The number of bytes to be read from the connection.
	 *
	 * @sa sendChunk()
	 */
	void receiveChunk(uchar *buffer, uint size);

	/** @overload
	 * Receive some number of bytes from the connection. The number received is
	 * stated in @a size.
	 *
	 * This will block until either the connection fails or the number of bytes
	 * is received, or the timeout @a timeOut is reached.
	 *
	 * @param buffer The array of bytes that the data is to be written into.
	 * Must be at least @a size big.
	 * @param size The number of bytes to be read from the connection.
	 * @param timeOut Number of milliseconds to wait before giving up.
	 * @return true if all was received ok, false if there was a timeout or
	 * problem with the connection.
	 *
	 * @sa sendChunk()
	 */
	bool receiveChunk(uchar *buffer, uint size, uint timeOut);

	/**
	 * Block until the next communication is received. It is interpreted as an
	 * acknowledgement sent by ack().
	 *
	 * This is one of the few receiving methods that provides you with a return
	 * value which determines if the communication went ok.
	 *
	 * @param ackType If non-0, the sign of the send acknowledgement is
	 * populated into boolean here.
	 * @return true if the ack was received ok, false if there was a problem
	 * with the connection.
	 *
	 * @sa ack()
	 */
	bool waitForAck(bool *ackType = 0);

	/**
	 * Block until the next communication is received. It is interpreted as an
	 * acknowledgement sent by ack().
	 *
	 * This will block until either the connection fails or the number of bytes
	 * is received, or the timeout @a timeOut is reached.
	 *
	 * @param timeOut Number of milliseconds to wait before giving up.
	 * @param ackType If non-0, the sign of the send acknowledgement is
	 * populated into boolean here.
	 * @return true if the ack was received ok, false if there was a timeout or
	 * problem with the connection.
	 *
	 * @sa ack()
	 */
	bool waitForAck(uint timeOut, bool *ackType = 0);

	//@}

	/**
	 * @name Advanced methods for use after handshaking.
	 *
	 * These are advanced methods that take notice of any differences between
	 * the two hosts byte ordering, altering data if necessary to compensate.
	 *
	 * They must only ever be used after a successful handshake.
	 *
	 * For brevity 32bit word has been shortened to Word in the names.
	 * Though the type given is an int, floats may be used in their place.
	 */
	//@{

	/**
	 * Conduct a handshake operation. This should be done when ever possible at
	 * the start of a connection.
	 *
	 * It is always best to use this when ever possible. On asymmetric
	 * connections it is easy to make sure that you can guarantee both sides'
	 * opposite value will be different. Otherwise you may need to use the
	 * overloaded version.
	 *
	 * @param opposite This must be true on one side of the handshake and false
	 * on the other. It won't work otherwise.
	 */
	void handshake(bool opposite);

	/** @overload
	 * Conduct a handshake operation. This should be done when ever possible at
	 * the start of a connection.
	 *
	 * This assumes that srand() has been set up with a REALLY unpredictable
	 * value. If not, this method may loop infinitely, causing your program to
	 * crash.
	 *
	 * Only ever use this on symmetric connections where either node has been
	 * properly seeded (with something unique like pid, datetime and ip).
	 */
	void handshake();

	/**
	 * Determine if the two hosts share the same word byte-ordering.
	 *
	 * @return true if they share the same byte-ordering.
	 */
	bool sameByteOrder() const { return theSameByteOrder; }

	/**
	 * Send a single word down the connection. This can be one of int, float
	 * or uint. Undefined action for any other types used.
	 *
	 * @param i The word to be sent down.
	 *
	 * @sa safeReceiveWord()
	 */
	void safeSendWord(float i) { sendChunk((const uchar *)&i, 4); }
	void safeSendWord(int32_t i) { sendChunk((const uchar *)&i, 4); }
	void safeSendWord(uint32_t i) { sendChunk((const uchar *)&i, 4); }

	/**
	 * Send a number of words down the connection. These can be one of int,
	 * float or uint. Undefined action for any other types used.
	 *
	 * @param i The array of words. Must be at least @a size big.
	 * @param size The number of values to be sent.
	 *
	 * @sa safeReceiveWordArray()
	 */
	void safeSendWordArray(const float *i, uint size) { sendChunk((const uchar *)i, 4 * size); }
	void safeSendWordArray(const int32_t *i, uint size) { sendChunk((const uchar *)i, 4 * size); }
	void safeSendWordArray(const uint32_t *i, uint size) { sendChunk((const uchar *)i, 4 * size); }

	/**
	 * Receive a single word from the connection. The word type must be
	 * given as the template parameter and may be one of float, int32_t or
	 * uint32_t. This must correspond to the type sent with safeSendWord().
	 *
	 * This will block until either the connection fails or the number of words
	 * is received.
	 *
	 * @return The word received from the connection. Undefined if isOpen()
	 * returns false, or if the corresponding word sent was a float.
	 *
	 * @sa safeSendWord()
	 */
	template<typename T> T safeReceiveWord() { return T(); }

	/**
	 * Receive a number of words from the connection. The word type may be one
	 * of float, int or uint. This must correspond to the type sent with
	 * safeSendWord().
	 *
	 * This will block until either the connection fails or the number of words
	 * is received.
	 *
	 * @param i The array of words that the data is to be written into.
	 * Must be at least @a size big.
	 * @param size The number of words to be read from the connection.
	 *
	 * @sa safeSendWordArray()
	 */
	template<typename T> void safeReceiveWordArray(T *t, uint32_t size) {}

	/**
	 * Send a string value down the connection.
	 *
	 * @param s The QCString value to be sent.
	 */
	void sendString(const QByteArray &s);

	/**
	 * Receive a string value from the connection.
	 *
	 * This will block until either the connection fails or the number of bytes
	 * is received.
	 *
	 * @return The sent string.
	 */
	QByteArray receiveString();
	//@}

	/**
	 * Basic constructor.
	 *
	 * @param sd The QSocketDevice this session will use to do its
	 * communications. It will adopt this and thus destroy it when this object
	 * gets destroyed. Deleting @a sd yourself will result in a memory error.
	 */
	QSocketSession(QTcpSocket *sd);

	/**
	 * Safe destructor.
	 */
	~QSocketSession();
};

template<> float QSocketSession::safeReceiveWord();
template<> int32_t QSocketSession::safeReceiveWord();
template<> uint32_t QSocketSession::safeReceiveWord();
template<> void QSocketSession::safeReceiveWordArray(float *t, uint32_t size);
template<> void QSocketSession::safeReceiveWordArray(int32_t *t, uint32_t size);
template<> void QSocketSession::safeReceiveWordArray(uint32_t *t, uint32_t size);

#endif

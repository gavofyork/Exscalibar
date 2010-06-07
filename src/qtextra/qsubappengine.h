/* Copyright 2003, 2004, 2005, 2007, 2009 Gavin Wood <gav@kde.org>
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

#include <QThread>
#include <QTimer>

#include <exscalibar.h>

/** @ingroup QtExtra
 * @brief Implement to give non-event driven programs use of the Qt event loop.
 * @author Gav Wood <gav@kde.org>
 *
 * QSubApp is a class for helping provide a convenient way to encapsulate the
 * idea of a normal C++ (non-event driven) style program while still retaining
 * the Qt event loop.
 *
 * Subclass QSubApp and reimplement the main() method, creating one globalscope
 * instance on it.
 *
 * Example:
 *
 * @code
 * class MySubApp: public QSubApp
 * {
 *     void main();
 * } theSubApp;
 *
 * void MySubApp::main()
 * {
 *     // Do some stuff.
 * }
 * @endcode
 */
class DLLEXPORT QSubApp : public QThread
{
Q_OBJECT
	friend class QLauncher;
	static QSubApp *theSubApp;
	virtual void run();

protected:
	/**
	 * Reimplement to define the main()-like function for your program.
	 */
	virtual void main() = 0;

	/**
	 * The default and basic constructor.
	 */
	QSubApp();

public:
	/**
	 * Block for at least @a secs seconds.
	 *
	 * @param Minimum number of seconds to block for.
	 */
	static void sleep(uint secs) { sleep(secs); }

	/**
	 * Block for at least @a usecs microseconds.
	 *
	 * @param Minimum number of microseconds to block for.
	 */
	static void usleep(uint usecs) { usleep(usecs); }
};

class QLauncher : public QTimer
{
Q_OBJECT
public slots:
	void go() { if (QSubApp::theSubApp) QSubApp::theSubApp->start(); }
};

// Basically, the main() of a normal Qt program, except we can't call it that
// since the real main has to be inside the code that uses SubApp.
DLLEXPORT int main_stub(int argc, char **argv);


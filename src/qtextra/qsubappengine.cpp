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

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QApplication>
#include <QEvent>

#include <exscalibar.h>

class QLauncher;

class DLLEXPORT QSubApp : public QThread
{
Q_OBJECT

	friend class QLauncher;
	static QSubApp *theSubApp;
	virtual void run();

protected:
	virtual void main() = 0;
	QSubApp();
};

class QLauncher : public QTimer
{
Q_OBJECT
public slots:
	void go() { if (QSubApp::theSubApp) QSubApp::theSubApp->start(); }
};

QSubApp *QSubApp::theSubApp = 0;

QSubApp::QSubApp()
{
	theSubApp = this;
}

void QSubApp::run()
{
	main();
	QApplication::postEvent(qApp, new QEvent(QEvent::Quit));
}

DLLEXPORT int main_stub(int argc, char **argv)
{
	QApplication app(argc, argv, false);
	QLauncher t;
	QTimer::singleShot(0, &t, SLOT(go()));
	app.exec();
	return 0;
}


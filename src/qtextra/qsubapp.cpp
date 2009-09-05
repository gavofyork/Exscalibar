/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@kde.org                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QObject>
#include <qthread.h>
#include <qtimer.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QEvent>

#include <exscalibar.h>

#include "qsubappengine.h"

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


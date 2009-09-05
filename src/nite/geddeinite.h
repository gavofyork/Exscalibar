/***************************************************************************
 *   Copyright (C) 2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
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

#ifndef GEDDEINITE_H
#define GEDDEINITE_H

#define __GEDDEI_BUILD

//Added by qt3to4:
#include <QCloseEvent>
#include <QMainWindow>
#include <QDockWidget>

#include "processorgroup.h"
using namespace Geddei;

#include <q3valuelist.h>
#include <q3ptrlist.h>
#include <qpoint.h>

#include "ui_geddeinitebase.h"

class Q3DockWindow;
class Q3Table;
class Q3Canvas;
class Q3CanvasItem;

class BobsView;
class Bob;
class WatchProcessor;
class ProcessorView;

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class GeddeiNite: public QMainWindow, public Ui::GeddeiNiteBase
{
	Q_OBJECT

public:
	GeddeiNite();
	~GeddeiNite();

	const QString	makeUniqueName(const QString &type);
	void			setModified(bool modified = true);

	bool			tested() const { return theTested; }
	bool			connected() const { return theConnected; }

	ProcessorGroup&	group() { return theGroup; }

protected:
	void			closeEvent(QCloseEvent *);

private slots:
	void			slotPropertyChanged(QTableWidgetItem* _i);

	void			on_modeRun_toggled(bool testing);
	void			on_fileOpen_activated();
	void			on_fileSave_activated();
	void			on_fileSaveAs_activated();
	void			on_fileExit_activated() { qApp->exit(); }
	void			on_editRemove_activated();
	void			on_toolsDeployPlayer_activated();
	void			on_modeTest_activated();

private:
	void			doSave(const QString &filename);
	void			doLoad(const QString &filename);
	bool			connectAll();
	void			disconnectAll();
	void			updateItems();
	void			updateProperties();

	QGraphicsScene	theScene;
	ProcessorGroup	theGroup;

	QString			theFilename;

	bool			theUpdatingProperties;
	bool			theRunning;
	bool			theTested;
	bool			theConnected;
	bool			theIgnoreNext;
	bool			theModified;
};

#endif

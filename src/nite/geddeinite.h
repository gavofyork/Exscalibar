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
 * @author Gav Wood <gav@cs.york.ac.uk>
 */
class GeddeiNite: public QMainWindow, public Ui::GeddeiNiteBase
{
	Q_OBJECT

	QString theFilename;

	QPrinter *printer;
	QString filename;
	Q3Canvas *theCanvas;
	Q3CanvasItem *theActive;
	QDockWidget *theDockSelector, *theDockProperties;
	Q3Table *theProperties;
	ProcessorView *theSelector;
	WatchProcessor *theWatch;
	Q3PtrList<Bob> theBobs;
	ProcessorGroup theGroup;
	bool theRunning, theTested, theConnected, theIgnoreNext, theModified;
	int theDefaultBufferSize;

	void updateItems();
	void doSave(const QString &filename);
	void doLoad(const QString &filename);
	bool connectAll();
	void disconnectAll();

	void updateProperties();

private slots:
	void slotPropertyChanged(int row, int column);

	void on_modeRun_toggled(bool testing);
	void on_fileOpen_activated();
	void on_fileSave_activated();
	void on_fileSaveAs_activated();
	void on_filePrint_activated();
	void on_editRemove_activated();
	void on_toolsDeployPlayer_activated();
	void on_modeTest_activated();

protected:
	void closeEvent(QCloseEvent *);

public:
	const QString makeUniqueName(const QString &type);
	void setActive(Q3CanvasItem *item = NULL);
	void setModified(bool modified = true);

	int defaultBufferSize() const { return theDefaultBufferSize; }
	bool tested() const { return theTested; }
	bool connected() const { return theConnected; }

	ProcessorGroup &group() { return theGroup; }
	WatchProcessor *watch() { return theWatch; }

	bool bobCollision(Bob *b);
	void addBob(Bob *b);
	void removeBob(Bob *b);
	Bob *getBob(const QString &name);

	GeddeiNite();
	~GeddeiNite();
};

#endif

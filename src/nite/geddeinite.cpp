/***************************************************************************
 *   Copyright(C)2004 by Gav Wood                                        *
 *   gav@cs.york.ac.uk                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *  (at your option)any later version.                                   *
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

#define __GEDDEI_BUILD

#include "geddeinite.h"

#include <cassert>
#include <iostream>
using namespace std;

#include "processor.h"
#include "processorfactory.h"
#include "subprocessorfactory.h"
#include "buffer.h"
#include "bufferdata.h"
using namespace Geddei;

#include <QtXml>
#include <qpixmap.h>
#include <qfile.h>
#include <q3filedialog.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <q3textstream.h>
#include <qpainter.h>
#include <q3paintdevicemetrics.h>
#include <q3canvas.h>
#include <q3dockwindow.h>
#include <q3table.h>
#include <qaction.h>
#include <qdom.h>
//Added by qt3to4:
#include <QCloseEvent>
#include <Q3PtrList>

#include "bobport.h"
#include "boblink.h"
#include "bobsview.h"
#include "watchprocessor.h"
#include "processorview.h"
#include "floatinglink.h"
#include "player.h"
#include "softbob.h"
#include "domsoftbob.h"
#include "hardbob.h"

GeddeiNite::GeddeiNite(): QMainWindow(0, "GeddeiNite", Qt::WDestructiveClose), theConnected(false)
{
	setupUi(this);

	theDockProperties = new QDockWidget("Properties", this);
//	theDockProperties->setResizeEnabled(true);
//	theDockProperties->setVerticallyStretchable(true);
//	theDockProperties->setHorizontallyStretchable(true);
	addDockWidget(Qt::BottomDockWidgetArea, theDockProperties);

	theProperties = new Q3Table(0, 1, theDockProperties);
	theProperties->setTopMargin(0);
	theProperties->horizontalHeader()->hide();
	theDockProperties->setWidget(theProperties);
	theProperties->adjustSize();
	theProperties->setLeftMargin(theDockProperties->width() / 2);
	theProperties->verticalHeader()->setResizeEnabled(false);
	theProperties->verticalHeader()->setStretchEnabled(false);
	connect(theProperties, SIGNAL(valueChanged(int, int)), this, SLOT(slotPropertyChanged(int, int)));
	theDockProperties->show();

	theDockSelector = new QDockWidget("Available Processors", this);
//	theDockSelector->setResizeEnabled(true);
//	theDockSelector->setVerticallyStretchable(true);
//	theDockSelector->setHorizontallyStretchable(true);
	addDockWidget(Qt::RightDockWidgetArea, theDockSelector);

	theSelector = new ProcessorView(theDockSelector);
	theDockSelector->setWidget(theSelector);
	theDockSelector->show();

	updateItems();

	theCanvas = new Q3Canvas(this);
	theCanvas->resize(1600, 1200);
	theView->setCanvas(theCanvas);
	theView->setBackgroundColor(QColor(255, 255, 255));
	theView->setAcceptDrops(true);

	theActive = 0;
	theRunning = false;
	theIgnoreNext = false;
	theDefaultBufferSize = 262144;
	theFilename = "";
	theModified = false;

	setModified(false);
	statusBar()->message(tr("Ready"), 2000);
}

GeddeiNite::~GeddeiNite()
{
	if(modeRun->isOn())
		on_modeRun_toggled(false);
	Q3PtrList<Bob> bobs = theBobs;	// need copy as ~Bob() will alter theBobs.
	for(Q3PtrList<Bob>::iterator i = bobs.begin(); i != bobs.end(); i++) delete *i;
}

void GeddeiNite::on_filePrint_activated()
{
	updateItems();

}

const QString GeddeiNite::makeUniqueName(const QString &type)
{
	QString ret = "";
	for(int i = 1; theGroup.exists(ret = type + QString().setNum(i)); i++) {}
	return ret;
}

bool GeddeiNite::bobCollision(Bob *b)
{
	for(Q3PtrList<Bob>::iterator i = theBobs.begin(); i != theBobs.end(); i++)
		if(b->collidesWith(*i) && b != *i)
			return true;
	return false;
}

void GeddeiNite::addBob(Bob *b)
{
	theBobs.append(b);
}

void GeddeiNite::removeBob(Bob *b)
{
	theBobs.remove(b);
}

Bob *GeddeiNite::getBob(const QString &name)
{
	for(Q3PtrList<Bob>::iterator i = theBobs.begin(); i != theBobs.end(); i++)
		if((*i)->name() == name) return *i;
	return 0;
}

void GeddeiNite::doSave(const QString &filename)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("network");
	doc.appendChild(root);
	for(Q3PtrList<Bob>::iterator i = theBobs.begin(); i != theBobs.end(); i++)
	{	QDomElement proc;
		if(dynamic_cast<DomSoftBob *>(*i))
			proc = doc.createElement("subprocessor");
		else if(dynamic_cast<SoftBob *>(*i))
			proc = doc.createElement("processor");
		else
			proc = doc.createElement("builtin");
		root.appendChild(proc);
		(*i)->saveYourself(proc, doc);
	}

	QFile f(filename);
	if(!f.open(QIODevice::WriteOnly))
	{	statusBar()->message("Couldn't write to " + filename, 2000);
		return;
	}
	Q3TextStream out(&f);
	out << doc.toString();
	statusBar()->message("Saved.", 2000);
	setModified(false);
}

void GeddeiNite::doLoad(const QString &filename)
{
	QFile f(filename);
	if(!f.open(QIODevice::ReadOnly))
	{	statusBar()->message("Couldn't read from " + filename, 2000);
		return;
	}
	QDomDocument doc;
	doc.setContent(&f, false);

	QDomElement root = doc.documentElement();
	for(QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement elem = n.toElement();
		if(elem.isNull()) continue;
		if(elem.tagName() == "processor")
			(new SoftBob(0, 0, elem.attribute("type"), elem.attribute("name"), theCanvas))->loadYourselfPre(elem);
		else if(elem.tagName() == "subprocessor")
			(new DomSoftBob(0, 0, elem.attribute("type"), elem.attribute("name"), theCanvas))->loadYourselfPre(elem);
		else if(elem.tagName() == "builtin")
		{	if(elem.attribute("type") == "Player")
				(new HardBob(0, 0, elem.attribute("name"), theCanvas, new Player(elem.attribute("path"))))->loadYourselfPre(elem);
		}
	}
	for(QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement elem = n.toElement();
		if(elem.isNull()) continue;
		if(elem.tagName() == "processor" || elem.tagName() == "subprocessor" || elem.tagName() == "builtin")
			getBob(elem.attribute("name"))->loadYourselfPost(elem);
	}
	statusBar()->message("Loaded.", 2000);
	setModified(false);
}

void GeddeiNite::setModified(bool modified)
{
	theModified = modified;
	if(modified)
	{	theTested = false;
		modeRun->setEnabled(false);
	}
	setCaption((theFilename.isEmpty() ? "Untitled" : theFilename) + (modified ? " [ Modified ]" : "") + " - Geddei Nite");
}

void GeddeiNite::setActive(Q3CanvasItem *active)
{
	//TODO: need to check item type and update() it if neccessary
	if(active == theActive) return;
	if(theActive) { theActive->setActive(false); dynamic_cast<Refresher *>(theActive)->refresh(); }
	theActive = active;
	if(theActive) { theActive->setActive(true); dynamic_cast<Refresher *>(theActive)->refresh(); }
	update();
	updateProperties();
}

void GeddeiNite::updateItems()
{
	theSelector->clear();
	{	QStringList classes = ProcessorFactory::available();
		for(QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	Q3ListViewItem *item = new Q3ListViewItem(theSelector, *i, "processor");
			//item->setPixmap(0, QPixmap((const char **)processor));
			item->setDragEnabled(true);
		}
	}
	{	QStringList classes = SubProcessorFactory::available();
		for(QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	Q3ListViewItem *item = new Q3ListViewItem(theSelector, *i, "subprocessor");
			//item->setPixmap(0, QPixmap((const char **)subprocessor));
			item->setDragEnabled(true);
		}
	}
}

void GeddeiNite::updateProperties()
{
	if(theActive)
	{
		if(dynamic_cast<SoftBob *>(theActive))
		{	Properties p(dynamic_cast<SoftBob *>(theActive)->theProperties);
			theProperties->setNumRows(p.size() + 1);
			theProperties->verticalHeader()->setLabel(0, "Name");
			theProperties->setText(0, 0, dynamic_cast<Bob *>(theActive)->name());
			for(uint i = 0; i < p.size(); i++)
			{
				theProperties->verticalHeader()->setLabel(i + 1, p.keys()[i]);
				theProperties->setText(i + 1, 0, p[p.keys()[i]].toString());
			}
		}
		else if(dynamic_cast<BobLink *>(theActive))
		{	BobLink *link = dynamic_cast<BobLink *>(theActive);
			theProperties->setNumRows(2);
			theProperties->verticalHeader()->setLabel(0, "Buffer Size");
			theProperties->setText(0, 0, QString::number(link->bufferSize()));
			theProperties->verticalHeader()->setLabel(1, "Proximity");
			theProperties->setText(1, 0, QString::number(link->proximity()));
		}
		else
			theProperties->setNumRows(0);
	}
	else
		theProperties->setNumRows(0);
}

void GeddeiNite::on_fileSave_activated()
{
	if(theFilename.isEmpty())
	{	on_fileSaveAs_activated();
		return;
	}

	statusBar()->message("Saving...");
	doSave(theFilename);
}

void GeddeiNite::on_fileOpen_activated()
{
	QString filename = Q3FileDialog::getOpenFileName(QString::null, QString::null, this);
	if(!filename.isEmpty())
	{	if(theFilename.isEmpty() && !theModified)
		{	theFilename = filename;
			doLoad(theFilename);
		}
		else
		{	GeddeiNite *g = new GeddeiNite;
			g->theFilename = filename;
			g->doLoad(g->theFilename);
			g->show();
		}
	}
}

void GeddeiNite::on_fileSaveAs_activated()
{
	QString filename = Q3FileDialog::getSaveFileName(QString::null, QString::null, this);

	if(!filename.isEmpty())
	{	theFilename = filename;
		on_fileSave_activated();
	}
	else
		statusBar()->message("Save aborted.");
}

void GeddeiNite::on_editRemove_activated()
{
	if(!theActive) return;
	Q3CanvasItem *theActive = GeddeiNite::theActive;
	setActive();
	if(dynamic_cast<BobLink *>(theActive))
	{	delete dynamic_cast<BobLink *>(theActive);
		theCanvas->update();
		setModified(true);
	}
	else if(dynamic_cast<SoftBob *>(theActive))
	{
		delete dynamic_cast<SoftBob *>(theActive);
		theCanvas->update();
		setModified(true);
	}
	else if(dynamic_cast<HardBob *>(theActive))
	{
		delete dynamic_cast<HardBob *>(theActive);
		theCanvas->update();
		setModified(true);
	}
	else
		statusBar()->message("Cannot delete this object: You didn't create it.", 2000);
}

void GeddeiNite::slotPropertyChanged(int row, int column)
{
	assert(theActive);
	if(dynamic_cast<SoftBob *>(theActive))
	{	Properties &p(dynamic_cast<SoftBob *>(theActive)->theProperties);
		if(row > 0)
		{	p.set(theProperties->verticalHeader()->label(row), theProperties->text(row, column));
			dynamic_cast<SoftBob *>(theActive)->propertiesChanged();
		}
		else
			if(theGroup.exists(theProperties->text(row, column)))
			{	statusBar()->message("This name is already in use. Try another.", 2000);
				theProperties->setText(row, column, dynamic_cast<Bob *>(theActive)->name());
			}
			else
				dynamic_cast<Bob *>(theActive)->setName(theProperties->text(row, column));
	}
	else if(dynamic_cast<BobLink *>(theActive))
	{	BobLink *link = dynamic_cast<BobLink *>(theActive);
		switch(row)
		{	case 0: link->setBufferSize(theProperties->text(row, column).toUInt()); break;
			case 1: link->setProximity(theProperties->text(row, column).toUInt()); break;
			default: ;
		}
	}
	setModified(true);
}

void GeddeiNite::on_modeTest_activated()
{
	theTested = connectAll();
	if(theTested)
	{	disconnectAll();
		// TODO: disable test button.
	}
	modeRun->setEnabled(theTested);
}

bool GeddeiNite::connectAll()
{
	bool successful = true;
	theWatch = 0;
	if(theActive)
		if(dynamic_cast<BobLink *>(theActive) || (dynamic_cast<BobPort *>(theActive) && !dynamic_cast<BobPort *>(theActive)->isInput()))
		{	theWatch = new WatchProcessor(this);
			theWatch->init("__Watch");
		}

	for(Q3PtrList<Bob>::iterator i = theBobs.begin(); i != theBobs.end(); i++)
		if(!(*i)->connectYourself()) { successful = false; break; }

	if(!successful)
	{	// error message, restore to stable state.
		statusBar()->message("Problem creating connections.");
		theCanvas->update();
		disconnectAll();
		return false;
	}
	successful = theGroup.confirmTypes();
	if(!successful)
	{	// error message, restore to stable state.
		statusBar()->message("Problem confirming types.");
		theCanvas->update();
		disconnectAll();
		return false;
	}
	theConnected = true;
	return true;
}

void GeddeiNite::disconnectAll()
{
	theConnected = false;
	for(Q3PtrList<Bob>::iterator i = theBobs.begin(); i != theBobs.end(); i++)
		(*i)->disconnectYourself();

	if(theWatch)
	{
		theWatch->disconnectAll();
		delete theWatch;
		theWatch = 0;
	}
}

void GeddeiNite::on_toolsDeployPlayer_activated()
{
	QString filename = Q3FileDialog::getOpenFileName("/tmp", "Uncompressed audio (*.wav);;All files (*)", this);
	if(!filename.isEmpty())
		new HardBob(100, 60, filename, theCanvas, new Player(filename));
}

void GeddeiNite::on_modeRun_toggled(bool running)
{
	if(running == theRunning) { theIgnoreNext = true; return; }
	if(theIgnoreNext) { theIgnoreNext = false; return; }
	assert(theTested);

	theRunning = running;
	if(theRunning)
	{	connectAll();
		qDebug("All connected - starting watch...");
		if(theWatch) theWatch->go();
		qDebug("OK - starting...");
		bool successful = theGroup.go();
		qDebug("Started.");
//		if(successful) successful = theGroup.waitUntilGoing() == Processor::None;
		if(!successful)
		{
			// some sort of cool depiction of the error.
			// restore to stable state.
			statusBar()->message("Problem starting processors.");
			theCanvas->update();
			return;
		}
		// start some sort of monitor...
		qDebug("Exitting...");
	}
	else if(!running)
	{	if(theWatch) theWatch->stop();
		theGroup.stop(false);
		theWatch->reset();
		theGroup.reset();
		disconnectAll();
	}
	theCanvas->update();
}

void GeddeiNite::closeEvent(QCloseEvent *e)
{
	if(theModified)
	{}	// TODO: ask if want to save
	e->accept();
}

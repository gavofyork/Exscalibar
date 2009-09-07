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

#include <cassert>
#include <iostream>
using namespace std;

#include "processoritem.h"
#include "processorview.h"
#include "processorsview.h"
#include "geddeinite.h"

GeddeiNite::GeddeiNite():
	QMainWindow				(0, "GeddeiNite", Qt::WDestructiveClose),
	theRunning				(false),
	theConnected			(false),
	theIgnoreNext			(false),
	theModified				(false)
{
	setupUi(this);
	connect(theProperties, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slotPropertyChanged(QTableWidgetItem*)));
	connect(&theScene, SIGNAL(selectionChanged()), this, SLOT(slotUpdateProperties()));
	connect(&theScene, SIGNAL(changed()), this, SLOT(slotChanged()));
	updateItems();

	theView->setAcceptDrops(true);
	theView->setScene(&theScene);

	setModified(false);

	QSettings s;
	restoreState(s.value("mainwindow/state").toByteArray());
	restoreGeometry(s.value("mainwindow/geometry").toByteArray());

	if (QFile::exists(s.value("mainwindow/lastproject").toString()))
		doLoad(s.value("mainwindow/lastproject").toString());
	statusBar()->message(tr("Ready"), 2000);

	slotUpdateProperties();
}

GeddeiNite::~GeddeiNite()
{
	if (modeRun->isOn())
		on_modeRun_toggled(false);

	QSettings s;
	s.setValue("mainwindow/state", saveState());
	s.setValue("mainwindow/geometry", saveGeometry());
	s.setValue("mainwindow/lastproject", theFilename);
}

const QString GeddeiNite::makeUniqueName(const QString &type)
{
	QString ret = "";
	for (int i = 1; theGroup.exists(ret = type + QString().setNum(i)); i++) {}
	return ret;
}

void GeddeiNite::doSave(const QString& _filename)
{
	QDomDocument doc;
	QDomElement root = doc.createElement("network");
	doc.appendChild(root);
	foreach (QGraphicsItem* i, theScene.items())
		if (ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i))
			pi->saveYourself(root, doc);

	QFile f(_filename);
	if (!f.open(QIODevice::WriteOnly))
	{	statusBar()->message("Couldn't write to " + _filename, 2000);
		return;
	}
	QTextStream out(&f);
	out << doc.toString();
	statusBar()->message("Saved.", 2000);
	setModified(false);
}

void GeddeiNite::doLoad(const QString &filename)
{
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly))
	{	statusBar()->message("Couldn't read from " + filename, 2000);
		return;
	}
	QDomDocument doc;
	doc.setContent(&f, false);

	theFilename = filename;

	QDomElement root = doc.documentElement();
	for (QDomNode n = root.firstChild(); !n.isNull(); n = n.nextSibling())
	{	QDomElement elem = n.toElement();
		if (elem.isNull())
			continue;
		else if (elem.tagName() == "processor")
			ProcessorItem::fromDom(elem, &theScene);
	}
	statusBar()->message("Loaded.", 2000);
	setModified(false);
}

void GeddeiNite::setModified(bool modified)
{
	theModified = modified;
	if (modified)
	{
		modeRun->setEnabled(connectAll());
		disconnectAll();
	}
	setCaption((theFilename.isEmpty() ? "Untitled" : theFilename) + (modified ? " [ Modified ]" : "") + " - Geddei Nite");
}

void GeddeiNite::updateItems()
{
	theProcessors->clear();
	{	QStringList classes = ProcessorFactory::available();
		for (QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	QListWidgetItem *item = new QListWidgetItem(*i, theProcessors, QListWidgetItem::UserType);
			item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
	}
	{	QStringList classes = SubProcessorFactory::available();
		for (QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	QListWidgetItem *item = new QListWidgetItem(*i, theProcessors, QListWidgetItem::UserType + 1);
			item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
	}
}

void GeddeiNite::slotUpdateProperties()
{
	theUpdatingProperties = true;
	QGraphicsItem* i = theScene.selectedItems().size() ? theScene.selectedItems()[0] : 0;
	if (i && qgraphicsitem_cast<ProcessorItem*>(i))
	{
		ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i);
		Properties const& p(pi->properties());
		theProperties->setRowCount(p.size());
		for (uint i = 0; i < p.size(); i++)
		{
			theProperties->setVerticalHeaderItem(i, new QTableWidgetItem(p.keys()[i]));
			theProperties->setItem(i, 0, new QTableWidgetItem(p[p.keys()[i]].toString()));
		}
		theProperties->resizeColumnsToContents();
		theProperties->setEnabled(true);
	}
/*	else if (i && qgraphicsitem_cast<ConnectionItem*>(i))
	{
		BobLink *link = dynamic_cast<BobLink *>(theActive);
		theProperties->setRowCount(2);
		theProperties->setItem(0, 0, new QTableWidgetItem(QString::number(link->bufferSize())));
		theProperties->setVerticalHeaderItem(0, new QTableWidgetItem("Buffer Size"));
		theProperties->setItem(1, 0, new QTableWidgetItem(QString::number(link->proximity())));
		theProperties->setVerticalHeaderItem(1, new QTableWidgetItem("Proximity"));
		theProperties->resizeColumnsToContents();
		theProperties->setEnabled(true);
	}*/
	else
	{
		theProperties->clear();
		theProperties->setRowCount(1);
		theProperties->setEnabled(false);
	}
	theUpdatingProperties = false;
	theProperties->update();
}

void GeddeiNite::on_fileSave_activated()
{
	if (theFilename.isEmpty())
	{	on_fileSaveAs_activated();
		return;
	}

	statusBar()->message("Saving...");
	doSave(theFilename);
}

void GeddeiNite::on_fileOpen_activated()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open Processor Network", "/tmp", "XML (*.xml);;All files (*)");
	if (!filename.isEmpty())
	{	if (theFilename.isEmpty() && !theModified)
			doLoad(filename);
		else
		{	GeddeiNite *g = new GeddeiNite;
			g->doLoad(filename);
			g->show();
		}
	}
}

void GeddeiNite::on_fileSaveAs_activated()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save Processor Network", "/tmp", "XML (*.xml);;All files (*)");
	if (!filename.isEmpty())
	{	theFilename = filename;
		on_fileSave_activated();
	}
	else
		statusBar()->message("Save aborted.");
}

void GeddeiNite::on_editRemove_activated()
{
	if (!theRunning && theScene.selectedItems().size() && qgraphicsitem_cast<ProcessorItem*>(theScene.selectedItems()[0]))
	{
		delete qgraphicsitem_cast<ProcessorItem*>(theScene.selectedItems()[0]);
		setModified(true);
		theScene.update();
	}
}

void GeddeiNite::slotPropertyChanged(QTableWidgetItem* _i)
{
	if (theUpdatingProperties || theRunning || !theScene.selectedItems().size())
		return;
	if (qgraphicsitem_cast<ProcessorItem*>(theScene.selectedItems()[0]) && theProperties->verticalHeaderItem(_i->row()))
	{
		ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(theScene.selectedItems()[0]);
		pi->setProperty(theProperties->verticalHeaderItem(_i->row())->text(), _i->text());
	}
	setModified(true);
}

bool GeddeiNite::connectAll()
{
	foreach (QGraphicsItem* i, theScene.items())
		if (ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i))
			if (!pi->connectYourself(theGroup))
			{
				statusBar()->message("Problem creating connections.");
				disconnectAll();
				return false;
			}

	if (!theGroup.confirmTypes())
	{
		statusBar()->message("Problem confirming types.");
		disconnectAll();
		return false;
	}
	theConnected = true;
	return true;
}

void GeddeiNite::disconnectAll()
{
	foreach (QGraphicsItem* i, theScene.items())
		if (ProcessorItem* pi = qgraphicsitem_cast<ProcessorItem*>(i))
			pi->disconnectYourself();
	theConnected = false;
}

void GeddeiNite::on_toolsDeployPlayer_activated()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open audio file", "/tmp", "Uncompressed audio (*.wav);;Ogg Vorbis (*.ogg);;FLAC audio (*.flac);;MP3 audio (*.mp3);;All files (*)");
	if (!filename.isEmpty())
	{
		ProcessorItem* pi = new ProcessorItem(ProcessorFactory::create("Player"), Properties("Filename", filename));
		theScene.addItem(pi);
		setModified(true);
	}
}

void GeddeiNite::on_modeRun_toggled(bool running)
{
	setFocus();

	if (theIgnoreNext)
	{
		theIgnoreNext = false;
		return;
	}

	if (running && !theRunning)
	{
		connectAll();
		bool successful = theGroup.go();
		if (successful)
		{
			theProperties->setEnabled(false);
			theRunning = true;
		}
		else
		{
			// some sort of cool depiction of the error.
			// restore to stable state.
			disconnectAll();
			modeStop->setChecked(true);
			statusBar()->message("Problem starting processors.");
		}
	}
	else if (!running && theRunning)
	{
		theRunning = false;
		theGroup.stop(false);
		theGroup.reset();
		disconnectAll();
		theProperties->setEnabled(true);
	}
	theScene.update();
}

void GeddeiNite::closeEvent(QCloseEvent *e)
{
	if (theModified)
	{}	// TODO: ask if want to save
	e->accept();
}

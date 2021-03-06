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

#include <cassert>
#include <iostream>
using namespace std;

#include "qscheduler.h"
#include "qtask.h"

#include "ConnectionItem.h"
#include "MultipleConnectionItem.h"
#include "MultipleInputItem.h"
#include "MultipleOutputItem.h"
#include "ProcessorItem.h"
#include "DomProcessorItem.h"
#include "MultiProcessorItem.h"
#include "MultiDomProcessorItem.h"
#include "ProcessorView.h"
#include "ProcessorsView.h"
#include "GeddeiNite.h"

GeddeiNite::GeddeiNite(bool _autoLoad):
	QMainWindow				(0),
	theScene				(this),
	theRunning				(false),
	theConnected			(false),
	theIgnoreNext			(false),
	theModified				(false)
{
	setAttribute(Qt::WA_DeleteOnClose);

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

	if (QFile::exists(s.value("mainwindow/lastproject").toString()) && _autoLoad)
		doLoad(s.value("mainwindow/lastproject").toString());
	statusBar()->showMessage(tr("Ready"), 2000);

	slotUpdateProperties();
}

GeddeiNite::~GeddeiNite()
{
	if (modeRun->isChecked())
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
	foreach (BaseItem* pi, filterRelaxed<BaseItem>(theScene.items()))
		pi->saveYourself(root, doc);
	foreach (MultipleConnectionItem* ci, filter<MultipleConnectionItem>(theScene.items()))
		ci->saveYourself(root, doc);
	foreach (ConnectionItem* ci, filter<ConnectionItem>(theScene.items()))
		ci->saveYourself(root, doc);

	QFile f(_filename);
	if (!f.open(QIODevice::WriteOnly))
	{	statusBar()->showMessage("Couldn't write to " + _filename, 2000);
		return;
	}
	QTextStream out(&f);
	out << doc.toString();
	statusBar()->showMessage("Saved.", 2000);
	setModified(false);
}

void GeddeiNite::doLoad(const QString &filename)
{
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly))
	{	statusBar()->showMessage("Couldn't read from " + filename, 2000);
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
		else if (elem.tagName() == "domprocessor")
			DomProcessorItem::fromDom(elem, &theScene);
		else if (elem.tagName() == "multiprocessor")
			MultiProcessorItem::fromDom(elem, &theScene);
		else if (elem.tagName() == "multidomprocessor")
			MultiDomProcessorItem::fromDom(elem, &theScene);
		else if (elem.tagName() == "connection")
			ConnectionItem::fromDom(elem, &theScene);
		else if (elem.tagName() == "multipleconnection")
		{
			MultipleConnectionItem::fromDom(elem, &theScene);
			connectAll();
			disconnectAll();
		}
	}
	setModified(true);
	statusBar()->showMessage("Loaded.", 2000);
	setModified(false);
}

void GeddeiNite::setModified(bool _modified)
{
	theModified = _modified;
	if (_modified && !theRunning)
	{
		if (connectAll())
		{
			modeRun->setEnabled(true);
			disconnectAll();
		}
		else
			modeRun->setEnabled(false);
	}
	setWindowTitle((theFilename.isEmpty() ? "Untitled" : theFilename) + (_modified ? " [ Modified ]" : "") + " - Geddei Nite");
}

void GeddeiNite::updateItems()
{
	theProcessors->clear();
	{	QStringList classes = SubProcessorFactory::available();
		for (QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	QListWidgetItem *item = new QListWidgetItem(QIcon(":/Icons/subprocessor.png"), *i, theProcessors, QListWidgetItem::UserType + 1);
			item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
	}
	{	QStringList classes = ProcessorFactory::available();
		for (QStringList::iterator i = classes.begin(); i != classes.end(); i++)
		{	QListWidgetItem *item = new QListWidgetItem(QIcon(":/Icons/processor.png"), *i, theProcessors, QListWidgetItem::UserType);
			item->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
	}
}

void GeddeiNite::on_theName_textChanged(QString const& _nn)
{
	if (theUpdatingProperties)
		return;
	QGraphicsItem* i = theScene.selectedItems().size() ? theScene.selectedItems()[0] : 0;
	if (BaseItem* bi = dynamic_cast<BaseItem*>(i))
		bi->setName(_nn);
}

void GeddeiNite::slotUpdateProperties()
{
	theUpdatingProperties = true;
	QGraphicsItem* i = theScene.selectedItems().size() ? theScene.selectedItems()[0] : 0;
	if (BaseItem* bi = dynamic_cast<BaseItem*>(i))
	{
		theName->setText(bi->name());
		theName->setEnabled(!theRunning);
		theType->setText(bi->typeName());
		Properties const& p(bi->properties());
		theProperties->setRowCount(p.size());
		for (uint i = 0; i < p.size(); i++)
		{
			theProperties->setVerticalHeaderItem(i, new QTableWidgetItem(p.keys()[i]));
			theProperties->setItem(i, 0, new QTableWidgetItem(p[p.keys()[i]].toString()));
		}
		theProperties->resizeColumnsToContents();
		theProperties->setEnabled(true);
	}
	else
	{
		theName->setText("");
		theName->setEnabled(false);
		theType->setText("");
		theProperties->clear();
		theProperties->setRowCount(1);
		theProperties->setEnabled(false);
	}
	theUpdatingProperties = false;
	theProperties->update();
	theProperties->updateGeometry();
	QString html = "Nothing selected.";
	if (i && dynamic_cast<ConnectionItem*>(i))
		html = dynamic_cast<ConnectionItem*>(i)->to()->typeInfo();
	else if (i && dynamic_cast<MultipleConnectionItem*>(i))
		html = dynamic_cast<MultipleConnectionItem*>(i)->to()->typeInfo();
	theLinkInfo->setHtml("<div style=\"font-size: 8pt\">" + html + "</div>");
}

void GeddeiNite::on_fileSave_activated()
{
	if (theFilename.isEmpty())
	{	on_fileSaveAs_activated();
		return;
	}

	statusBar()->showMessage("Saving...");
	doSave(theFilename);
}

void GeddeiNite::on_fileOpen_activated()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open Processor Network", "/tmp", "XML (*.xml);;All files (*)");
	if (!filename.isEmpty())
	{	if (theFilename.isEmpty() && !theModified)
			doLoad(filename);
		else
		{	GeddeiNite *g = new GeddeiNite(false);
			g->doLoad(filename);
			g->show();
		}
	}
}

void GeddeiNite::on_fileNew_activated()
{
	GeddeiNite *g = new GeddeiNite(false);
	g->show();
}

void GeddeiNite::on_fileSaveAs_activated()
{
	QString filename = QFileDialog::getSaveFileName(this, "Save Processor Network", "/tmp", "XML (*.xml);;All files (*)");
	if (!filename.isEmpty())
	{	theFilename = filename;
		on_fileSave_activated();
	}
	else
		statusBar()->showMessage("Save aborted.");
}

void GeddeiNite::on_editRemove_activated()
{
	if (theRunning || theScene.selectedItems().size() != 1)
		return;

	if (BaseItem* bi = dynamic_cast<BaseItem*>(theScene.selectedItems()[0]))
	{
		foreach (ConnectionItem* i, filter<ConnectionItem>(theScene.items()))
			if (i->fromProcessor() == bi || i->toProcessor() == bi)
				delete i;
		foreach (MultipleConnectionItem* i, filter<MultipleConnectionItem>(theScene.items()))
			if (i->from()->baseItem() == bi || i->to()->baseItem() == bi)
				delete i;
		delete bi;
		setModified(true);
		theScene.update();
	}
	else if (ConnectionItem* ci = dynamic_cast<ConnectionItem*>(theScene.selectedItems()[0]))
	{
		delete ci;
		setModified(true);
		theScene.update();
	}
	else if (MultipleConnectionItem* mci = dynamic_cast<MultipleConnectionItem*>(theScene.selectedItems()[0]))
	{
		delete mci;
		setModified(true);
		theScene.update();
	}
}

void GeddeiNite::slotPropertyChanged(QTableWidgetItem* _i)
{
	if (theUpdatingProperties || !theScene.selectedItems().size())
		return;
	if (dynamic_cast<BaseItem*>(theScene.selectedItems()[0]) && theProperties->verticalHeaderItem(_i->row()))
	{
		BaseItem* bi = dynamic_cast<BaseItem*>(theScene.selectedItems()[0]);
		bi->setProperty(theProperties->verticalHeaderItem(_i->row())->text(), _i->text());
	}
	if (!theRunning)
		setModified(true);
}

bool GeddeiNite::connectAll()
{
	QString failed;
	foreach (ConnectionItem* i, filterRelaxed<ConnectionItem>(theScene.items()))
		i->resetNature();
	foreach (ConnectionItem* i, filterRelaxed<ConnectionItem>(theScene.items()))
		i->refreshNature();

	foreach (BaseItem* i, filterRelaxed<BaseItem>(theScene.items()))
		i->prepYourself(theGroup);
	foreach (BaseItem* i, filterRelaxed<BaseItem>(theScene.items()))
		i->preConnectYourself();
	foreach (BaseItem* i, filterRelaxed<BaseItem>(theScene.items()))
		if (!i->connectYourself())
			failed += i->name() + ", ";
	if (!failed.isEmpty())
	{
		failed.chop(2);
		statusBar()->showMessage("Problem creating connections with " + failed, 10000);
		disconnectAll();
		return false;
	}

	if (!theGroup.confirmTypes())
	{
		Groupable* p = theGroup.errorProc();
		assert(p);
		Groupable::ErrorType et = p->errorType();
		int ed = p->errorData();
		statusBar()->showMessage("Problem confirming types (" + p->name() + ": " + QString("%1[%2]").arg(et).arg(ed) + ")");
		disconnectAll();
		return false;
	}

	foreach (BaseItem* bi, filterRelaxed<BaseItem>(theScene.items()))
		bi->typesConfirmed();

	theConnected = true;
	return true;
}

void GeddeiNite::disconnectAll()
{
	foreach (BaseItem* i, filterRelaxed<BaseItem>(theScene.items()))
		i->disconnectYourself();
	theConnected = false;
	foreach (BaseItem* i, filterRelaxed<BaseItem>(theScene.items()))
		i->unprepYourself();
}

void GeddeiNite::on_toolsDeployPlayer_activated()
{
	QString filename = QFileDialog::getOpenFileName(this, "Open audio file", "/tmp", "Uncompressed audio (*.wav);;Ogg Vorbis (*.ogg);;FLAC audio (*.flac);;MP3 audio (*.mp3);;All files (*)");
	if (!filename.isEmpty())
	{
		ProcessorBasedItem* pi = new ProcessorItem("Player", Properties("Filename", filename));
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
		if (connectAll() && theGroup.go(true))
		{
			theProcessors->setEnabled(false);
			theScene.onStarted();
			m_ourTimer = startTimer(500);
			m_startTime = rdtsc();
			theScheduled->startUpdating();
			theRunning = true;

			QList<QTask*> ts = QScheduler::get()->tasks();
			float c = 1.f / ts.count();
			float i = -c;
			foreach (QTask* t, ts)
				m_colourMap[t] = QColor::fromHsvF(i += c, 1, 1);
		}
		else
		{
			theGroup.stop(true);
			// some sort of cool depiction of the error.
			// restore to stable state.
			disconnectAll();
			modeStop->setChecked(true);
			statusBar()->showMessage("Problem starting processors.");
		}
	}
	else if (!running && theRunning)
	{
		m_colourMap.clear();
		theScheduled->stopUpdating();
		theScene.onStopped();
		theRunning = false;
		theGroup.stop(false);
		theGroup.reset();
		disconnectAll();
		theProcessors->setEnabled(true);
		theName->setEnabled(!theName->text().isEmpty());
		killTimer(m_ourTimer);
	}
	theScene.update();
}

void GeddeiNite::timerEvent(QTimerEvent*)
{
	double elapsed = rdtscElapsed(m_startTime);
	theTasks->clear();
	double total = 0;
	QList<QTask*> ts = QScheduler::get()->tasks();

	foreach (QTask* t, ts)
		total += t->timeInTask();

	foreach (QTask* t, ts)
	{
		QColor col = QColor::fromHsvF(myColour(t).hueF(), 0.4, 1);
		CoProcessor* cop = dynamic_cast<CoProcessor*>(t);
		QTreeWidgetItem* it = new QTreeWidgetItem(QStringList()
												  << (cop ? cop->type() : "n/a")
												  << t->taskName()
												  << QString::number(t->timeInTask() / total * 100, 'g', 3)
												  << QString::number(t->timeInTask() / elapsed * 100, 'g', 3));
		it->setBackgroundColor(0, col);
		theTasks->addTopLevelItem(it);
		if (cop && cop->type() == "DomProcessor")
		{
			QList<QPair<SubProcessor*, double> > ss;
			ss << qMakePair<SubProcessor*, double>(static_cast<DomProcessor*>(cop)->primary(), t->timeInTask());
			double remainder = t->timeInTask();
			while (ss.count())
			{
				QPair<SubProcessor*, double> s = ss.takeLast();
				if (Combination* c = dynamic_cast<Combination*>(s.first))
					ss << qMakePair(c->y(), c->totalTimeY()) << qMakePair(c->x(), c->totalTimeX());
				else
				{
					it->addChild(new QTreeWidgetItem(QStringList()
						<< s.first->type()
						<< ""
						<< QString::number(s.second / total * 100, 'g', 3)
						<< QString::number(s.second / elapsed * 100, 'g', 3)
						));
					remainder -= s.second;
				}
			}
			it->setText(2, QString::number(remainder / total * 100, 'g', 3));
			it->setText(3, QString::number(remainder / elapsed * 100, 'g', 3));
		}
	}
	theTasks->expandAll();
}

void GeddeiNite::closeEvent(QCloseEvent *e)
{
	if (theModified)
	{}	// TODO: ask if want to save
	e->accept();
}

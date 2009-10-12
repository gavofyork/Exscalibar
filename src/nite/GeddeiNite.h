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

#include <QtGui>
#include <Geddei>
using namespace Geddei;

#include "ProcessorsScene.h"
#include "ui_GeddeiNiteBase.h"

/** @internal
 * @author Gav Wood <gav@kde.org>
 */
class GeddeiNite: public QMainWindow, public Ui::GeddeiNiteBase
{
	Q_OBJECT

public:
	GeddeiNite(bool _autoLoad = true);
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
	void			slotUpdateProperties();
	void			slotChanged() { setModified(true); }

	void			on_theName_textChanged(QString const& _nn);

	void			on_modeRun_toggled(bool testing);
	void			on_fileNew_activated();
	void			on_fileOpen_activated();
	void			on_fileSave_activated();
	void			on_fileSaveAs_activated();
	void			on_fileExit_activated() { qApp->exit(); }
	void			on_editRemove_activated();
	void			on_toolsDeployPlayer_activated();
	void			on_viewDynamicDisplay_toggled(bool _dd) { theScene.setDynamicDisplay(_dd); }

private:
	void			doSave(const QString &filename);
	void			doLoad(const QString &filename);
	bool			connectAll();
	void			disconnectAll();
	void			updateItems();

	ProcessorsScene	theScene;
	ProcessorGroup	theGroup;

	QString			theFilename;

	bool			theUpdatingProperties;
	bool			theRunning;
	bool			theTested;
	bool			theConnected;
	bool			theIgnoreNext;
	bool			theModified;
};

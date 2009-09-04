/****************************************************************************
** Form interface generated from reading ui file 'geddeinitebase.ui'
**
** Created: Tue Dec 12 11:08:08 2006
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.6   edited Aug 31 2005 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef GEDDEINITEBASE_H
#define GEDDEINITEBASE_H

#include <qvariant.h>
#include <qpixmap.h>
#include <q3mainwindow.h>
//Added by qt3to4:
#include <Q3ActionGroup>
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <Q3PopupMenu>

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QAction;
class Q3ActionGroup;
class Q3ToolBar;
class Q3PopupMenu;
class BobsView;

class GeddeiNiteBase : public Q3MainWindow
{
    Q_OBJECT

public:
    GeddeiNiteBase( QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::WType_TopLevel );
    ~GeddeiNiteBase();

    BobsView* theView;
    QMenuBar *MenuBar;
    Q3PopupMenu *fileMenu;
    Q3PopupMenu *editMenu;
    Q3PopupMenu *ModeMenu;
    Q3PopupMenu *unnamed;
    Q3PopupMenu *View;
    Q3ToolBar *toolBar;
    Q3ToolBar *Toolbar;
    QAction* fileNewAction;
    QAction* fileOpenAction;
    QAction* fileSaveAction;
    QAction* fileSaveAsAction;
    QAction* filePrintAction;
    QAction* fileExitAction;
    QAction* editUndoAction;
    QAction* editRedoAction;
    QAction* editCutAction;
    QAction* editCopyAction;
    QAction* editPasteAction;
    QAction* editFindAction;
    Q3ActionGroup* modeActionGroup;
    QAction* modeStopAction;
    QAction* modeDistributeAction;
    QAction* modeRunAction;
    QAction* modeMakeLinkAction;
    QAction* viewSnapToGridAction;
    QAction* editRemoveAction;
    QAction* toolsDeployFilePlayerAction;
    QAction* modeTestAction;
    QAction* modeTestRunAction;
    QAction* editPreferencesAction;

public slots:
    virtual void fileNew();
    virtual void fileOpen();
    virtual void fileSave();
    virtual void fileSaveAs();
    virtual void filePrint();
    virtual void fileExit();
    virtual void editUndo();
    virtual void editRedo();
    virtual void editCut();
    virtual void editCopy();
    virtual void editPaste();
    virtual void editFind();
    virtual void editRemove();
    virtual void slotRunToggled(bool);
    virtual void slotTest();
    virtual void slotLink();
    virtual void slotDeployPlayer();

protected:
    Q3GridLayout* GeddeiNiteBaseLayout;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;
    QPixmap image6;
    QPixmap image7;
    QPixmap image8;
    QPixmap image9;
    QPixmap image10;
    QPixmap image11;
    QPixmap image12;
    QPixmap image13;
    QPixmap image14;
    QPixmap image15;
    QPixmap image16;
    QPixmap image17;
    QPixmap image18;
    QPixmap image19;
    QPixmap image20;
    QPixmap image21;

};

#endif // GEDDEINITEBASE_H

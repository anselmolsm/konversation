/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  prefspagescripts.h  -  Provides a user interface to customize login scripts
  begin:     Tue Sep 24 2002
  copyright: (C) 2002 by Matthias Gierlings
  email:     gismore@users.sourceforge.net

  $Id$
*/


#ifndef PREFSPAGESCRIPTS_H
#define PREFSPAGESCRIPTS_H

#include <klistview.h>
#include <keditcl.h>

#include <qlayout.h>
#include <qdir.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include "prefspage.h"
#include "preferences.h"

/*
  @author Matthias Gierlings
*/

class PrefsPageScripts : public PrefsPage
{
  Q_OBJECT

  public:
	  PrefsPageScripts(QFrame *passedParent, Preferences *passedPreferences);
	  ~PrefsPageScripts();

	protected:
    QBoxLayout      *mainBox;
		KListView       *scriptsList;
		KListViewItem   *scriptsListEntry;
		KEdit           *scriptEditor;
    QDir						*scriptsDirectory;
		QFile						openedScriptFile;
		QString					scriptFilePath, oldScriptFileName;
		QStringList			scriptFiles;
		QTextStream			*script;
		Preferences			*myPreferences;
		char						streamBuffer[512];

	signals:
		void fileRenamed(QListViewItem *passedItem);

	public slots:
		void openScriptFile(QListViewItem *passedItem);
		void renameScriptFile(QListViewItem *passedItem);
};

#endif

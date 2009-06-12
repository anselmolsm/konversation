/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2006 Dario Abatianni <eisfuchs@tigress.com>
*/

#include "watchednicknames_config.h"
#include "preferences.h"
#include "application.h"
#include "mainwindow.h"


WatchedNicknames_Config::WatchedNicknames_Config(QWidget *parent, const char *name)
 : QWidget(parent)
{
  setObjectName(QString::fromLatin1(name));
  setupUi(this);

  // reset flag to defined state (used to block signals when just selecting a new item)
  newItemSelected=false;

  reloadSettings();

  connect(kcfg_UseNotify,SIGNAL (toggled(bool)),this,SLOT (checkIfEmptyListview(bool)) );
  connect(newButton,SIGNAL (clicked()),this,SLOT (newNotify()) );
  connect(removeButton,SIGNAL (clicked()),this,SLOT (removeNotify()) );
  connect(notifyListView,SIGNAL (currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),this,SLOT (entrySelected(QTreeWidgetItem*)) );

  connect(networkDropdown,SIGNAL (activated(int)),this,SLOT (networkChanged(int)) );
  connect(nicknameInput,SIGNAL (textChanged(const QString&)),this,SLOT (nicknameChanged(const QString&)) );

  connect(KonversationApplication::instance(), SIGNAL(serverGroupsChanged(const Konversation::ServerGroupSettingsPtr)),
      this, SLOT(reloadSettings()));
}

WatchedNicknames_Config::~WatchedNicknames_Config()
{
}

void WatchedNicknames_Config::restorePageToDefaults()
{
}

// fill in the notify listview with groups and nicknames
void WatchedNicknames_Config::loadSettings()
{
    // cleanup, so we won't add duplicate items
    notifyListView->clear();
    networkDropdown->clear();
    // make sure all widgets are disabled
    notifyListView->clearSelection();
    enableEditWidgets(false);
    updateNotifyListView();

    // remember current list for hasChanged()
    m_oldNotifyList=currentNotifyList();
}

void WatchedNicknames_Config::reloadSettings()
{
  // cleanup, so we won't add duplicate items
  notifyListView->clear();
  networkDropdown->clear();
  // make sure all widgets are disabled
  notifyListView->clearSelection();
  enableEditWidgets(false);
  updateNotifyListView();

  // remember current list for hasChanged()
  m_oldNotifyList=currentNotifyList();
}

// adds a new network branch to the listview
void WatchedNicknames_Config::addNetworkBranch(Konversation::ServerGroupSettingsPtr serverGroup)
{
  // get the current notify list and an iterator
  QMap<int, QStringList> notifyList = Preferences::notifyList();

  if (notifyList.isEmpty())
  {
     return;
  }

  // get the group iterator to find all servers in the group
  QMap<int, QStringList>::const_iterator groupIt=notifyList.constFind(serverGroup->id());

  if(groupIt == notifyList.constEnd())
    return;

  QTreeWidgetItem* groupItem = new QTreeWidgetItem(notifyListView, QStringList() << serverGroup->name());
  groupItem->setData(0, Qt::UserRole, serverGroup->id());
  // get list of nicks for the current group
  QStringList nicks=groupIt.value();
  // add group to dropdown list
  networkDropdown->addItem(serverGroup->name(), serverGroup->id());
  // add nicknames to group branch (reverse order again)
  for(int i=0; i < nicks.count(); i++)
  {
    new QTreeWidgetItem(groupItem, QStringList() << nicks.at(i));
  } // for
  // unfold group branch
  groupItem->setExpanded(true);
}

// save list of notifies permanently, taken from the listview
void WatchedNicknames_Config::saveSettings()
{
  // create new in-memory notify structure
  QMap<int,QStringList> notifyList;

  // get first notify group
  QTreeWidget* listView=notifyListView;
  QTreeWidgetItem* group=listView->topLevelItem(0);

  // loop as long as there are more groups in the listview
  while(group)
  {
    int groupId=group->data(0,Qt::UserRole).toInt();

    // later contains all nicks separated by blanks
    QString nicks;
    // get first nick in the group
    QTreeWidgetItem* nick=group->child(0);
    // loop as long as there are still nicks in this group
    while(nick)
    {
      // add nick to string container and add a blank
      nicks+=nick->text(0)+' ';
      // get next nick in the group
      nick=listView->itemBelow(nick);
      if (nick && !nick->parent()) nick = 0; // do not take toplevel items
    } // while

    // write nick list to in-memory notify qstringlist
    notifyList.insert(groupId, nicks.trimmed().split(' ', QString::SkipEmptyParts));
    // get next group
    group=listView->itemBelow(group);
  } // while

  // update in-memory notify list
  Preferences::setNotifyList(notifyList);
  static_cast<KonversationApplication*>(kapp)->saveOptions(false);

  // remember current list for hasChanged()
  m_oldNotifyList=currentNotifyList();
}

// returns the currently edited notify list
QMap<int, QString> WatchedNicknames_Config::currentNotifyList()
{
  // prepare list
  QMap<int, QString> newList;

  QTreeWidget* listView=notifyListView;
  QTreeWidgetItemIterator it(listView);
  while(*it)
  {
    newList.insert((*it)->data(0,Qt::UserRole).toInt(),(*it)->text(0));
    ++it;
  }

  return newList;
}

bool WatchedNicknames_Config::hasChanged()
{
  // return true if something has changed
  return(m_oldNotifyList!=currentNotifyList());
}

// slots

void WatchedNicknames_Config::updateNotifyListView()
{
    // get list of server networks
    Konversation::ServerGroupHash serverGroupHash = Preferences::serverGroupHash();
    QHashIterator<int, Konversation::ServerGroupSettingsPtr> it(serverGroupHash);
    // iterate through all networks in the server group list
    while(it.hasNext())
    {
        addNetworkBranch(it.next().value());
    }
}

// helper function to disable "New" button on empty listview
void WatchedNicknames_Config::checkIfEmptyListview(bool state)
{
  // only enable "New" button if there is at least one group in the list
  if(!notifyListView->topLevelItemCount()) state=false;
  newButton->setEnabled(state);
}

// add new notify entry
void WatchedNicknames_Config::newNotify()
{
  // get listview object and possible first selected item
  QTreeWidget* listView=notifyListView;
  QTreeWidgetItem* item=listView->currentItem();

  // if there was an item selected, try to find the group it belongs to,
  // so the newly created item will go into the same group, otherwise
  // just create the new entry inside of the first group
  if(item)
  {
    if(item->parent()) item=item->parent();
  }
  else
    item=listView->topLevelItem(0);

  // finally insert new item
  item=new QTreeWidgetItem(item, QStringList() << i18n("New"));
  // make this item the current and selected item
  item->setSelected(true);
  listView->setCurrentItem(item);
  // update network and nickname inputs
  entrySelected(item);
  // unfold group branch in case it was empty before
  listView->scrollToItem(item);
  nicknameInput->setFocus();
  nicknameInput->selectAll();
  // tell the config system that something has changed
  emit modified();
}

// remove a notify entry from the listview
void WatchedNicknames_Config::removeNotify()
{
  // get listview pointer and the selected item
  QTreeWidget* listView=notifyListView;
  QTreeWidgetItem* item=listView->currentItem();

  // sanity check
  if(item)
  {
    // check which item to highlight after we deleted the chosen one
    QTreeWidgetItem* itemAfter=listView->itemBelow(item);
    if(!itemAfter) itemAfter=listView->itemAbove(item);
    delete(item);

    // if there was an item, highlight it
    if(itemAfter)
    {
      itemAfter->setSelected(true);
      listView->setCurrentItem(itemAfter);
      // update input widgets
      entrySelected(itemAfter);
    }
    // tell the config system that something has changed
    emit modified();
  }
}

// what to do when the user selects an entry
void WatchedNicknames_Config::entrySelected(QTreeWidgetItem* notifyEntry)
{
  // play it safe, assume disabling all widgets first
  bool enabled=false;

  // if there actually was an entry selected ...
  if(notifyEntry)
  {
    // is this entry a nickname?
    QTreeWidgetItem* group=notifyEntry->parent();
    if(group)
    {
      // all edit widgets may be enabled
      enabled=true;
      // tell all now emitted signals that we just clicked on a new item, so they should
      // not emit the modified() signal.
      newItemSelected=true;
      // copy network name and nickname to edit widgets
      nicknameInput->setText(notifyEntry->text(0));
      networkDropdown->setCurrentIndex(networkDropdown->findData(group->data(0,Qt::UserRole).toInt()));
      // all signals will now emit the modified() signal again
      newItemSelected=false;
    }
  }
  enableEditWidgets(enabled);
}

// enable/disable edit widgets
void WatchedNicknames_Config::enableEditWidgets(bool enabled)
{
  removeButton->setEnabled(enabled);
  networkLabel->setEnabled(enabled);
  networkDropdown->setEnabled(enabled);
  nicknameLabel->setEnabled(enabled);
  nicknameInput->setEnabled(enabled);
}

// user changed the network this nickname is on
void WatchedNicknames_Config::networkChanged(int index)
{
  // get listview pointer and selected entry
  QTreeWidget* listView=notifyListView;
  QTreeWidgetItem* item=listView->currentItem();
  //actually an int but we keep it as a QVariant for comparisons
  QVariant newGroupId = networkDropdown->itemData(index);
  // sanity check
  if(item)
  {
    // get group the nickname is presently associated to
    QTreeWidgetItem* group=item->parent();
    // did the user actually change anything?
    if(group && group->data(0,Qt::UserRole)!=newGroupId)
    {
      // find the branch the new network is in
      QTreeWidgetItem* lookGroup=listView->topLevelItem(0);
      while(lookGroup && (lookGroup->data(0,Qt::UserRole)!=newGroupId)) lookGroup=listView->itemBelow(lookGroup);
      // if it was found (should never fail)
      if(lookGroup)
      {
        // deselect nickname, unlink it and relink it to the new network
        item->setSelected(false);
        group->removeChild(item);
        lookGroup->insertChild(0, item);
        // make the moved nickname current and selected item
        item->setSelected(true);
        listView->setCurrentItem(item);
        // unfold group branch in case it was empty before
        lookGroup->setExpanded(true);
        // tell the config system that something has changed
        if(!newItemSelected) emit modified();
      }
    }
  }
}

// the user edited the nickname
void WatchedNicknames_Config::nicknameChanged(const QString& newNickname)
{
  // get listview pointer and selected item
  QTreeWidget* listView=notifyListView;
  QTreeWidgetItem* item=listView->currentItem();

  // sanity check
  if(item)
  {
    // rename item
    item->setText(0,newNickname);
    // tell the config system that something has changed
    if(!newItemSelected) emit modified();
  }
}

#include "watchednicknames_config.moc"

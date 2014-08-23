/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Shows the list of channels

  Copyright (C) 2003 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2009 Travis McHenry <wordsizzle@gmail.com>
*/

#include "channellistpanel.h"
#include "channel.h"
#include "preferences.h"
#include "server.h"
#include "common.h"
#include "application.h"

#include <QHeaderView>
#include <QTextDocument>

#include <KRun>
#include <QFileDialog>
#include <QMenu>
#include <KToolBar>


ChannelListModel::ChannelListModel(QObject* parent) : QAbstractListModel(parent)
{
}

void ChannelListModel::append(const ChannelItem& item)
{
    m_channelList.append(item);
    reset();
}

int ChannelListModel::columnCount(const QModelIndex& /*parent*/) const
{
    return 3;
}

int ChannelListModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_channelList.count();
}

QVariant ChannelListModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid() || index.row() >= m_channelList.count ())
        return QVariant();

    const ChannelItem& item = m_channelList[index.row()];

    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
            case 0:
                return item.name;
            case 1:
                return item.users;
            case 2:
                return item.topic;
            default:
                return QVariant();
        }
    }
    else if(role == Qt::ToolTipRole)
    {
        return QString(QLatin1String("<qt>") + Qt::escape(item.topic) + QLatin1String("</qt>"));
    }
    return QVariant();
}

QVariant ChannelListModel::headerData (int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical || role != Qt::DisplayRole)
        return QVariant();

    switch(section)
    {
        case 0:
            return i18n("Channel Name");
        case 1:
            return i18n("Users");
        case 2:
            return i18n("Channel Topic");
        default:
            return QVariant();
    }
}

ChannelListProxyModel::ChannelListProxyModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    m_minUsers = 0;
    m_maxUsers = 0;
    m_filterChannel = true;
    m_filterTopic = false;
}

bool ChannelListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
    QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);

    return (((m_filterChannel && sourceModel()->data(index0).toString().contains(filterRegExp()))
        || (m_filterTopic && sourceModel()->data(index2).toString().contains(filterRegExp()))
        || (!m_filterChannel && !m_filterTopic))
        && usersInRange(sourceModel()->data(index1).toInt()));
}

bool ChannelListProxyModel::usersInRange(int users) const
{
    return (!m_minUsers || users >= m_minUsers)
    && (!m_maxUsers || users <= m_maxUsers);
}

void ChannelListProxyModel::setFilterMinimumUsers(int users)
{
    m_minUsers = users;
}

void ChannelListProxyModel::setFilterMaximumUsers(int users)
{
    m_maxUsers = users;
}

void ChannelListProxyModel::setFilterTopic(bool filter)
{
    m_filterTopic = filter;
}

void ChannelListProxyModel::setFilterChannel(bool filter)
{
    m_filterChannel = filter;
}

ChannelListPanel::ChannelListPanel(QWidget* parent) : ChatWindow(parent)
{
    setType(ChatWindow::ChannelList);
    setName(i18n("Channel List"));

    m_firstRun = true;
    m_regexState = false;
    m_numUsers = 0;
    m_numChannels = 0;
    m_visibleUsers = 0;
    m_visibleChannels = 0;
    m_progressTimer = new QTimer(this);
    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_tempTimer = new QTimer(this);
    m_tempTimer->setSingleShot(true);

    setSpacing(0);
    m_toolBar = new KToolBar(this, true, true);
    m_toolBar->setObjectName(QStringLiteral("channellistpanel_toolbar"));
    m_saveList = m_toolBar->addAction(QIcon::fromTheme(QStringLiteral("document-save")), i18nc("save list", "Save &List..."), this, SLOT(saveList()));
    m_saveList->setWhatsThis(i18n("Click here to save the channel list."));
    m_refreshList = m_toolBar->addAction(QIcon::fromTheme(QStringLiteral("view-refresh")), i18nc("refresh list", "&Refresh List"), this, SLOT(refreshList()));
    m_refreshList->setWhatsThis(i18n("Click here to refresh the channel list."));
    m_toolBar->addSeparator();
    m_joinChannel = m_toolBar->addAction(QIcon::fromTheme(QStringLiteral("irc-join-channel")), i18nc("join channel", "&Join Channel"), this, SLOT(joinChannelClicked()));
    m_joinChannel->setWhatsThis(i18n("Click here to join the channel. A new tab is created for the channel."));
    //UI Setup
    setupUi(this);

    m_channelListModel = new ChannelListModel(this);

    m_proxyModel = new ChannelListProxyModel(this);
    m_proxyModel->setSourceModel(m_channelListModel);
    m_channelListView->setModel(m_proxyModel);
    m_channelListView->header()->resizeSection(1,75); // resize users section to be smaller

    Preferences::restoreColumnState(m_channelListView, QStringLiteral("ChannelList ViewSettings"));

    // double click on channel entry joins the channel
    connect(m_channelListView, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(joinChannelClicked()) );
    connect(m_channelListView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(currentChanged(QModelIndex,QModelIndex)));
    connect(m_channelListView, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenu(QPoint)) );

    connect(m_regexBox, SIGNAL(stateChanged(int)), this, SLOT(filterChanged()));
    connect(m_topicBox, SIGNAL(stateChanged(int)), this, SLOT(filterChanged()));
    connect(m_channelBox, SIGNAL(stateChanged(int)), this, SLOT(filterChanged()));
    connect(m_minUser, SIGNAL(valueChanged(int)), this, SLOT(filterChanged()));
    connect(m_maxUser, SIGNAL(valueChanged(int)), this, SLOT(filterChanged()));

    connect(m_filterLine, SIGNAL(returnPressed()), this, SLOT(applyFilterClicked()) );
    connect(m_filterLine, SIGNAL(textChanged(QString)), this, SLOT(filterChanged()));

    connect(m_filterTimer, SIGNAL(timeout()), this, SLOT(updateFilter()));
    connect(m_progressTimer, SIGNAL(timeout()), this, SLOT(setProgress()));
    connect(m_tempTimer, SIGNAL(timeout()), this, SLOT(endOfChannelList()));

    updateUsersChannels();
}

ChannelListPanel::~ChannelListPanel()
{
    Preferences::saveColumnState(m_channelListView, QStringLiteral("ChannelList ViewSettings"));
}

void ChannelListPanel::refreshList()
{
    if (!m_refreshList->isEnabled())
        return;

    m_numUsers = 0;
    m_numChannels = 0;
    m_visibleUsers = 0;
    m_visibleChannels = 0;

    //hide temporarily to prevent multiple refreshes, but renable if it doesn't start in
    //3 seconds in case we got a 'server busy' error. It doesn't matter if it's slower than
    //that because addToChannelList's first run handles this anyway
    m_refreshList->setEnabled(false);
    m_tempTimer->start(3000);

    emit refreshChannelList();
}

void ChannelListPanel::addToChannelList(const QString& channel,int users,const QString& topic)
{
    if (m_firstRun)
    {
        if(m_tempTimer->isActive())
            m_tempTimer->stop();

        m_refreshList->setEnabled(false);

        m_statsLabel->setText(i18n("Refreshing."));
        m_progressTimer->start(500);

        m_firstRun = false;
        m_channelListModel = new ChannelListModel(this);
    }

    ChannelItem item;
    item.name = channel;
    item.users = users;
    item.topic = Konversation::removeIrcMarkup(topic);
    m_channelListModel->append(item);

    ++m_numChannels;
    m_numUsers += users;
}

void ChannelListPanel::endOfChannelList()
{
    m_progressTimer->stop();

    m_proxyModel->setSourceModel(m_channelListModel);
    m_proxyModel->invalidate();
    m_refreshList->setEnabled(true);
    m_firstRun = true;
    updateUsersChannels();
}

void ChannelListPanel::filterChanged()
{
    m_filterTimer->start(300);
}

void ChannelListPanel::updateFilter()
{
    QString text = m_filterLine->text();
    int max = m_maxUser->value();
    int min = m_minUser->value();
    bool topic = m_topicBox->isChecked();
    bool channel = m_channelBox->isChecked();
    bool regex = m_regexBox->isChecked();
    bool regexChanged = (regex != m_regexState);
    if (regexChanged) m_regexState = regex;

    bool change = false;

    if (m_proxyModel->filterRegExp().pattern() != text || regexChanged)
    {
        change = true;
        if(m_regexState)
            m_proxyModel->setFilterRegExp(text);
        else
            m_proxyModel->setFilterWildcard(text);
    }

    if (m_proxyModel->filterMinimumUsers() != min)
    {
        change = true;
        m_proxyModel->setFilterMinimumUsers(min);
    }

    if (m_proxyModel->filterMaximumUsers() != max)
    {
        change = true;
        m_proxyModel->setFilterMaximumUsers(max);
    }

    if (m_proxyModel->filterTopic() != topic)
    {
        change = true;
        m_proxyModel->setFilterTopic(topic);
    }

    if (m_proxyModel->filterChannel() != channel)
    {
        change = true;
        m_proxyModel->setFilterChannel(channel);
    }

    if (change)
        m_proxyModel->invalidate();

    updateUsersChannels();
}

void ChannelListPanel::currentChanged(QModelIndex current,QModelIndex previous)
{
    Q_UNUSED(previous);
    m_joinChannel->setEnabled(m_online && current.isValid());
}

void ChannelListPanel::setProgress()
{
    QString text = m_statsLabel->text();
    if(text.length() < 13)
        m_statsLabel->setText(text + QLatin1Char('.'));
    else
        m_statsLabel->setText(i18n("Refreshing."));
}


void ChannelListPanel::countUsers(const QModelIndex& index, int pos)
{
    m_visibleUsers += index.data().toInt();
    ++pos;
    if (pos < m_proxyModel->rowCount())
        countUsers(index.sibling(pos,1), pos);
}

void ChannelListPanel::updateUsersChannels()
{
    m_visibleUsers = 0;
    countUsers(m_proxyModel->index(0,1,QModelIndex()),0);
    m_visibleChannels = m_proxyModel->rowCount();
    m_statsLabel->setText(i18n("Channels: %1 (%2 shown)", m_numChannels, m_visibleChannels) +
                          i18n(" Non-unique users: %1 (%2 shown)", m_numUsers, m_visibleUsers));
}

void ChannelListPanel::saveList()
{
    // Ask user for file name
    QString fileName = QFileDialog::getSaveFileName(
        this,
        i18n("Save Channel List"));

    if (!fileName.isEmpty())
    {
        // first find the longest channel name and nick number for clean table layouting
        int maxChannelWidth=0;
        int maxUsersWidth=0;

        int rows = m_proxyModel->rowCount();
        QModelIndex index = m_proxyModel->index(0,0,QModelIndex());
        for (int r = 0; r < rows; r++)
        {
            QString channel = index.sibling(r,0).data().toString();
            QString users = index.sibling(r,1).data().toString();

            if (channel.length()>maxChannelWidth)
            {
                maxChannelWidth = channel.length();
            }

            if (users.length()>maxUsersWidth)
            {
                maxUsersWidth = users.length();
            }
        }

        // now save the list to disk
        QFile listFile(fileName);
        listFile.open(QIODevice::WriteOnly);
        // wrap the file into a stream
        QTextStream stream(&listFile);

        QString header(i18n("Konversation Channel List: %1 - %2\n\n",
            m_server->getServerName(),
            QDateTime::currentDateTime().toString()));

        // send header to stream
        stream << header;

        for (int r = 0; r < rows; r++)
        {
            QString channel = index.sibling(r,0).data().toString();
            QString users = index.sibling(r,1).data().toString();
            QString topic = index.sibling(r,2).data().toString();

            QString channelName;
            channelName.fill(QLatin1Char(' '), maxChannelWidth);
            channelName.replace(0, channel.length(), channel);

            QString usersPad;
            usersPad.fill(QLatin1Char(' '),maxUsersWidth);
            QString usersNum(usersPad+users);
            usersNum = usersNum.right(maxUsersWidth);

            QString line(channelName+QLatin1Char(' ')+usersNum+QLatin1Char(' ')+topic+QLatin1Char('\n'));
            stream << line;
        }

        listFile.close();
    }
}

void ChannelListPanel::joinChannelClicked()
{
    QModelIndex item = m_channelListView->currentIndex();
    if(item.isValid())
    {
        if(item.column() != 0)
            item = item.sibling(item.row(),0);
        emit joinChannel(item.data().toString());
    }
}

void ChannelListPanel::applyFilterClicked()
{
    if (!m_numChannels)
    {
        refreshList();

        return;
    }
}

void ChannelListPanel::contextMenu(const QPoint& p)
{
    QModelIndex item = m_channelListView->indexAt(p);
    if (!item.isValid()) return;

    if (item.column() != 2)
        item = item.sibling(item.row(),2);

    QString filteredLine = item.data().toString();
    QMenu* menu = new QMenu(this);

    // Join Channel Action
    QAction *joinAction = new QAction(menu);
    joinAction->setText(i18n("Join Channel"));
    joinAction->setIcon(QIcon::fromTheme(QStringLiteral("irc-join-channel")));
    menu->addAction(joinAction);
    connect(joinAction, SIGNAL(triggered()), this, SLOT(joinChannelClicked()));

    // Adds a separator between the Join action and the URL(s) submenu
    menu->addSeparator();

    // open URL submenu
    QMenu* showURLmenu = new QMenu(i18n("Open URL"), menu);

    QList<QPair<int, int> > urlRanges = Konversation::getUrlRanges(filteredLine);
    QPair<int, int> urlRange;
    QListIterator<QPair<int, int> > i(urlRanges);

    while (i.hasNext())
    {
        urlRange = i.next();

        QString url = filteredLine.mid(urlRange.first, urlRange.second);

        QAction* action = new QAction(showURLmenu);
        action->setText(url);
        action->setData(url);

        showURLmenu->addAction(action);

        connect(action, SIGNAL(triggered()), this, SLOT(openURL()));
    }

    if (showURLmenu->actions().count()==0)
        showURLmenu->setEnabled(false);

    menu->addMenu(showURLmenu);
    menu->exec(QCursor::pos());

    delete menu;
}

void ChannelListPanel::openURL()
{
    const QAction* action = static_cast<const QAction*>(sender());

    if (action)
    {
        Application* konvApp = static_cast<Application *>(kapp);
        konvApp->openUrl(action->data().toString());
    }
}

bool ChannelListPanel::closeYourself()
{
    // make the server delete us so server can reset the pointer to us
    m_server->closeChannelListPanel();
    return true;
}

void ChannelListPanel::appendInputText(const QString& text, bool fromCursor)
{
    Q_UNUSED(fromCursor);
    m_filterLine->setText(m_filterLine->text() + text);
}

//Used to disable functions when not connected
void ChannelListPanel::serverOnline(bool online)
{
    m_online = online;
    m_refreshList->setEnabled(m_online);
    m_joinChannel->setEnabled(m_online && m_channelListView->currentIndex().isValid());
}

void ChannelListPanel::emitUpdateInfo()
{
    QString info;
    info = i18n("Channel List for %1", m_server->getDisplayName());
    emit updateInfo(info);
}

void ChannelListPanel::setFilter(const QString& filter)
{
    m_filterLine->setText(filter);
}

#include "channellistpanel.moc"

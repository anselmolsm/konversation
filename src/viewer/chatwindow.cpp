/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2002 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
*/

#include "chatwindow.h"
#include "channel.h"
#include "ircview.h"
#include "server.h"
#include "application.h" ////// header renamed
#include "logfilereader.h"

#include <qdatetime.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextcodec.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QKeyEvent>
#include <QEvent>

#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kactioncollection.h>
#include <kaction.h>


ChatWindow::ChatWindow(QWidget* parent) : KVBox(parent)
{
    setName("ChatWindowObject");
    setTextView(0);
    firstLog=true;
    m_server=0;
    m_notificationsEnabled = true;
    m_channelEncodingSupported = false;
    m_currentTabNotify = Konversation::tnfNone;

    setMargin(margin());
    setSpacing(spacing());

    // The font size of the KTabWidget container may be inappropriately
    // small due to the "Tab bar" font size setting.
    setFont(KGlobalSettings::generalFont());
}

ChatWindow::~ChatWindow()
{
    emit closing(this);
    m_server=0;
}

void ChatWindow::updateAppearance()
{
    // The font size of the KTabWidget container may be inappropriately
    // small due to the "Tab bar" font size setting.
    setFont(KGlobalSettings::generalFont());

    if (textView)
    { /* TODO FIXME
        if (Preferences::showIRCViewScrollBar())
            textView->setVScrollBarMode(Q3ScrollView::AlwaysOn);
        else
            textView->setVScrollBarMode(Q3ScrollView::AlwaysOff);
        */
    }
}

void ChatWindow::setName(const QString& newName)
{
    name=newName;
    emit nameChanged(this,newName);
}

QString ChatWindow::getName()
{
    return name;
}

void ChatWindow::setType(WindowType newType)
{
    type=newType;
}

ChatWindow::WindowType ChatWindow::getType()
{
    return type;
}

void ChatWindow::setServer(Server* newServer)
{
    if (!newServer)
    {
        kDebug("ChatWindow::setServer(0)!") << endl;
    }
    else
    {
        m_server=newServer;
        connect(m_server,SIGNAL (serverOnline(bool)),this,SLOT (serverOnline(bool)) );

        // check if we need to set up the signals
        if(getType() != ChannelList)
        {
            if(textView) textView->setServer(newServer);
            else kDebug() << "ChatWindow::setServer(): textView==0!" << endl;
        }

        emit serverOnline(m_server->isConnected());
    }
}

Server* ChatWindow::getServer()
{
    return m_server;
}

void ChatWindow::serverOnline(bool /* state */)
{
    //emit online(this,state);
}

void ChatWindow::setTextView(IRCView* newView)
{
    textView = newView;

    if(!textView)
    {
        return;
    }

    if(Preferences::self()->showIRCViewScrollBar())
    { // TODO FIXME
        //textView->setVScrollBarMode(Q3ScrollView::Auto);
    }
    else
    { // TODO FIXME
        //textView->setVScrollBarMode(Q3ScrollView::AlwaysOff);
    }

    textView->setChatWin(this);
    connect(textView,SIGNAL(textToLog(const QString&)), this,SLOT(logText(const QString&)));
    connect(textView,SIGNAL(setStatusBarTempText(const QString&)), this, SIGNAL(setStatusBarTempText(const QString&)));
    connect(textView,SIGNAL(clearStatusBarTempText()), this, SIGNAL(clearStatusBarTempText()));
}

void ChatWindow::appendRaw(const QString& message, bool suppressTimestamps)
{
    if(!textView) return;
    textView->appendRaw(message, suppressTimestamps);
}

void ChatWindow::append(const QString& nickname,const QString& message)
{
    if(!textView) return ;
    textView->append(nickname,message);
}

void ChatWindow::appendQuery(const QString& nickname,const QString& message, bool inChannel)
{
    if(!textView) return ;
    textView->appendQuery(nickname,message, inChannel);
}

void ChatWindow::appendAction(const QString& nickname, const QString& message)
{
    if(!textView) return;

    if (getType() == Query || getType() == DccChat)
        textView->appendQueryAction(nickname, message);
    else
        textView->appendChannelAction(nickname, message);
}

void ChatWindow::appendServerMessage(const QString& type,const QString& message, bool parseURL)
{
    if(!textView) return ;
    textView->appendServerMessage(type,message, parseURL);
}

void ChatWindow::appendCommandMessage(const QString& command,const QString& message, bool important, bool parseURL, bool self)
{
    if(!textView) return ;
    textView->appendCommandMessage(command,message,important, parseURL, self);
}

void ChatWindow::appendBacklogMessage(const QString& firstColumn,const QString& message)
{
    if(!textView) return ;
    textView->appendBacklogMessage(firstColumn,message);
}

void ChatWindow::cdIntoLogPath()
{
    QDir logPath=QDir::home();
    // Try to "cd" into the logfile path
    if(!logPath.cd(Preferences::self()->logfilePath()))
    {
        // Only create log path if logging is enabled
        if(log)
        {
            // Try to create the logfile path and "cd" into it again
            logPath.mkpath(Preferences::self()->logfilePath());
            logPath.cd(Preferences::self()->logfilePath());
        }
    }

    // add the logfile name to the path
    logfile.setFileName(logPath.path()+'/'+logName);
}

void ChatWindow::setLogfileName(const QString& name)
{
    // Only change name of logfile if the window was new.
    if(firstLog)
    {
        // status panels get special treatment here, since they have no server at the beginning
        if (getType() == Status || getType() == DccChat)
        {
            logName = name + ".log";
        }
        else if (m_server)
        {
            // make sure that no path delimiters are in the name
            logName = QString(m_server->getDisplayName().toLower()).append('_').append(name).append(".log").replace('/','_');
        }

        // load backlog to show
        if(Preferences::self()->showBacklog())
        {
            // "cd" into log path or create path, if it's not there
            cdIntoLogPath();
            // Show last log lines. This idea was stole ... um ... inspired by PMP :)
            // Don't do this for the server status windows, though
            if((getType() != Status) && logfile.open(QIODevice::ReadOnly))
            {
                unsigned long filePosition;

                QString backlogLine;
                Q3TextStream backlog(&logfile);
                backlog.setEncoding(Q3TextStream::UnicodeUTF8);

                QStringList firstColumns;
                QStringList messages;
                int offset = 0;
                unsigned int lastPacketHeadPosition = backlog.device()->size();
                const unsigned int packetSize = 4096;
                while(messages.count() < (int)Preferences::self()->backlogLines() && backlog.device()->size() > packetSize * offset)
                {
                    QStringList firstColumnsInPacket;
                    QStringList messagesInPacket;

                    // packetSize * offset < size <= packetSize * ( offset + 1 )

                    // Check if the log is bigger than packetSize * ( offset + 1 )
                    if(backlog.device()->size() > packetSize * ( offset + 1 ))
                    {
                        // Set file pointer to the packet size above the offset
                        backlog.device()->at(backlog.device()->size() - packetSize * ( offset + 1 ));
                        // Skip first line, since it may be incomplete
                        backlog.readLine();
                    }
                    else
                    {
                        // Set file pointer to the head
                        backlog.device()->reset();
                    }

                    unsigned int currentPacketHeadPosition = backlog.device()->at();

                    // Loop until end of file reached
                    while(!backlog.atEnd() && backlog.device()->at() < lastPacketHeadPosition)
                    {
                        // remember actual file position to check for deadlocks
                        filePosition = backlog.device()->at();
                        backlogLine = backlog.readLine();

                        // check for deadlocks
                        if(backlog.device()->at() == filePosition) backlog.device()->at(filePosition + 1);

                        // if a tab character is present in the line
                        if (backlogLine.contains('\t'))
                        {
                            // extract first column from log
                            QString backlogFirst = backlogLine.left(backlogLine.indexOf('\t'));
                            // cut first column from line
                            backlogLine = backlogLine.mid(backlogLine.indexOf('\t') + 1);
                            // Logfile is in utf8 so we don't need to do encoding stuff here
                            // append backlog with time and first column to text view
                            firstColumnsInPacket << backlogFirst;
                            messagesInPacket << backlogLine;
                        }
                    } // while

                    // remember the position not to read the same lines again
                    lastPacketHeadPosition = currentPacketHeadPosition;
                    ++offset;

                    firstColumns = firstColumnsInPacket + firstColumns;
                    messages = messagesInPacket + messages;
                }
                backlog.unsetDevice();
                logfile.close();

                // trim
                int surplus = messages.count() - Preferences::self()->backlogLines();
                // "surplus" can be a minus value. (when the backlog is too short)
                if(surplus > 0)
                {
                    for(int i = 0 ; i < surplus ; ++i)
                    {
                        firstColumns.pop_front();
                        messages.pop_front();
                    }
                }

                QStringList::Iterator itFirstColumn = firstColumns.begin();
                QStringList::Iterator itMessage = messages.begin();
                for( ; itFirstColumn != firstColumns.end() ; ++itFirstColumn, ++itMessage )
                    appendBacklogMessage(*itFirstColumn, *itMessage);
            }
        } // if(Preferences::showBacklog())
    }
}

void ChatWindow::logText(const QString& text)
{
    if(log)
    {
        // "cd" into log path or create path, if it's not there
        cdIntoLogPath();

        if(logfile.open(QIODevice::WriteOnly | QIODevice::Append))
        {
            // wrap the file into a stream
            Q3TextStream logStream(&logfile);
            // write log in utf8 to help i18n
            logStream.setEncoding(Q3TextStream::UnicodeUTF8);

            if(firstLog)
            {
                QString intro(i18n("\n*** Logfile started\n*** on %1\n\n", QDateTime::currentDateTime().toString()));
                logStream << intro;
                firstLog=false;
            }

            QTime time=QTime::currentTime();
            QString logLine(QString("[%1] [%2] %3\n").arg(QDate::currentDate().toString()).
                arg(time.toString("hh:mm:ss")).arg(text));

            logStream << logLine;

            // detach stream from file
            logStream.unsetDevice();

            // close file
            logfile.close();
        }
        else kWarning() << "ChatWindow::logText(): open(QIODevice::Append) for " << logfile.fileName() << " failed!" << endl;
    }
}

void ChatWindow::setChannelEncodingSupported(bool enabled)
{
    m_channelEncodingSupported = enabled;
}

bool ChatWindow::isChannelEncodingSupported() const
{
    return m_channelEncodingSupported;
}

int ChatWindow::spacing()
{
    if(Preferences::self()->useSpacing())
        return Preferences::self()->spacing();
    else
        return KDialog::spacingHint();
}

int ChatWindow::margin()
{
    if(Preferences::self()->useSpacing())
        return Preferences::self()->margin();
    else
        return 0;
}

// Accessors
IRCView* ChatWindow::getTextView() const
{
  return textView;
}

void ChatWindow::setLog(bool activate)
{
  log=activate;
}

// reimplement this in all panels that have user input
QString ChatWindow::getTextInLine()
{
  return QString();
}

bool ChatWindow::canBeFrontView()
{
  return false;
}

bool ChatWindow::searchView()
{
  return false;
}

// reimplement this in all panels that have user input
void ChatWindow::indicateAway(bool)
{
}

// reimplement this in all panels that have user input
void ChatWindow::appendInputText(const QString&, bool)
{
}

// reimplement this if your window needs special close treatment
bool ChatWindow::closeYourself(bool /* askForConfirmation */)
{
    deleteLater();

    return true;
}

bool ChatWindow::eventFilter(QObject* watched, QEvent* e)
{
    if(e->type() == QEvent::KeyPress)
    {
        QKeyEvent* ke = static_cast<QKeyEvent*>(e);

        bool scrollMod = (Preferences::self()->useMultiRowInputBox() ? false : (ke->state() == Qt::ShiftModifier));

        if(ke->key() == Qt::Key_Up && scrollMod)
        {
            if(textView)
            {
                QScrollBar* sbar = textView->verticalScrollBar();
                sbar->setValue(sbar->value() - sbar->singleStep());
            }

            return true;
        }
        else if(ke->key() == Qt::Key_Down && scrollMod)
        {
            if(textView)
            {
                QScrollBar* sbar = textView->verticalScrollBar();
                sbar->setValue(sbar->value() + sbar->singleStep());
            }

            return true;
        }
        else if(ke->key() == Qt::Key_PageUp)
        {
            if(textView)
            {
                QScrollBar* sbar = textView->verticalScrollBar();
                sbar->setValue(sbar->value() - sbar->pageStep());
            }

            return true;
        }
        else if(ke->key() == Qt::Key_PageDown)
        {
            if(textView)
            {
                QScrollBar* sbar = textView->verticalScrollBar();
                sbar->setValue(sbar->value() + sbar->pageStep());
            }

            return true;
        }

    }

    return KVBox::eventFilter(watched, e);
}

void ChatWindow::adjustFocus()
{
    childAdjustFocus();
}

void ChatWindow::emitUpdateInfo()
{
    QString info = getName();
    emit updateInfo(info);
}

QColor ChatWindow::highlightColor()
{
    return getTextView()->highlightColor();
}

void ChatWindow::activateTabNotification(Konversation::TabNotifyType type)
{
    if (!notificationsEnabled())
        return;

    if(type > m_currentTabNotify)
        return;

    m_currentTabNotify = type;

    emit updateTabNotification(this,type);
}

void ChatWindow::resetTabNotification()
{
    m_currentTabNotify = Konversation::tnfNone;
}

// #include "./viewer/chatwindow.moc"

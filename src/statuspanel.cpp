/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2003 Dario Abatianni <eisfuchs@tigress.com>
  Copyright (C) 2006-2008 Eike Hein <hein@kde.org>
*/

#include "statuspanel.h"
#include "channel.h"
#include "application.h" ////// header renamed
#include "ircinput.h"
#include "ircview.h"
#include "ircviewbox.h"
#include "server.h"

#include <qpushbutton.h>
#include <qlabel.h>

#include <qtextcodec.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kvbox.h>
#include <kcombobox.h>


StatusPanel::StatusPanel(QWidget* parent) : ChatWindow(parent)
{
    setType(ChatWindow::Status);

    setChannelEncodingSupported(true);

    awayChanged=false;
    awayState=false;

    // set up text view, will automatically take care of logging
    IRCViewBox* ircBox = new IRCViewBox(this, 0); // Server will be set later in setServer()
    setTextView(ircBox->ircView());

    KHBox* commandLineBox=new KHBox(this);
    commandLineBox->setSpacing(spacing());
    commandLineBox->setMargin(0);

    nicknameCombobox=new KComboBox(commandLineBox);
    nicknameCombobox->setEditable(true);
    nicknameCombobox->setSizeAdjustPolicy(KComboBox::AdjustToContents);
    nicknameCombobox->setWhatsThis(i18n("<qt><p>This shows your current nick, and any alternatives you have set up.  If you select or type in a different nickname, then a request will be sent to the IRC server to change your nick.  If the server allows it, the new nickname will be selected.  If you type in a new nickname, you need to press 'Enter' at the end.</p><p>You can add change the alternative nicknames from the <em>Identities</em> option in the <em>File</em> menu.</p></qt>"));

    awayLabel=new QLabel(i18n("(away)"), commandLineBox);
    awayLabel->hide();
    statusInput=new IRCInput(commandLineBox);

    getTextView()->installEventFilter(statusInput);
    statusInput->installEventFilter(this);

    setLog(Preferences::self()->log());

    connect(getTextView(),SIGNAL (gotFocus()),statusInput,SLOT (setFocus()) );

    connect(getTextView(),SIGNAL (sendFile()),this,SLOT (sendFileMenu()) );
    connect(getTextView(),SIGNAL (autoText(const QString&)),this,SLOT (sendStatusText(const QString&)) );

    connect(statusInput,SIGNAL (submit()),this,SLOT(statusTextEntered()) );
    connect(statusInput,SIGNAL (textPasted(const QString&)),this,SLOT(textPasted(const QString&)) );
    connect(getTextView(), SIGNAL(textPasted(bool)), statusInput, SLOT(paste(bool)));
    connect(getTextView(), SIGNAL(popupCommand(int)), this, SLOT(popupCommand(int)));

    connect(nicknameCombobox,SIGNAL (activated(int)),this,SLOT(nicknameComboboxChanged()));
    Q_ASSERT(nicknameCombobox->lineEdit());       //it should be editedable.  if we design it so it isn't, remove these lines.
    if(nicknameCombobox->lineEdit())
        connect(nicknameCombobox->lineEdit(), SIGNAL (editingFinished()),this,SLOT(nicknameComboboxChanged()));

    updateAppearance();
}

StatusPanel::~StatusPanel()
{
}

void StatusPanel::serverSaysClose()
{
    closeYourself(false);
}

void StatusPanel::setNickname(const QString& newNickname)
{
    nicknameCombobox->setCurrentIndex(nicknameCombobox->findText(newNickname));
}

void StatusPanel::childAdjustFocus()
{
    statusInput->setFocus();
}

void StatusPanel::sendStatusText(const QString& sendLine)
{
    // create a work copy
    QString outputAll(sendLine);
    // replace aliases and wildcards
    if(m_server->getOutputFilter()->replaceAliases(outputAll))
    {
        outputAll = m_server->parseWildcards(outputAll, m_server->getNickname(), QString(), QString(), QString(), QString());
    }

    // Send all strings, one after another
    QStringList outList=outputAll.split('\n');
    for(int index=0;index<outList.count();index++)
    {
        QString output(outList[index]);

        // encoding stuff is done in Server()
        Konversation::OutputFilterResult result = m_server->getOutputFilter()->parse(m_server->getNickname(), output, QString());

        if(!result.output.isEmpty())
        {
            appendServerMessage(result.typeString, result.output);
        }
        m_server->queue(result.toServer);
    } // for
}

void StatusPanel::statusTextEntered()
{
    QString line=statusInput->toPlainText();
    statusInput->clear();

    if(line.toLower()==Preferences::self()->commandChar()+"clear") textView->clear();
    else
    {
        if(line.length()) sendStatusText(line);
    }
}

void StatusPanel::textPasted(const QString& text)
{
    if(m_server)
    {
        QStringList multiline=text.split('\n');
        for(int index=0;index<multiline.count();index++)
        {
            QString line=multiline[index];
            QString cChar(Preferences::self()->commandChar());
            // make sure that lines starting with command char get escaped
            if(line.startsWith(cChar)) line=cChar+line;
            sendStatusText(line);
        }
    }
}

void StatusPanel::updateAppearance()
{
    QColor fg;
    QColor bg;
    if(Preferences::self()->inputFieldsBackgroundColor())
    {
        fg=Preferences::self()->color(Preferences::ChannelMessage);
        bg=Preferences::self()->color(Preferences::TextViewBackground);
    }
    else
    {
        fg = palette().windowText().color();
        bg = palette().base().color();
    }

    QPalette statusInputPalette(statusInput->palette());
    statusInputPalette.setColor(QPalette::WindowText, fg);
    statusInputPalette.setColor(QPalette::Text, fg);
    statusInputPalette.setColor(QPalette::Base, bg);
    statusInput->setPalette(statusInputPalette);

    getTextView()->setPalette(QPalette()); 

    if(Preferences::self()->showBackgroundImage())
    {
        getTextView()->setViewBackground(Preferences::self()->color(Preferences::TextViewBackground),
            Preferences::self()->backgroundImage());
    }
    else
    {
        getTextView()->setViewBackground(Preferences::self()->color(Preferences::TextViewBackground),
            QString());
    }

    if (Preferences::self()->customTextFont())
    {
        getTextView()->setFont(Preferences::self()->textFont());
        statusInput->setFont(Preferences::self()->textFont());
        nicknameCombobox->setFont(Preferences::self()->textFont());
    }
    else
    {
        getTextView()->setFont(KGlobalSettings::generalFont());
        statusInput->setFont(KGlobalSettings::generalFont());
        nicknameCombobox->setFont(KGlobalSettings::generalFont());
    }

    showNicknameBox(Preferences::self()->showNicknameBox());

    ChatWindow::updateAppearance();
}

void StatusPanel::setName(const QString& newName)
{
    ChatWindow::setName(newName);
    setLogfileName(newName.toLower());
}

void StatusPanel::updateName()
{
    QString newName = getServer()->getDisplayName();
    setName(newName);
    setLogfileName(newName.toLower());
}

void StatusPanel::sendFileMenu()
{
    emit sendFile();
}

void StatusPanel::indicateAway(bool show)
{
    // QT does not redraw the label properly when they are not on screen
    // while getting hidden, so we remember the "soon to be" state here.
    if(isHidden())
    {
        awayChanged=true;
        awayState=show;
    }
    else
    {
        if(show)
            awayLabel->show();
        else
            awayLabel->hide();
    }
}

// fix Qt's broken behavior on hidden QListView pages
void StatusPanel::showEvent(QShowEvent*)
{
    if(awayChanged)
    {
        awayChanged=false;
        indicateAway(awayState);
    }
}

QString StatusPanel::getTextInLine() { return statusInput->toPlainText(); }

bool StatusPanel::canBeFrontView()        { return true; }
bool StatusPanel::searchView()       { return true; }

void StatusPanel::setNotificationsEnabled(bool enable)
{
    if (m_server->getServerGroup()) m_server->getServerGroup()->setNotificationsEnabled(enable);

    m_notificationsEnabled = enable;
}

bool StatusPanel::closeYourself(bool confirm)
{
    int result;

    //FIXME: Show "Do you really want to close ..." warnings in
    // disconnected state instead of closing directly. Can't do
    // that due to string freeze at the moment.
    if (confirm && !m_server->isConnected())
    {
        result = KMessageBox::warningContinueCancel(
                this,
                i18n("Do you really want to close '%1'?\n\n All associated tabs will be closed as well.",getName()),
                i18n("Close Tab"),
                KStandardGuiItem::close(),
                KStandardGuiItem::cancel(),
                "QuitServerTab");
    }
    else
    {
        result = KMessageBox::warningContinueCancel(
            this,
            i18n("Do you want to disconnect from '%1'?\n\n All associated tabs will be closed as well.",m_server->getServerName()),
            i18n("Disconnect From Server"),
            KGuiItem(i18n("Disconnect")),
            KStandardGuiItem::cancel(),
            "QuitServerTab");
    }

    if (result==KMessageBox::Continue)
    {
        if (m_server->getServerGroup()) m_server->getServerGroup()->setNotificationsEnabled(notificationsEnabled());
        m_server->quitServer();
        // This will delete the status view as well.
        m_server->deleteLater();
        m_server = 0;
        return true;
    }
    return false;
}

void StatusPanel::nicknameComboboxChanged()
{
    QString newNick=nicknameCombobox->currentText();
    oldNick=m_server->getNickname();
    if (oldNick != newNick)
    {
        nicknameCombobox->setCurrentIndex(nicknameCombobox->findText(oldNick));
        m_server->queue("NICK "+newNick);
    }
    // return focus to input line
    statusInput->setFocus();
}

void StatusPanel::changeNickname(const QString& newNickname)
{
    m_server->queue("NICK "+newNickname);
}

void StatusPanel::emitUpdateInfo()
{
    emit updateInfo(getServer()->getDisplayName());
}

void StatusPanel::appendInputText(const QString& text, bool fromCursor)
{
    if (!fromCursor)
    {
        QTextCursor c(statusInput->textCursor());
        c.movePosition(QTextCursor::End);
        c.insertText(text);
    }
    else
    {
        statusInput->textCursor().insertText(text);
    }
}
                                                  // virtual
void StatusPanel::setChannelEncoding(const QString& encoding)
{
    Preferences::setChannelEncoding(m_server->getDisplayName(), ":server", encoding);
}

QString StatusPanel::getChannelEncoding()         // virtual
{
    return Preferences::channelEncoding(m_server->getDisplayName(), ":server");
}

                                                  // virtual
QString StatusPanel::getChannelEncodingDefaultDesc()
{
    return i18n("Identity Default ( %1 )", getServer()->getIdentity()->getCodecName());
}

//Used to disable functions when not connected
void StatusPanel::serverOnline(bool online)
{
    //statusInput->setEnabled(online);
    getTextView()->setNickAndChannelContextMenusEnabled(online);
    nicknameCombobox->setEnabled(online);
}

void StatusPanel::showNicknameBox(bool show)
{
    if(show)
    {
        nicknameCombobox->show();
    }
    else
    {
        nicknameCombobox->hide();
    }
}

void StatusPanel::setServer(Server* server)
{
    ChatWindow::setServer(server);
    nicknameCombobox->setModel(m_server->nickListModel());
}

void StatusPanel::popupCommand(int command)
{
    switch(command)
    {
        case Konversation::Join:
            m_server->queue("JOIN " + getTextView()->currentChannel());
            break;
        case Konversation::Topic:
            m_server->requestTopic(getTextView()->currentChannel());
            break;
        case Konversation::Names:
            m_server->queue("NAMES " + getTextView()->currentChannel(), Server::LowPriority);
            break;
    }
}

#include "statuspanel.moc"

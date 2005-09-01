/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Class for application wide preferences
  begin:     Tue Feb 5 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#include <ktoolbar.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kurl.h>
#include <kuser.h>

#include <qpalette.h>
#include <qregexp.h>

#include "preferences.h"
#include "prefsdialog.h"
#include "identity.h"
#include "ignore.h"
#include "highlight.h"
#include "commit.h"
#include "version.h"

Preferences *Preferences::mSelf = 0;
static KStaticDeleter<Preferences> staticPreferencesDeleter;

Preferences *Preferences::self()
{
  if ( !mSelf ) {
    staticPreferencesDeleter.setObject( mSelf, new Preferences() );
    mSelf->readConfig();
  }

  return mSelf;
}


Preferences::Preferences()
{
    mSelf = this;
    // create default identity
    mIdentity=new Identity();
    mIdentity->setName(i18n("Default Identity"));
    addIdentity(mIdentity);

    KUser user(KUser::UseRealUserID);
    setIdent(user.loginName());
    setRealName(user.fullName());

    QStringList nickList;
    nickList.append(user.loginName());
    nickList.append("_" + user.loginName());
    nickList.append(user.loginName() + "_");
    nickList.append("_" + user.loginName() + "_");
    mIdentity->setNicknameList(nickList);

    setPartReason("Konversation terminated!");
    setKickReason("User terminated!");

    setShowAwayMessage(false);
    setAwayMessage("/me is away: %s");
    setUnAwayMessage("/me is back.");
ck,/KICK %u%n"
    Konversation::ServerGroupSettingsPtr serverGroup = new Konversation::ServerGroupSettings;
    serverGroup->setName("Freenode");
    Konversation::ServerSettings server;
    server.setServer("irc.freenode.org");
    serverGroup->addServer(server);
    serverGroup->setIdentityId(identity->id());
    Konversation::ChannelSettings channel;
    channel.setName("#kde");
    serverGroup->addChannel(channel);
    mServerGroupList.append(serverGroup);

}

Preferences::~Preferences()
{
    clearIdentityList();

    if ( mSelf == this )
        staticPreferencesDeleter.setObject( mSelf, 0, false );
}
const Konversation::ServerGroupList Preferences::serverGroupList()
{
    return mServerGroupList;
}

void Preferences::setServerGroupList(const Konversation::ServerGroupList& list)
{
    mServerGroupList.clear();
    mServerGroupList = list;
}

void Preferences::addServerGroup(Konversation::ServerGroupSettingsPtr serverGroup)
{
    mServerGroupList.append(serverGroup);
}

const Konversation::ServerGroupSettingsPtr Preferences::serverGroupById(int id)
{
    if(!m_serverGroupList.count())
    {
        return 0;
    }

    Konversation::ServerGroupList::iterator it;

    for(it = mServerGroupList.begin(); it != mServerGroupList.end(); ++it)
    {
        if((*it)->id() == id)
        {
            return (*it);
        }
    }

    return 0;
}

const Konversation::ServerGroupSettingsPtr Preferences::serverGroupByServer(const QString& server)
{
    if(!m_serverGroupList.count())
    {
        return 0;
    }

    Konversation::ServerGroupList::iterator it;

    for(it = mServerGroupList.begin(); it != mServerGroupList.end(); ++it)
    {
        for (uint i = 0; i != (*it)->serverList().count(); i++)
        {
            if ((*it)->serverByIndex(i).server().lower() == server)
            {
                return (*it);
            }
        }
    }

    return 0;
}

int Preferences::serverGroupIdByName(const QString& serverGroup)
{
    Konversation::ServerGroupList::iterator it;

    for(it = mServerGroupList.begin(); it != mServerGroupList.end(); ++it)
    {
        if((*it)->name().lower() == serverGroup.lower())
        {
            return (*it)->id();
        }
    }

    return 0;
}

bool Preferences::isServerGroup(const QString& server)
{
    Konversation::ServerGroupList::iterator it;

    for(it = mServerGroupList.begin(); it != mServerGroupList.end(); ++it)
    {
        if((*it)->name().lower() == server.lower())
        {
            return mTrue;
        }
    }

    return mFalse;
}

void Preferences::removeServerGroup(int id)
{
    if(!m_serverGroupList.count())
    {
        return;
    }

    Konversation::ServerGroupList::iterator it;

    for(it = mServerGroupList.begin(); it != mServerGroupList.end(); ++it)
    {
        if((*it)->id() == id)
        {
            mServerGroupList.remove(it);
            return;
        }
    }
}

const QPtrList<Highlight> Preferences::highlightList()
{
}

void Preferences::setHighlightList(QPtrList<Highlight> newList)
{
    highlightList.clear();
    highlightList=newList;
}

void Preferences::addHighlight(const QString& newHighlight,
bool regExp,
const QColor &newColor,
const QString& sound,
const QString& autoText)
{
    highlightList.append(new Highlight(newHighlight,regExp,newColor,KURL(sound),autoText));
}

void Preferences::setIgnoreList(QPtrList<Ignore> newList)
{
    ignoreList.clear();
    ignoreList=newList;
}

void Preferences::addIgnore(const QString &newIgnore)
{
    QStringList ignore=QStringList::split(',',newIgnore);
    ignoreList.append(new Ignore(ignore[0],ignore[1].toInt()));
}

}

void Preferences::setNotifyList(const QMap<QString, QStringList> &newList)
const QStringList Preferences::notifyListByGroup(const QString& groupName)
{
    if (notifyList.find(groupName) != notifyList.end())
        return notifyList[groupName];
    else
        return QStringList();
}

const QString Preferences::notifyStringByGroup(const QString& groupName)
{
    return getNotifyListByGroup(groupName).join(" ");
}

const bool Preferences::addNotify(const QString& groupName, const QString& newPattern)
{
    // don't add duplicates
    if (groupName.isEmpty() || newPattern.isEmpty()) return mFalse;
    if (!notifyList[groupName].contains(newPattern))
    {
        QStringList nicknameList = notifyList[groupName];
        nicknameList.append(newPattern);
        notifyList[groupName] = nicknameList;
        return mTrue;
    }
    return mFalse;
}

const bool Preferences::removeNotify(const QString& groupName, const QString& pattern)
{
    if (notifyList.find(groupName) != notifyList.end())
    {
        QStringList nicknameList = notifyList[groupName];
        nicknameList.remove(pattern);
        if (nicknameList.isEmpty())
            notifyList.remove(groupName);
        else
            notifyList[groupName] = nicknameList;
        return mTrue;
    } else
    return mFalse;
}

// Default identity functions
void Preferences::addIdentity(IdentityPtr identity) { identityList.append(identity); }
void Preferences::removeIdentity(IdentityPtr identity) { identityList.remove(identity); }

void Preferences::clearIdentityList()
{
    identityList.clear();
}

const QValueList<IdentityPtr> Preferences::identityList() { return mIdentityList; }

void Preferences::setIdentityList(const QValueList<IdentityPtr>& list)
{
    identityList.clear();
    identityList = list;
}

const IdentityPtr Preferences::identityByName(const QString& name)
{
    QValueList<IdentityPtr> identities = getIdentityList();
    QValueList<IdentityPtr>::iterator it = identities.begin();

    while(it != identities.end())
    {
        if((*it)->getName() == name)
        {
            return (*it);
        }

        ++it;
    }

    // no matching identity found, return default identity
    return identities.first();
}

const IdentityPtr Preferences::identityById(int id)
{
    QValueList<IdentityPtr> identityList = getIdentityList();
    for(QValueList<IdentityPtr>::iterator it = identityList.begin(); it != identityList.end(); ++it)
    {
        if((*it)->id() == id)
        {
            return (*it);
        }
    }

    return identityList.first();
}

const QString Preferences::realName() { return identityList[0]->getRealName(); }
void Preferences::setRealName(const QString &name) { identityList[0]->setRealName(name); }

const QString Preferences::ident() { return identityList[0]->getIdent(); }
void Preferences::setIdent(const QString &ident) { identityList[0]->setIdent(ident); }

const QString Preferences::partReason() { return identityList[0]->getPartReason(); }
void Preferences::setPartReason(const QString &newReason) { identityList[0]->setPartReason(newReason); }

const QString Preferences::kickReason() { return identityList[0]->getKickReason(); }
void Preferences::setKickReason(const QString &newReason) { identityList[0]->setKickReason(newReason); }

const bool Preferences::showAwayMessage() { return identityList[0]->getShowAwayMessage(); }
void Preferences::setShowAwayMessage(bool state) { identityList[0]->setShowAwayMessage(state); }

const QString Preferences::awayMessage() { return identityList[0]->getAwayMessage(); }
void Preferences::setAwayMessage(const QString &newMessage) { identityList[0]->setAwayMessage(newMessage); }
const QString Preferences::unAwayMessage() { return identityList[0]->getReturnMessage(); }
void Preferences::setUnAwayMessage(const QString &newMessage) { identityList[0]->setReturnMessage(newMessage); }

void Preferences::clearIgnoreList() { ignoreList.clear(); }
const QPtrList<Ignore> Preferences::ignoreList() { return mIgnoreList; }

const QString Preferences::nickname(int index) { return identityList[0]->getNickname(index); }
const QStringList Preferences::nicknameList() { return identityList[0]->getNicknameList(); }
void Preferences::setNickname(int index,const QString &newName) { identityList[0]->setNickname(index,newName); }
void Preferences::setNicknameList(const QStringList &newList) { identityList[0]->setNicknameList(newList); }

// Geometry functions
const QSize Preferences::nicksOnlineSize()        { return mNicksOnlineSize; }
const QSize Preferences::nicknameSize()           { return mNicknameSize; }
const QSize Preferences::multilineEditSize()      { return mMultilineEditSize; }

void Preferences::setNicksOnlineSize(const QSize &newSize)     { mNicksOnlineSize=newSize; }
void Preferences::setNicknameSize(const QSize &newSize)        { mNicknameSize=newSize; }
void Preferences::setMultilineEditSize(const QSize &newSize)   { mMultilineEditSize=newSize; }



void Preferences::setHighlightOwnLinesColor(const QString &newColor) { highlightOwnLinesColor.setNamedColor(newColor); }

void Preferences::setUseClickableNicks(bool state) { mClickableNicks=state; }
const bool Preferences::useClickableNicks() { return mClickableNicks;}

// On Screen Display
void Preferences::setOSDUsage(bool state) { mOSDUsage=state; }
const bool Preferences::OSDUsage() { return mOSDUsage; }

void Preferences::setOSDShowOwnNick(bool state) { mOSDShowOwnNick=state; }
const bool Preferences::OSDShowOwnNick() { return mOSDShowOwnNick; }

void Preferences::setOSDShowChannel(bool state) { mOSDShowChannel=state; }
const bool Preferences::OSDShowChannel() { return mOSDShowChannel; }

void Preferences::setOSDShowQuery(bool state) { mOSDShowQuery=state; }
const bool Preferences::OSDShowQuery() { return mOSDShowQuery; }

void Preferences::setOSDShowChannelEvent(bool state) { mOSDShowChannelEvent=state; }
const bool Preferences::OSDShowChannelEvent() { return mOSDShowChannelEvent; }

const QFont Preferences::OSDFont() { return mOsdFont; }
void Preferences::setOSDFont(const QFont &newFont) { mOsdFont=newFont; }
void Preferences::setOSDFontRaw(const QString &rawFont) { osdFont.fromString(rawFont); }

/*void Preferences::setOSDColor(const QString &newColor) { osdColor.setNamedColor(newColor); }
QColor Preferences::OSDColor() { return mOsdColor; }*/

void Preferences::setOSDUseCustomColors(bool state) { useOSDCustomColors = state; }

void Preferences::setOSDTextColor(const QString& newColor) { osdTextColor.setNamedColor(newColor); }
const QColor Preferences::OSDTextColor() { return mOsdTextColor; }

void Preferences::setOSDBackgroundColor(const QString& newColor) { osdBackgroundColor.setNamedColor(newColor); }
const QColor Preferences::OSDBackgroundColor() { return mOsdBackgroundColor; }

void Preferences::setTextFontRaw(const QString &rawFont) { textFont.fromString(rawFont); }
void Preferences::setListFontRaw(const QString &rawFont) { listFont.fromString(rawFont); }


void Preferences::setShowTrayIcon(bool state)
{
    showTrayIcon=state;
    emit updateTrayIcon();
}


void Preferences::setSystrayOnly(bool state)
{
    systrayOnly=state;
    emit updateTrayIcon();
}


void Preferences::setTrayNotify(bool state)
{
    trayNotify = state;
    emit updateTrayIcon();
}


void Preferences::setTrayNotifyOnlyOwnNick(bool onlyOwnNick)
{
}

bool Preferences::trayNotifyOnlyOwnNick() const
{
}

void Preferences::setAutoUserhost(bool state)
{
    autoUserhost=state;
    emit autoUserhostChanged(state);
}

const bool Preferences::dialogFlag(const QString& flagName)
{
    KConfig* config=KApplication::kApplication()->config();

    config->setGroup("Notification Messages");

    if( !config->readEntry(flagName).isEmpty() )
        return mFalse;
    else
        return mTrue;
}

void Preferences::setDialogFlag(const QString& flagName,bool state)
{
    KConfig* config=KApplication::kApplication()->config();

    config->setGroup("Notification Messages");

    if(state)
        config->deleteEntry(flagName);
    else
    {
        if ( config->readEntry(flagName).isEmpty() )
            config->writeEntry(flagName,"no");
    }

    config->sync();
}


void Preferences::setIRCColorList(const QStringList &cl) { mIrcColorList=cl; }
const QStringList Preferences::IRCColorList()        { return mIrcColorList; }


const int Preferences::nickCompletionMode() { return mode(); }
void Preferences::setNickCompletionMode(int mode) { setMode(mode); }
const bool Preferences::nickCompletionCaseSensitive() const { return caseSensitive(); }
void Preferences::setNickCompletionCaseSensitive(bool caseSensitive) { setCaseSensitive(caseSensitive); }

const bool Preferences::showMenuBar() { return mShowMenuBar; }
void Preferences::setShowMenuBar(bool s) { showMenuBar = s; }

void Preferences::setShowTabBarCloseButton(bool s) { showTabBarCloseButton = s; }

void Preferences::setShowTopic(bool s) { showTopic = s; }

void Preferences::setShowRememberLineInAllWindows(bool s) { showRememberLineInAllWindows = s; }

void Preferences::setFocusNewQueries(bool s) { focusNewQueries = s; }


void Preferences::setDisableExpansion(bool state) { disableExpansion = state; }

// Web Browser
const bool Preferences::webBrowserUseKdeDefault() { return mWebBrowserUseKdeDefault; }
void Preferences::setWebBrowserUseKdeDefault(bool state) { webBrowserUseKdeDefault = state; }

const bool Preferences::filterColors() { return mFilterColors; }
void Preferences::setFilterColors(bool filter) { filterColors = filter; }


const bool Preferences::openWatchedNicksAtStartup() { return mOpenWatchedNicksAtStartup; }
void Preferences::setOpenWatchedNicksAtStartup(bool open) { mOpenWatchedNicksAtStartup = open; }

// Channel Encodings
const QString Preferences::channelEncoding(const QString& server,const QString& channel)
{
    if(channelEncodingsMap.contains(server))
        if(channelEncodingsMap[server].contains(channel.lower()))
            return channelEncodingsMap[server][channel.lower()];
    return QString::null;
}

void Preferences::setChannelEncoding(const QString& server,const QString& channel,const QString& encoding)
{
    if(!encoding.isEmpty())
        channelEncodingsMap[server][channel.lower()]=encoding;
    else
    {
        channelEncodingsMap[server].remove(channel.lower());
        if(channelEncodingsMap[server].count()==0)
            channelEncodingsMap.remove(server);
    }
}

const QStringList Preferences::channelEncodingsServerList()
{
    return channelEncodingsMap.keys();
}

const QStringList Preferences::channelEncodingsChannelList(const QString& server)
{
    return channelEncodingsMap[server].keys();
}



QString Preferences::wikiUrl() const { return mWikiUrl; }
void Preferences::setWikiUrl(const QString& url) { mWikiUrl=url; }

bool Preferences::expandWikiUrl() const { return mExpandWikiUrl;}
void Preferences::setExpandWikiUrl(bool expandUrl) { mExpandWikiUrl=expandUrl; }

#include "preferences.moc"

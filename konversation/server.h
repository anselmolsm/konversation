/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  server.h  -  Server Class to handle connection to the IRC server
  begin:     Sun Jan 20 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#ifndef SERVER_H
#define SERVER_H

#include <qtimer.h>
#include <qdatetime.h>
#include <qdict.h>

#include "inputfilter.h"
#include "outputfilter.h"
#include "ircserversocket.h"
#include "ircresolver.h"

#include "dcctransfer.h"
#include "nickinfo.h"

/*
  @author Dario Abatianni
*/

class Channel;
class Query;
class StatusPanel;
class Identity;
class KonversationMainWindow;
class RawLog;
class ChannelListPanel;

// A NickInfoList is a list of NickInfo objects, indexed by lowercase nickname.
typedef QDict<NickInfo> NickInfoList;
// A ChannelNick is a user mode and pointer to a NickInfo.
typedef struct ChannelNick { unsigned int mode; NickInfo* nickInfo; } ChannelNick;
// A ChannelNickList is a list of ChannelNicks, indexed by lowercase nickname.
typedef QDict<ChannelNick> ChannelNickList;
// A ChannelMembershipList is a list of ChannelNickLists, indexed by lowercase channel name.
typedef QDict<ChannelNickList> ChannelMembershipList;
// An iterator for a NickInfoList.
typedef QDictIterator<NickInfo> NickInfoListIterator;
// An interator for a ChannelNickList.
typedef QDictIterator<ChannelNick> ChannelNickListIterator;
// An iterator for a ChannelMembershipList.
typedef QDictIterator<ChannelNickList> ChannelMembershipListIterator;

class Server : public QObject
{
  Q_OBJECT

  public:
    Server(KonversationMainWindow* mainWindow,int number);
    ~Server();

    QString getServerName() const;
    QString getServerGroup() const;
    Identity *getIdentity();
    int getPort() const;
    int getLag() const;
    bool getAutoJoin() const;
    void setAutoJoin(bool on);

    bool isConnected() const;

    QString getAutoJoinChannel() const;
    void setAutoJoinChannel(const QString &channel);

    QString getAutoJoinChannelKey() const;
    void setAutoJoinChannelKey(const QString &key);

    void setDeliberateQuit(bool on);
    bool getDeliberateQuit() const;

    QString getNextNickname();

    void setIrcName(const QString &newIrcName);
    QString getIrcName() const;

    void addPendingNickList(const QString& channelName,const QStringList& nickList);
    void addNickToChannel(const QString &channelName,const QString &nickname,const QString &hostmask,
                          bool admin,bool owner,bool op,bool halfop,bool voice);
    void addHostmaskToNick(const QString &sourceNick, const QString &sourceHostmask);
    void nickJoinsChannel(const QString &channelName, const QString &nickname, const QString &hostmask);
    void renameNick(const QString &nickname,const QString &newNick);
    void removeNickFromChannel(const QString &channelName, const QString &nickname, const QString &reason, bool quit=false);
    void nickWasKickedFromChannel(const QString &channelName, const QString &nickname, const QString &kicker, const QString &reason);
    void removeNickFromServer(const QString &nickname, const QString &reason);
    void noMorePendingNicks(const QString& channel);

    // extended user modes support
    void setPrefixes(const QString &modes, const QString& prefixes);
    bool mangleNicknameWithModes(QString &nickname,bool& isAdmin,bool& isOwner,bool &isOp,
                                 bool& isHalfop,bool &hasVoice,char *realMode );

    bool isNickname(const QString& compare);
    QString getNickname() const;
    OutputFilter& getOutputFilter();

    void joinChannel(const QString& name, const QString& hostmask, const QString& key);
    void removeChannel(Channel* channel);
    void appendToChannel(const QString& channel,const QString& nickname, const QString& message);
    void appendActionToChannel(const QString& channel, const QString& nickname, const QString& message);
    void appendServerMessageToChannel(const QString& channel, const QString& type, const QString& message);
    void appendCommandMessageToChannel(const QString& channel, const QString& command, const QString& message);
    void appendStatusMessage(const QString& type,const QString& message);

    void dcopSay(const QString& target,const QString& command);
    void dcopInfo(const QString& string);
    void ctcpReply(const QString& receiver, const QString& text);

    void setChannelTopic(const QString& channel, const QString& topic);
    void setChannelTopic(const QString& nickname, const QString& channel, const QString& topic); // Overloaded
    void updateChannelMode(const QString& nick, const QString& channel, char mode, bool plus, const QString& parameter);
    void updateChannelModeWidgets(const QString& channel, char mode, const QString& parameter);
    void updateFonts();
    void setShowQuickButtons(bool state);
    void setShowModeButtons(bool state);
    void setShowTopic(bool state);

    QString getNextQueryName();
    void appendToQuery(const QString& queryName,const QString& message);
    void appendActionToQuery(const QString& queryName,const QString& message);
    void appendServerMessageToQuery(const QString& queryName,const QString& type,const QString& message);
    void appendCommandMessageToQuery(const QString& queryName,const QString& command,const QString& message);

    Channel* getChannelByName(const QString& name);
    Query* getQueryByName(const QString& name);
    QString parseWildcards(const QString& toParse, const QString& nickname, const QString& channelName, const QString &channelKey, const QStringList& nickList, const QString& parameter);
    QString parseWildcards(const QString& toParse, const QString& nickname, const QString& channelName, const QString &channelKey, const QString& nick, const QString& parameter);

    QString getAutoJoinCommand() const;

    void notifyAction(const QString& nick);
    ChannelListPanel* getChannelListPanel() const;
    
    StatusPanel* getStatusView() const { return statusView; }

    bool connected();
    QString getIp();
    QString getNumericalIp();
    
    // Given a nickname, returns NickInfo object.  0 if not found.
    NickInfo* getNickInfo(const QString& nickname);
    // Returns the list of members for a channel in the joinedChannels list.  0 if channel is not in the joinedChannels list.
    // Using code must not alter the list.
    const ChannelNickList* getJoinedChannelMembers(const QString& channelName) const;
    // Returns the list of members for a channel in the unjoinedChannels list.  0 if channel is not in the unjoinedChannels list.
    // Using code must not alter the list.
    const ChannelNickList* getUnjoinedChannelMembers(const QString& channelName) const;
    // Searches the Joined and Unjoined lists for the given channel and returns the member list.  0 if channel is not in either list.
    // Using code must not alter the list.
    const ChannelNickList* getChannelMembers(const QString& channelName) const;
    // Returns a list of all the channels (joined or unjoined) that a nick is in.
    QStringList getNickChannels(QString& nickname);
    // Returns pointer to the ChannelNick (mode and pointer to NickInfo) for a given channel and nickname.
   // 0 if not found.
    ChannelNick* getChannelNick(const QString& channelName, const QString& nickname);

  signals:
    void nicknameChanged(const QString&);
    void serverLag(Server* server,int msec); // will be connected to KonversationMainWindow::updateLag()
    void tooLongLag(Server* server, int msec); // will be connected to KonversationMainWindow::updateLag()
    void resetLag();
    void nicksNowOnline(Server* server,const QStringList& list,bool changed); // Will be emitted when new 303 came in
    void addDccPanel(); // will be connected to MainWindow::addDccPanel()
    void addKonsolePanel(); // will be connected to MainWindow::addKonsolePanel()
    void closeDccPanel(); // will be connected to MainWindow::closeDccPanel()
    void deleted(Server* myself); // will be connected to KonversationApplication::removeServer()
    void awayState(bool away); // will be connected to any user input panel;
    void multiServerCommand(const QString& command, const QString& parameter);
    void serverOnline(bool state); // will be connected to all server dependant tabs

  public slots:
    void connectToIRCServer();
    void queue(const QString &buffer);
    void queueList(const QStringList &buffer);
    void setNickname(const QString &newNickname);
    void addQuery(const QString &nickname, const QString &hostmask);
    void closeQuery(const QString &name);
    void closeChannel(const QString &name);
    void quitServer();
    void requestKonsolePanel();
    void requestDccPanel();
    void requestDccChat(const QString& nickname);
    void requestCloseDccPanel();
    void requestBan(const QStringList& users,const QString& channel,const QString& option);
    void requestUnban(const QString& mask,const QString& channel);
    void addDccSend(const QString &recipient,const QString &file);
    void removeQuery(Query *query);
    void startNotifyTimer(int msec=0);
    void sendJoinCommand(const QString& channelName);
    void requestChannelList();
    void requestUserhost(const QString& nicks);
    void addRawLog(bool show);
    void closeRawLog();
    void addChannelListPanel();
    void closeChannelListPanel();
    void updateChannelQuickButtons();
    void sendMultiServerCommand(const QString& command, const QString& parameter);
    void executeMultiServerCommand(const QString& command, const QString& parameter);

  protected slots:
    void ircServerConnectionSuccess();
    void incoming();
    void processIncomingData();
    void send();
    void broken(int state);
    void notifyTimeout();
    void notifyCheckTimeout();
    void connectionEstablished();
    void notifyResponse(const QString& nicksOnline);
    void addDccGet(const QString& sourceNick,const QStringList& dccArguments);
    void requestDccSend();                           // -> to outputFilter, dccPanel
    void requestDccSend(const QString& recipient);   // -> to outputFilter
    void resumeDccGetTransfer(const QString& sourceNick,const QStringList& dccArguments);  // -> to inputFilter
    void resumeDccSendTransfer(const QString& sourceNick,const QStringList& dccArguments); // -> to inputFilter
    void dccSendRequest(const QString& recipient,const QString& fileName,const QString& address,const QString& port,unsigned long size);
    void dccResumeGetRequest(const QString& sender,const QString& fileName,const QString& port,int startAt);
    void dccGetDone(const QString& fileName);
    void dccSendDone(const QString& fileName);
    void dccStatusChanged(const DccTransfer* item);
    void away();
    void unAway();
    void sendToAllChannels(const QString& text);
    void scriptNotFound(const QString& name);
    void scriptExecutionError(const QString& name);
    void userhost(const QString& nick,const QString& hostmask,bool away,bool ircOp);
    void setTopicAuthor(const QString& channel,const QString& author);
    void invitation(const QString& nick,const QString& channel);
    void sendToAllChannelsAndQueries(const QString& text);

  protected:
    // constants
    static const int BUFFER_LEN=513;

    KonversationMainWindow* getMainWindow() const;
    void setMainWindow(KonversationMainWindow* newMainWindow);

    bool eventFilter(QObject* parent, QEvent *event);

    void lookupFinished();
    void startNotifyCheckTimer();
    bool isAChannel(const QString &check);
    void setIdentity(Identity *newIdentity);
    
    void autoRejoinChannels();
    
    // Adds a nickname to the joinedChannels list, removing it from unjoinedChannels list, if present.
    // Creates new NickInfo if necessary.
    // If needed, moves the channel from the unjoined list to the joined list.
    // If mode != 99 sets the mode for this nick in this channel.
    // Returns the NickInfo for the nickname.
    NickInfo* addNickToJoinedChannelsList(const QString& channelName, const QString& nickname, unsigned int mode = 99);
    // Adds a nickname to the unjoinedChannels list, removing it from joinedChannels list, if present.
    // Creates new NickInfo if necessary.
    // If needed, moves the channel from the joined list to the unjoined list.
    // If mode != 99 sets the mode for this nick in this channel.
    // Returns the NickInfo for the nickname.
    NickInfo* addNickToUnjoinedChannelsList(const QString& channelName, const QString& nickname, unsigned int mode = 99);
    // Adds a nickname to the Online list, removing it from the Offline list, if present.  Returns the NickInfo of the nickname.
    // Creates new NickInfo if necessary.
    NickInfo* addNickToOnlineList(const QString& nickname);
    // Adds a nickname to the Offline list provided it is on the watch list, removing it from the Online list, if present.
    // Returns the NickInfo of the nickname or 0 if deleted altogether.
    // Creates new NickInfo if necessary.
    NickInfo* addNickToOfflineList(const QString& nickname, const QStringList& watchList);
    // Renames a nickname in all NickInfo lists.
    // Returns pointer to the NickInfo object or 0 if nick not found.
    NickInfo* renameNickInfo(const QString& nickname, const QString& newname);

    unsigned int completeQueryPosition;
    unsigned int tryNickNumber;
    unsigned int reconnectCounter;

    QString serverGroup;
    QString serverName;
    QString bot;
    QString botPassword;
    int serverPort;

    QString serverNickPrefixes;     // Prefixes used by the server to indicate a mode
    QString serverNickPrefixModes;  // if supplied: modes related to those prefixes

    IRCResolver resolver;
    Identity* identity;

    bool autoJoin;
    bool autoRejoin;
    bool autoReconnect;
    bool deliberateQuit;

    QString autoJoinChannel;
    QString autoJoinChannelKey;

    KonversationMainWindow* mainWindow;
    IRCServerSocket serverSocket;

    QTimer reconnectTimer;
    QTimer incomingTimer;
    QTimer outgoingTimer;

    int timerInterval;       // flood protection
    
    QTimer notifyTimer;
    QTimer notifyCheckTimer; // Checks if the ISON reply needs too long
    QTime notifySent;
    QStringList notifyCache; // List of users found with ISON
    int checkTime;           // Time elapsed while waiting for server 303 response
    int currentLag;

    QString ircName;
    QString inputBuffer;
    QString outputBuffer;
    QString nickname;
    QString serverKey;
    QString lastDccDir;

    QPtrList<Channel> channelList;
    QPtrList<Query> queryList;

    InputFilter inputFilter;
    OutputFilter outputFilter;

    StatusPanel* statusView;
    RawLog* rawLog;
    ChannelListPanel* channelListPanel;

    QDateTime awayTime;
    bool isAway;
    bool alreadyConnected;
    bool rejoinChannels;
    
    QString nonAwayNick;
    
    // All nicks known to this server.  Note this is NOT a list of all nicks on the server.
    NickInfoList allNicks;
    // List of membership lists for joined channels.  A "joined" channel is a channel that user has joined, i.e.,
    // a tab appears for the channel in the main window.
    ChannelMembershipList joinedChannels;
    // List of membership lists for unjoined channels.  These come from WHOIS responses.  Note that this is NOT
    // a list of all channels on the server, just those we are interested in because of nicks in the Nick Watch List.
    ChannelMembershipList unjoinedChannels;
    // List of nicks in the Nick Watch List that are online.
    NickInfoList nicknamesOnline;
    // List of nicks in the Nick Watch List that are not online.
    NickInfoList nicknamesOffline;
    // List of nicks in Queries.
    NickInfoList queryNicks;
};

#endif

/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  the text widget used for all text based panels
  begin:     Sun Jan 20 2002
  copyright: (C) 2002 by Dario Abatianni
  email:     eisfuchs@tigress.com
*/

#ifndef IRCVIEW_H
#define IRCVIEW_H

#include <qmap.h>

#include <ktextbrowser.h>

/*
  @author Dario Abatianni
*/

class QPixmap;
class QEvent;
class KPopupMenu;

class Server;

class IRCView : public KTextBrowser
{
  Q_OBJECT

  public:
    IRCView(QWidget* parent,Server* newServer);
    ~IRCView();

    void clear();
    void setViewBackground(const QString& color,const QString& pixmapName);
    void setServer(Server* server);
    
    // Returns the current nick under context menu.
    const QString& getContextNick() const;
    // Resets context nick
    void clearContextNick();

    void updateStyleSheet();

    QPopupMenu* getPopup();
    enum PopupIDs
    {
      Copy,CopyUrl,SelectAll,
      Search,
      SendFile,
      Bookmark
    };

    enum NickPopupIDs
      {
	ModesSub,GiveOp,TakeOp,GiveVoice,TakeVoice,
	KickBanSub,Ignore,
	Kick,KickBan,BanNick,BanHost,BanDomain,BanUserHost,BanUserDomain,
	KickBanHost,KickBanDomain,KickBanUserHost,KickBanUserDomain,
	Whois,Version,Ping,Query,DccSend,
	CustomID, AddressbookSub, AddressbookChange, AddressbookNew, AddressbookDelete,
	AddressbookEdit, SendEmail
      };


  signals:
    // Notify container of new text and highlight state
    void newText(const QString& highlightColor, bool important);
    void gotFocus();                  // So we can set focus to input line
    void textToLog(const QString& text);
    void sendFile();
    void extendedPopup(int id);
    void autoText(const QString& text);
    void textPasted();
    void popupCommand(int);

  public slots:
    void append(const QString& nick,const QString& message);
    void appendRaw(const QString& message, bool suppressTimestamps=false);
    void appendQuery(const QString& nick,const QString& message);
    void appendAction(const QString& nick,const QString& message);
    void appendServerMessage(const QString& type,const QString& message);
    void appendCommandMessage(const QString& command, const QString& message, bool important,
      bool parseURL = true, bool self = false);
    void appendBacklogMessage(const QString& firstColumn,const QString& message);
    void search();
    void searchAgain();

    virtual void removeSelectedText( int selNum = 0 );

    virtual void scrollToBottom(); // Overwritten for internal reasons

  protected slots:
    void highlightedSlot(const QString& link);
    void urlClickSlot(const QString &url);
    
  protected:
    QString filter(const QString& line,const QString& defaultColor,const QString& who=NULL,bool doHighlight=true, bool parseURL = true);
    void doAppend(QString line, bool important = true, bool self = false);
    void replaceDecoration(QString& line,char decoration,char replacement);

    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);
    void focusInEvent(QFocusEvent* event);

    bool eventFilter(QObject* object,QEvent* event);

    bool contextMenu(QContextMenuEvent* ce);
    void setupNickPopupMenu();

    QChar::Direction basicDirection(const QString &string);
    
    /// Returns a formated timestamp if timestamps are enabled else it returns QString::null
    QString timeStamp();
    
    // used by search function
    int findParagraph;
    int findIndex;

    // decide if we should place the scrollbar at the bottom on show()
    bool resetScrollbar;

    QString autoTextToSend;
    QString m_highlightColor;
    bool copyUrlMenu;
    QString urlToCopy;

    QString buffer;
    Server* m_server;
    QPopupMenu* popup;

    KPopupMenu* nickPopup;
    KPopupMenu* modes;
    KPopupMenu* kickban;
    
    static QChar LRM;
    static QChar RLM;
    static QChar LRE;
    static QChar RLE;
    static QChar RLO;
    static QChar LRO;
    static QChar PDF;

    bool caseSensitive;
    bool wholeWords;
    bool forward;
    bool fromCursor;
    QString pattern;
    
    uint offset;
    QStringList colorList;
    QMap<QString,QString> colorMap;
    
    QString m_currentNick;
    bool m_isOnNick;
    int popupId;
};

#endif

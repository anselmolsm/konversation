#ifndef KONV_DCOP_H
#define KONV_DCOP_H

#include <qobject.h>
#include <dcopobject.h>
#include <qptrlist.h>

#include "konviface.h"

class KonvDCOP : public QObject, virtual public KonvIface
{
  Q_OBJECT

  public:
    KonvDCOP();
    QPtrList<IRCEvent> registered_events;

    bool isIgnore (int serverid, const QString &hostmask, Ignore::Type type);
    bool isIgnore (int serverid, const QString &hostmask, int type);
    QString getNickname (int serverid);

  signals:
    void dcopSay(const QString& server,const QString& target,const QString& command);
    void dcopInfo(const QString& string);

  public slots:
    int registerEventHook(const QString& type,const QString& criteria,const QString& app,const QString& object,const QString& signal);
    void unregisterEventHook (int id);

    void raw(const QString& server,const QString& command);
    void say(const QString& server,const QString& target,const QString& command);
    void info(const QString& string);
    void debug(const QString& string);
    void error(const QString& string);

  protected:
    int hookId;
};

class KonvIdentDCOP : public QObject, virtual public KonvIdentityIface
{
  Q_OBJECT

  public:
  KonvIdentDCOP();

  void setrealName(const QString &identity, const QString& name);
  QString getrealName(const QString &identity);
  void setIdent(const QString &identity, const QString& ident);
  QString getIdent(const QString &identity);

  void setNickname(const QString &identity, int index,const QString& nick);
  QString getNickname(const QString &identity, int index);

  void setBot(const QString &identity, const QString& bot);
  QString getBot(const QString &identity);
  void setPassword(const QString &identity, const QString& password);
  QString getPassword(const QString &identity);

  void setNicknameList(const QString &identity, const QStringList& newList);
  QStringList getNicknameList(const QString &identity);

  void setPartReason(const QString &identity, const QString& reason);
  QString getPartReason(const QString &identity);
  void setKickReason(const QString &identity, const QString& reason);
  QString getKickReason(const QString &identity);

  void setShowAwayMessage(const QString &identity, bool state);
  bool getShowAwayMessage(const QString &identity);

  void setAwayMessage(const QString &identity, const QString& message);
  QString getAwayMessage(const QString &identity);
  void setReturnMessage(const QString &identity, const QString& message);
  QString getReturnMessage(const QString &identity);

};

class KonvPrefsDCOP : public QObject, virtual public KonvPreferencesIface
{
  Q_OBJECT

  public:
  KonvPrefsDCOP ();

  bool getAutoReconnect();
  void setAutoReconnect(bool state);
  bool getAutoRejoin();
  void setAutoRejoin(bool state);
  bool getBeep();
  void setBeep(bool state);
  void clearServerList();
  void changeServerProperty(int id,int property,const QString& value);
  void updateServer(int id,const QString& newDefinition);
  void setLog(bool state);
  bool getLog();
  void setLowerLog(bool state);
  bool getLowerLog();
  void setLogFollowsNick(bool state);
  bool getLogFollowsNick();
  void setLogPath(QString path);
  QString getLogPath();
  void setDccAddPartner(bool state);
  bool getDccAddPartner();
  void setDccCreateFolder(bool state);
  bool getDccCreateFolder();
  void setDccAutoGet(bool state);
  bool getDccAutoGet();
  void setDccAutoResume(bool state);
  bool getDccAutoResume();
  void setDccBufferSize(unsigned long size);
  unsigned long getDccBufferSize();
  void setDccPath(QString path);
  QString getDccPath();
  void setDccRollback(unsigned long bytes);
  unsigned long getDccRollback();
  void setBlinkingTabs(bool blink);
  bool getBlinkingTabs();
  void setBringToFront(bool state);
  bool getBringToFront();
  void setCloseButtonsOnTabs(bool state);
  bool getCloseButtonsOnTabs();
  int getNotifyDelay();
  void setNotifyDelay(int delay);
  bool getUseNotify();
  void setUseNotify(bool use);
  QStringList getNotifyList();
  QString getNotifyString();
  void setNotifyList(QStringList newList);
  bool addNotify(QString newPattern);
  bool removeNotify(QString pattern);
  void addIgnore(QString newIgnore);
  void clearIgnoreList();
  //QPtrList<Ignore> getIgnoreList();
  void setColor(QString colorName,QString color);
  void setIgnoreList(QPtrList<Ignore> newList);
  void setNickCompleteSuffixStart(QString suffix);
  void setNickCompleteSuffixMiddle(QString suffix);
  QString getNickCompleteSuffixStart();
  QString getNickCompleteSuffixMiddle();
  void setOSDUsage(bool state);
  bool getOSDUsage();
};

#endif

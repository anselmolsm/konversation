/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  serverison.h  -  Class to give a list of all the nicks known to the
                   watchednick list that are on this server.  There is
                   one instance of this class for each Server object.
  begin:     Fri Sep 03 2004
  copyright: (C) 2004 by John Tapsell
  email:     john@geola.co.uk
*/

/**
 * @author John Tapsell <john@geola.co.uk>
 * @author Gary Cramblitt <garycramblitt@comcast.net>
 */

#ifndef SERVERISON_H
#define SERVERISON_H

#include "nickinfo.h"

#include <QStringList>

class Server;

class ServerISON : public QObject
{
    Q_OBJECT

    public:
        explicit ServerISON(Server* server);
        /**
         * Returns a lower case list of nicks whose online status is being tracked
         *
         * The resulting nicks don't have the servername/servergroup attached.
         *
         * @returns              a lower case list of nicks whose online status is being tracked
         */
        QStringList getWatchList();
        /**
         * The same list as getWatchList, but with nicks in joined channels eliminated.
         * There is no point in performing an ISON on such nicks because we already
         * know they are online.  This function is called, and the result sent to the
         * server as an /ISON command.
         */
        QStringList getISONList();

    private Q_SLOTS:
        void nickInfoChanged(Server* server, const NickInfoPtr nickInfo);
        void slotServerGroupsChanged();
        void slotChannelMembersChanged(Server* server, const QString& channelName, bool joined, bool parted, const QString& nickname);
        void slotChannelJoinedOrUnjoined(Server* server, const QString& channelName, bool joined);

    private:
        /// A pointer to the server we are a member of.
        Server* m_server;
        /// List from above merged with Watch List from preferences.
        QStringList m_watchList;
        /// List from above but with nicks that are in joined channels eliminated.
        /// There is no point in doing an ISON on such nicks because we know they are
        /// online in one of the channels the user is in.
        QStringList m_ISONList;
        /// If this is true, then we need to call recalculateAddressee before returning m_ISONList
        bool m_ISONList_invalid;
        /**
         * Rebuilds list of nicks to watch whenever a preferences change occurs (whenever m_ISONLIst_invalid is true).
         */
        void recalculateAddressees();

};
#endif

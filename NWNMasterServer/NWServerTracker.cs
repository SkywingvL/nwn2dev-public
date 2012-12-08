/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    ServerTracker.cs

Abstract:

    This module houses logic to track active game servers.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;

namespace NWNMasterServer
{
    /// <summary>
    /// This class manages tracking server activity.
    /// </summary>
    internal class NWServerTracker
    {

        /// <summary>
        /// Instantiate a server tracker instance.  The list of currently
        /// known servers is retrieved from the database, but heartbeats are
        /// not yet enabled as the I/O subsystem is not yet online.
        /// </summary>
        /// <param name="MasterServer">Supplies the underlying associated
        /// master server instance.</param>
        public NWServerTracker(NWMasterServer MasterServer)
        {
            this.MasterServer = MasterServer;
        }

        /// <summary>
        /// Queue initial heartbeats to active servers
        /// </summary>
        public void QueueInitialHeartbeats()
        {
            DateTime Now = DateTime.UtcNow;

            lock (ActiveServerTable)
            {
                foreach (var Pair in ActiveServerTable)
                {
                    NWGameServer Server = Pair.Value;

                    lock (Server)
                    {
                        if (!Server.Online)
                            continue;

                        if ((Server.LastHeartbeat >= Now) ||
                            (Now - Server.LastHeartbeat) < HeartbeatCutoffTimeSpan)
                        {
                            Server.StartHeartbeat();
                        }
                        else
                        {
                            Server.Online = false;
                            Server.StopHeartbeat();
                            Server.Save();
                        }
                    }
                }
            }
        }

        /// <summary>
        /// Attempt to look up a game server by address, creating the server if
        /// requested (if it did not exist).
        /// </summary>
        /// <param name="ServerAddress">Supplies the server address to look up
        /// in the active server table.</param>
        /// <param name="Create">Supplies true if the server instance should be
        /// created if it did not exist.</param>
        /// <returns>The active server instance, else null if no server
        /// satisfied the given criteria.</returns>
        public NWGameServer LookupServerByAddress(IPEndPoint ServerAddress, bool Create = true)
        {
            NWGameServer Server;

            lock (ActiveServerTable)
            {
                if (!ActiveServerTable.TryGetValue(ServerAddress, out Server))
                    Server = null;

                if ((Create != false) && (Server == null))
                {
                    Server = new NWGameServer(MasterServer, ServerAddress);
                }
            }

            return Server;
        }

        /// <summary>
        /// The maximum lifetime, in seconds, for a server to be considered
        /// active (for purposes of sending a heartbeat ping), since the last
        /// successfully received message from the server.
        /// </summary>
        private const int SERVER_LIFETIME = 2 * 24 * 60 * 60;

        /// <summary>
        /// The heartbeat cutoff time span, derived from SERVER_LIFETIME and
        /// used for heartbeat comparisons.
        /// </summary>
        private static TimeSpan HeartbeatCutoffTimeSpan = TimeSpan.FromSeconds(SERVER_LIFETIME);

        /// <summary>
        /// The list of active game servers that have had live connectivity in
        /// the past 48 hours.
        /// </summary>
        private Dictionary<IPEndPoint, NWGameServer> ActiveServerTable = new Dictionary<IPEndPoint, NWGameServer>();

        /// <summary>
        /// Back link to the underlying master server instance.
        /// </summary>
        private NWMasterServer MasterServer;
    }
}

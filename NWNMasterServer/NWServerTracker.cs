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
using System.Threading;

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
        /// This method disables future heartbeats and flushes any servers that
        /// may be in the heartbeat path out of that code path before further
        /// forward progress is allowed.
        /// </summary>
        public void DrainHeartbeats()
        {
            //
            // Prevent new requestors from spinning up new heartbeat requests.
            //

            HeartbeatsEnabled = false;

            //
            // Flush any in-flight requestors out of the heartbeat path by
            // ensuring that they have released synchronization.
            //

            Monitor.Enter(HeartbeatLock);
            Monitor.Exit(HeartbeatLock);
        }

        /// <summary>
        /// Request a heartbeat on behalf of a server object.
        /// </summary>
        /// <param name="Server">Supplies the server object instance.</param>
        /// <returns>True if future heartbeats are enabled.</returns>
        public bool RequestHeartbeat(NWGameServer Server)
        {
            //
            // Check if heartbeats are enabled.  If so, take the heartbeat lock
            // and request the heartbeat.
            //

            if (!HeartbeatsEnabled)
                return false;

            lock (HeartbeatLock)
            {
                MasterServer.SendServerInfoRequest(Server.ServerAddress);
            }

            return true;
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
                    ActiveServerTable.Add(ServerAddress, Server);
                }
            }

            return Server;
        }

        /// <summary>
        /// The maximum lifetime, in seconds, for a server to be considered
        /// active (for purposes of sending a heartbeat ping), since the last
        /// successfully received message from the server.
        /// </summary>
        //private const int SERVER_LIFETIME = 2 * 24 * 60 * 60;
        private const int SERVER_LIFETIME = 60 * 60;

        /// <summary>
        /// The minimum amount of time between which a live heartbeat causes a
        /// server's last activity status to be refreshed in the database in
        /// the absence of any other events occuring.
        /// </summary>
        private const int HEARTBEAT_SAVE = 60;

        /// <summary>
        /// The heartbeat cutoff time span, derived from SERVER_LIFETIME and
        /// used for heartbeat comparisons.
        /// </summary>
        public static TimeSpan HeartbeatCutoffTimeSpan = TimeSpan.FromSeconds(SERVER_LIFETIME);

        /// <summary>
        /// The heartbeat save timespan, derived from HEARTBEAT_SAVE and used
        /// for autosave.
        /// </summary>
        public static TimeSpan HeartbeatSaveTimeSpan = TimeSpan.FromSeconds(HEARTBEAT_SAVE);

        /// <summary>
        /// The list of active game servers that have had live connectivity in
        /// the past 48 hours.
        /// </summary>
        private Dictionary<IPEndPoint, NWGameServer> ActiveServerTable = new Dictionary<IPEndPoint, NWGameServer>();

        /// <summary>
        /// Back link to the underlying master server instance.
        /// </summary>
        private NWMasterServer MasterServer;

        /// <summary>
        /// True if heartbears are permitted.
        /// </summary>
        private volatile bool HeartbeatsEnabled = true;

        /// <summary>
        /// The heartbeat lock.
        /// </summary>
        private object HeartbeatLock = new object();
    }
}

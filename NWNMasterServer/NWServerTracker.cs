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
using MySql.Data.MySqlClient;

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

            InitializeDatabase();
        }

        /// <summary>
        /// Queue initial heartbeats to active servers
        /// </summary>
        public void QueueInitialHeartbeats()
        {
            DateTime Now = DateTime.UtcNow;
            uint HeartbeatsStarted = 0;

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
                            HeartbeatsStarted += 1;
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

            Logger.Log(LogLevel.Normal, "NWServerTracker.QueueInitialHeartbeats(): Queued {0} initial server heartbeat requests.", HeartbeatsStarted);
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
                MasterServer.SendServerNameRequest(Server.ServerAddress);
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
        /// Prepare the database, creating tables (if they did not already
        /// exist).
        /// </summary>
        private void InitializeDatabase()
        {
            uint ServersAdded;

            if (String.IsNullOrEmpty(MasterServer.ConnectionString))
                return;

            try
            {
                MasterServer.ExecuteQueryNoReader(
@"CREATE TABLE IF NOT EXISTS `game_servers` (
    `game_server_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `product_id` int(10) UNSIGNED NOT NULL,
    `expansions_mask` int(10) UNSIGNED NOT NULL,
    `build_number` int(10) UNSIGNED NOT NULL,
    `module_name` varchar(32) NOT NULL,
    `server_name` varchar(256) NOT NULL,
    `active_player_count` int(10) UNSIGNED NOT NULL,
    `maximum_player_count` int(10) UNSIGNED NOT NULL,
    `local_vault` bool NOT NULL,
    `last_heartbeat` datetime NOT NULL,
    `server_address` varchar(128) NOT NULL,
    `online` bool NOT NULL,
    `private_server` bool NOT NULL,
    PRIMARY KEY (`game_server_id`),
    UNIQUE KEY (`product_id`, `server_address`),
    INDEX (`product_id`, `online`)
    )");

                string Query = String.Format(
@"SELECT `game_server_id`,
    `expansions_mask`,
    `build_number`,
    `module_name`,
    `server_name`,
    `active_player_count`,
    `maximum_player_count`,
    `local_vault`,
    `last_heartbeat`,
    `server_address`,
    `online`,
    `private_server`
FROM `game_servers`
WHERE `product_id` = {0}
AND `online` = true
AND `last_heartbeat` >= '{1}'",
                    MasterServer.ProductID,
                    MasterServer.DateToSQLDate(DateTime.UtcNow.Subtract(HeartbeatCutoffTimeSpan)));

                ServersAdded = 0;
                using (MySqlDataReader Reader = MasterServer.ExecuteQuery(Query))
                {
                    while (Reader.Read())
                    {
                        uint ServerId = Reader.GetUInt32(0);
                        string Hostname = Reader.GetString(9);
                        int i = Hostname.IndexOf(':');
                        IPEndPoint ServerAddress;

                        if (i == -1)
                        {
                            Logger.Log(LogLevel.Error, "NWServerTracker.InitializeDatabase(): Server {0} has invalid hostname '{1}'.",
                                ServerId,
                                Hostname);
                            continue;
                        }

                        try
                        {
                            ServerAddress = new IPEndPoint(
                                IPAddress.Parse(Hostname.Substring(0, i)),
                                Convert.ToInt32(Hostname.Substring(i + 1)));
                        }
                        catch (Exception e)
                        {
                            Logger.Log(LogLevel.Error, "NWServerTracker.InitializeDatabase(): Error initializing hostname {0} for server {1}: Exception: {2}",
                                Hostname,
                                ServerId,
                                e);
                            continue;
                        }

                        NWGameServer Server = new NWGameServer(MasterServer, ServerAddress);

                        Server.DatabaseId = ServerId;
                        Server.ExpansionsMask = (Byte)Reader.GetUInt32(1);
                        Server.BuildNumber = (UInt16)Reader.GetUInt32(2);
                        Server.ModuleName = Reader.GetString(3);
                        Server.ServerName = Reader.GetString(4);
                        Server.ActivePlayerCount = Reader.GetUInt32(5);
                        Server.MaximumPlayerCount = Reader.GetUInt32(6);
                        Server.LocalVault = Reader.GetBoolean(7);
                        Server.LastHeartbeat = Reader.GetDateTime(8);
                        Server.Online = Reader.GetBoolean(10);
                        Server.PrivateServer = Reader.GetBoolean(11);

                        lock (ActiveServerTable)
                        {
                            ActiveServerTable.Add(ServerAddress, Server);
                        }

                        ServersAdded += 1;
                    }
                }

                Logger.Log(LogLevel.Normal, "NWServerTracker.InitializeDatabase(): Read {0} active servers from database.", ServersAdded);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWServerTracker.InitializeDatabase(): Exception: {0}", e);
                MasterServer.Stop();
            }
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

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

            PendingGameServersSweepTimer = new System.Timers.Timer(PENDING_GAME_SERVER_SWEEP_INTERVAL);
            PendingGameServersSweepTimer.AutoReset = false;
            PendingGameServersSweepTimer.Elapsed += new System.Timers.ElapsedEventHandler(PendingGameServersSweepTimer_Elapsed);

            ScavengerSweepTimer = new System.Timers.Timer(SCAVENGE_SWEEP_INTERVAL);
            ScavengerSweepTimer.AutoReset = false;
            ScavengerSweepTimer.Elapsed += new System.Timers.ElapsedEventHandler(ScavengerSweepTimer_Elapsed);

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

            PendingGameServersSweepTimer.Start();
            ScavengerSweepTimer.Start();
        }

        /// <summary>
        /// This method disables future heartbeats and flushes any servers that
        /// may be in the heartbeat path out of that code path before further
        /// forward progress is allowed.
        /// </summary>
        public void DrainHeartbeats()
        {
            PendingGameServersSweepTimer.Stop();
            ScavengerSweepTimer.Stop();

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
                MasterServer.RefreshServerStatus(Server.ServerAddress);
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
                    Server.Load();
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
    `module_description` varchar(256) NOT NULL,
    `module_url` varchar(256) NOT NULL,
    `game_type` int(10) UNSIGNED NOT NULL,
    `minimum_level` int(10) UNSIGNED NOT NULL,
    `maximum_level` int(10) UNSIGNED NOT NULL,
    `pvp_level` int(10) UNSIGNED NOT NULL,
    `player_pause` bool NOT NULL,
    `one_party_only` bool NOT NULL,
    `elc_enforced` bool NOT NULL,
    `ilr_enforced` bool NOT NULL,
    `pwc_url` varchar(256) NOT NULL,
    `server_description` varchar(256) NOT NULL,
    PRIMARY KEY (`game_server_id`),
    UNIQUE KEY (`product_id`, `server_address`),
    INDEX (`product_id`, `online`),
    INDEX (`product_id`, `online`, `server_name`),
    INDEX (`product_id`, `online`, `module_name`),
    INDEX (`product_id`, `online`, `game_type`)
    )");

                MasterServer.ExecuteQueryNoReader(
@"CREATE TABLE IF NOT EXISTS `pending_game_servers` (
    `pending_game_server_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
    `product_id` int(10) UNSIGNED NOT NULL,
    `server_address` varchar(128) NOT NULL,
    PRIMARY KEY (`pending_game_server_id`),
    UNIQUE KEY (`product_id`, `server_address`)
    )");

                MasterServer.ExecuteQueryNoReader(
@"CREATE TABLE IF NOT EXISTS `client_extension_update` (
    `update_id` int(10) UNSIGNED NOT NULL,
    `product_id` int(10) UNSIGNED NOT NULL,
    `update_message` varchar(4096) NOT NULL,
    `update_url` varchar(4096) NOT NULL,
    `update_info_url` varchar(4096) NOT NULL,
    `update_version` varchar(128) NOT NULL,
    `update_motd` varchar(4096) NOT NULL,
    PRIMARY KEY (`update_id`)
    )");

                MasterServer.ExecuteQueryNoReader(
@"CREATE TABLE IF NOT EXISTS `stat_counters` (
    `stat_counter_name` varchar(64) NOT NULL,
    `stat_counter_value` int(10) UNSIGNED NOT NULL,
    `stat_counter_last_update` datetime NOT NULL,
    PRIMARY KEY (`stat_counter_name`)
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
    `private_server`,
    `module_description`,
    `module_url`,
    `game_type`,
    `minimum_level`,
    `maximum_level`,
    `pvp_level`,
    `player_pause`,
    `one_party_only`,
    `elc_enforced`,
    `ilr_enforced`,
    `pwc_url`, 
    `server_description` 
FROM `game_servers`
WHERE `product_id` = {0}
AND `online` = true",
                    MasterServer.ProductID);

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
                        Server.ModuleDescription = Reader.GetString(12);
                        Server.ModuleUrl = Reader.GetString(13);
                        Server.GameType = Reader.GetUInt32(14);
                        Server.MinimumLevel = Reader.GetUInt32(15);
                        Server.MaximumLevel = Reader.GetUInt32(16);
                        Server.PVPLevel = Reader.GetUInt32(17);
                        Server.PlayerPause = Reader.GetBoolean(18);
                        Server.OnePartyOnly = Reader.GetBoolean(19);
                        Server.ELCEnforced = Reader.GetBoolean(20);
                        Server.ILREnforced = Reader.GetBoolean(21);
                        Server.PWCUrl = Reader.GetString(22);
                        Server.ServerDescription = Reader.GetString(23);

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
        /// This timer callback runs when the pending game servers sweep timer
        /// elapses.  It checks whether there is a pending query queue, and if
        /// so, flushes it as appropriate.
        /// </summary>
        /// <param name="sender">Unused.</param>
        /// <param name="e">Unused.</param>
        private void PendingGameServersSweepTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            try
            {
                uint HighestPendingServerId = 0;
                bool DataReturned = false;
                string Query = String.Format(
@"SELECT `pending_game_server_id`, 
    `server_address` 
FROM 
    `pending_game_servers` 
WHERE 
    `product_id` = {0} 
GROUP BY `pending_game_server_id` 
ORDER BY `pending_game_server_id` 
LIMIT 100 ",
                       MasterServer.ProductID);

                using (MySqlDataReader Reader = MasterServer.ExecuteQuery(Query))
                {
                    while (Reader.Read())
                    {
                        uint PendingServerId = Reader.GetUInt32(0);
                        string Hostname = Reader.GetString(1);
                        IPEndPoint ServerAddress;

                        if (PendingServerId > HighestPendingServerId)
                            HighestPendingServerId = PendingServerId;

                        if (DataReturned == false)
                            DataReturned = true;

                        ServerAddress = ConvertServerHostnameToIPEndPoint(Hostname);
                        if (ServerAddress == null)
                        {
                            Logger.Log(LogLevel.Error, "NWServerTracker.PendingGameServersSweepTimer_Elapsed(): Server {0} has invalid hostname '{1}'.",
                                PendingServerId,
                                Hostname);
                            continue;
                        }

                        NWGameServer Server = LookupServerByAddress(ServerAddress, false);

                        if (Server == null || Server.Online == false)
                            MasterServer.SendServerInfoRequest(ServerAddress);
                    }
                }

                if (DataReturned)
                {
                    //
                    // Remove records that have already been processed.
                    //

                    MasterServer.ExecuteQueryNoReader(String.Format(
@"DELETE FROM 
    `pending_game_servers` 
WHERE 
    `product_id` = {0} 
AND 
    `pending_game_server_id` < {1}",
                                   MasterServer.ProductID,
                                   HighestPendingServerId + 1));
                }

            }
            catch (Exception ex)
            {
                Logger.Log(LogLevel.Error, "NWServerTracker.PendingGameServersSweepTimer_Elapsed(): Exception processing pending servers list: {0}.", ex);
            }

            if (!HeartbeatsEnabled)
                return;

            lock (HeartbeatLock)
            {
                PendingGameServersSweepTimer.Start();
            }
        }


        /// <summary>
        /// This timer callback runs when the scavenge sweep timer elapses.
        /// Its purpose is to cycle through offline game_servers records in the
        /// database, which have had a heartbeat more recent than
        /// ScavengerTimeSpan since the current time.  Such servers are then
        /// re-pinged in a low frequency interval so as to allow servers that
        /// had gone offline for a long period of time (but had been returned
        /// to service afterwards) to be automatically re-listed eventually.
        /// </summary>
        /// <param name="sender">Unused.</param>
        /// <param name="e">Unused.</param>
        private void ScavengerSweepTimer_Elapsed(object sender, System.Timers.ElapsedEventArgs e)
        {
            try
            {
                bool Processed = false;
                DateTime Now = DateTime.UtcNow;
                string Query = String.Format(
@"SELECT `game_server_id`,
    `server_address` 
FROM `game_servers` 
WHERE `product_id` = {0} 
AND `game_server_id` > {1}
AND `online` = false 
AND `last_heartbeat` >= '{2}' 
GROUP BY `game_server_id` 
ORDER BY `game_server_id` 
LIMIT 50",
                    MasterServer.ProductID,
                    ScavengeServerId,
                    MasterServer.DateToSQLDate(Now.Subtract(ScavengerTimeSpan)));

                //
                // Examine each server record returned and, if the server is
                // still offline, enqueue a ping to it.  Advance the iterator
                // to the next server as each server is processed.
                //

                using (MySqlDataReader Reader = MasterServer.ExecuteQuery(Query))
                {
                    while (Reader.Read())
                    {
                        uint ServerId = Reader.GetUInt32(0);
                        IPEndPoint ServerAddress = ConvertServerHostnameToIPEndPoint(Reader.GetString(1));

                        if (ServerId > ScavengeServerId)
                            ScavengeServerId = ServerId;

                        Processed = true;

                        if (ServerAddress == null)
                            continue;

                        NWGameServer Server = LookupServerByAddress(ServerAddress, false);

                        if (Server == null || Server.Online == false)
                            MasterServer.SendServerInfoRequest(ServerAddress);
                    }
                }

                //
                // If no servers were processed, then the end of the iterator
                // list must have been reached.  Re-start the sweep cycle at
                // the beginning next time.
                //

                if (Processed == false)
                    ScavengeServerId = 0;
            }
            catch (Exception ex)
            {
                Logger.Log(LogLevel.Error, "NWServerTracker.ScavengeSweepTimer_Elapsed(): Excepton processing scavenge server list: {0}", ex);
            }

            if (!HeartbeatsEnabled)
                return;

            lock (HeartbeatLock)
            {
                ScavengerSweepTimer.Start();
            }
        }

        /// <summary>
        /// This method converts a dotted-quad:port string into an IPEndPoint.
        /// </summary>
        /// <param name="Hostname">Supplies the dotted-quad:port string.
        /// </param>
        /// <returns>The converted IPEndPoint is returned.  If the address was
        /// malformed, null is returned.</returns>
        private IPEndPoint ConvertServerHostnameToIPEndPoint(string Hostname)
        {
            int i = Hostname.IndexOf(':');

            if (i == -1)
                return null;

            try
            {
                return new IPEndPoint(
                    IPAddress.Parse(Hostname.Substring(0, i)),
                    Convert.ToInt32(Hostname.Substring(i + 1)));
            }
            catch
            {
                return null;
            }
        }


        /// <summary>
        /// The maximum lifetime, in seconds, for a server to be considered
        /// active (for purposes of sending a heartbeat ping), since the last
        /// successfully received message from the server.
        /// </summary>
        //private const int SERVER_LIFETIME = 2 * 24 * 60 * 60;
        //private const int SERVER_LIFETIME = 4 * 60 * 60;
        private const int SERVER_LIFETIME = 5 * 60;

        /// <summary>
        /// The minimum amount of time between which a live heartbeat causes a
        /// server's last activity status to be refreshed in the database in
        /// the absence of any other events occuring.
        /// </summary>
        private const int HEARTBEAT_SAVE = 60;

        /// <summary>
        /// The interval, in milliseconds, after which the pending_game_servers
        /// table is scanned for servers to enqueue.
        /// </summary>
        private const int PENDING_GAME_SERVER_SWEEP_INTERVAL = 5 * 60 * 1000;

        /// <summary>
        /// The interval, in milliseconds, after which the game_servers table
        /// is next iterated through for a selection of servers to re-ping for
        /// the background scavenger.
        /// </summary>
        private const int SCAVENGE_SWEEP_INTERVAL = 1 * 60 * 60 * 1000;

        /// <summary>
        /// The number of days that a server will be periodically re-pinged by
        /// the background server scavenger since its last successful heartbeat
        /// return.
        /// </summary>
        private const int SCAVENGER_SPAN_DAYS = 7;

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
        /// The time span from now that servers are considered eligible for
        /// automatic scavenging from the offline list, since their last active
        /// heartbeat report.
        /// </summary>
        public static TimeSpan ScavengerTimeSpan = TimeSpan.FromDays(7);

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

        /// <summary>
        /// The timer used to trigger periodic scans of the
        /// pending_game_servers table for pulling in new server records from
        /// the web service API.
        /// </summary>
        private System.Timers.Timer PendingGameServersSweepTimer = null;

        /// <summary>
        /// The timer used to trigger periodic scavenger scans of the
        /// game_servers table, used to repopulate servers that have since gone
        /// offline longer than the heartbeat interval, but shorter than the
        /// scavenge time span.
        /// </summary>
        private System.Timers.Timer ScavengerSweepTimer = null;

        /// <summary>
        /// The server id of the last sever examined for scavenging.  If zero,
        /// then the scavenge cycle restarts next timer elapse.
        /// </summary>
        private uint ScavengeServerId = 0;
    }
}

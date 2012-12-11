/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NWGameServer.cs

Abstract:

    This module houses data associated with a game server.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Timers;
using MySql.Data.MySqlClient;

namespace NWNMasterServer
{
    /// <summary>
    /// This class records information associated with a single game server
    /// instance.
    /// </summary>
    internal class NWGameServer
    {
        public NWGameServer(NWMasterServer MasterServer, IPEndPoint ServerAddress)
        {
            Timer HbTimer;

            this.MasterServer = MasterServer;
            this.Address = ServerAddress;

            HbTimer = new Timer(HEARTBEAT_INTERVAL + (Rng.Next() % HEARTBEAT_JITTER));

            HbTimer.AutoReset = false;
            HbTimer.Elapsed += new ElapsedEventHandler(HeartbeatTimer_Elapsed);

            this.HeartbeatTimer = HbTimer;
        }

        /// <summary>
        /// Save the server to the database.  The server is assumed to be
        /// locked.
        /// </summary>
        public void Save()
        {
            if (MasterServer.ConnectionString == null)
                return;

            try
            {
                if (ModuleName == null)
                    ModuleName = "";

                if (ServerName == null)
                    ServerName = "";

                string Query = String.Format(
@"INSERT INTO `game_servers` (
    `game_server_id`,
    `product_id`,
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
    `private_server`)
VALUES (
    {0},
    {1},
    {2},
    {3},
    '{4}',
    '{5}',
    {6},
    {7},
    {8},
    '{9}',
    '{10}',
    {11},
    {12})
ON DUPLICATE KEY UPDATE 
    `expansions_mask` = {2},
    `build_number` = {3},
    `module_name` = '{4}',
    `server_name` = '{5}',
    `active_player_count` = {6},
    `maximum_player_count` = {7},
    `local_vault` = {8},
    `last_heartbeat` = '{9}',
    `server_address` = '{10}',
    `online` = {11},
    `private_server` = {12}",
                DatabaseId,
                MasterServer.ProductID,
                ExpansionsMask,
                BuildNumber,
                MySqlHelper.EscapeString(ModuleName.Length > 32 ? ModuleName.Substring(0, 32) : ModuleName),
                MySqlHelper.EscapeString(ServerName.Length > 256 ? ServerName.Substring(0, 256) : ServerName),
                ActivePlayerCount,
                MaximumPlayerCount,
                LocalVault,
                MasterServer.DateToSQLDate(LastHeartbeat),
                MySqlHelper.EscapeString(ServerAddress.ToString()),
                Online,
                PrivateServer
                );

                MasterServer.ExecuteQueryNoReaderCombine(Query);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWGameServer.Save(): Failed to save server {0}: Exception: {1}",
                    ServerAddress,
                    e);
            }
        }

        /// <summary>
        /// Attempt to load data from the database.  The server is assumed to
        /// be either locked or not yet published.
        /// </summary>
        public void Load()
        {
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
AND `server_address` = '{1}'",
                MasterServer.ProductID,
                MySqlHelper.EscapeString(ServerAddress.ToString()));

            try
            {
                using (MySqlDataReader Reader = MasterServer.ExecuteQuery(Query))
                {
                    if (!Reader.Read())
                        return;

                    DatabaseId = Reader.GetUInt32(0);

                    NWGameServer Server = new NWGameServer(MasterServer, ServerAddress);

                    ExpansionsMask = (Byte)Reader.GetUInt32(1);
                    BuildNumber = (UInt16)Reader.GetUInt32(2);
                    ModuleName = Reader.GetString(3);
                    ServerName = Reader.GetString(4);
                    MaximumPlayerCount = Reader.GetUInt32(6);
                    LocalVault = Reader.GetBoolean(7);
                    PrivateServer = Reader.GetBoolean(11);
                }
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.Load(): Failed to load server {0}: Exception: {1}", ServerAddress, e);
            }
        }


        /// <summary>
        /// Start the periodic heartbeat, if it is not already running.  The
        /// server is assumed to be locked.
        /// </summary>
        public void StartHeartbeat()
        {
            HeartbeatTimer.Start();
        }

        /// <summary>
        /// Stop the periodic heartbeat, if it is running.  The server is
        /// assumed to be locked.
        /// </summary>
        public void StopHeartbeat()
        {
            HeartbeatTimer.Stop();
        }

        /// <summary>
        /// Get a textural representation of the server.
        /// </summary>
        /// <returns>The server address.</returns>
        public override string ToString()
        {
            return ServerAddress.ToString();
        }

        /// <summary>
        /// Record activity for the server.  The server is assumed to be
        /// unlocked.
        /// </summary>
        /// <param name="ResetPlayerCount">Supplies true if the player count is
        /// to be reset if the server is transitioning to an online state, else
        /// false if the existing player count is assumed to be up to date,
        /// e.g. because the caller already updated it.</param>
        public void RecordActivity()
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                //
                // If it has been an extended duration since the last save event,
                // then save the server to the database so that it is persisted as
                // being online (even if idle with respect to players coming or
                // going).
                //

                if (Online)
                {
                    Timesave(Now);
                }
                else
                {
                    Online = true;
                    LastHeartbeat = Now;
                    StartHeartbeat();
                    Save();
                }
            }
        }

        /// <summary>
        /// Called when a heartbeat is requested (server not locked).
        /// </summary>
        /// <param name="PlayerCount">Supplies the active player count.</param>
        public void OnHeartbeat(uint PlayerCount)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                //
                // If the server is currently offline, transition it to the
                // online state and save.
                //
                // If the server is already online and the player count has
                // changed, then update the player count and save.
                //
                // Otherwise, refresh the heartbeat time and perform a periodic
                // save of the last heartbeat timer (if necessary).
                //

                if (!Online)
                {
                    Online = true;
                    ActivePlayerCount = PlayerCount;
                    LastHeartbeat = Now;
                    StartHeartbeat();
                    Save();
                }
                else if (ActivePlayerCount != PlayerCount)
                {
                    ActivePlayerCount = PlayerCount;
                    LastHeartbeat = Now;
                    Save();
                }
                else
                {
                    Timesave(Now);
                }
            }
        }

        /// <summary>
        /// Called when a shutdown notification for a server is received.
        /// </summary>
        public void OnShutdownNotify()
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                LastHeartbeat = Now;

                if (!Online)
                    return;

                ActivePlayerCount = 0;
                Online = false;
                StopHeartbeat();
                Save();
            }
        }

        /// <summary>
        /// Called when a startup notification for a server is received.
        /// </summary>
        /// <param name="Platform">Supplies the platform code.</param>
        /// <param name="BuildNumber">Supplies the build number.</param>
        public void OnStartupNotify(Byte Platform, UInt16 BuildNumber)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                if ((this.Platform != Platform) ||
                    (this.BuildNumber != BuildNumber) ||
                    (Online == false))
                {
                    LastHeartbeat = Now;
                    this.Platform = Platform;
                    this.BuildNumber = BuildNumber;
                    ActivePlayerCount = 0;

                    if (!Online)
                    {
                        Online = true;
                        StartHeartbeat();
                    }

                    Save();
                }
                else
                {
                    Timesave(Now);
                }
            }
        }

        /// <summary>
        /// Called when a module load notification for a server is received.
        /// </summary>
        /// <param name="ExpansionsMask">Supplies the expansion
        /// bitmask.</param>
        /// <param name="ModuleName">Supplies the module name.</param>
        public void OnModuleLoad(Byte ExpansionsMask, string ModuleName)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                if ((this.ExpansionsMask != ExpansionsMask) ||
                    (this.ModuleName != ModuleName) ||
                    (Online == false))
                {
                    LastHeartbeat = Now;
                    this.ExpansionsMask = ExpansionsMask;
                    this.ModuleName = ModuleName;

                    if (!Online)
                    {
                        Online = true;
                        StartHeartbeat();
                    }

                    Save();
                }
                else
                {
                    Timesave(Now);
                }
            }
        }

        /// <summary>
        /// Called when a server info update is available.
        /// </summary>
        /// <param name="Info">Supplies the new server information
        /// structure.</param>
        public void OnServerInfoUpdate(NWMasterServer.ServerInfo Info)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                if ((this.ExpansionsMask != Info.ExpansionsMask) ||
                    (this.MaximumPlayerCount != Info.MaximumPlayers) ||
                    (this.ActivePlayerCount != Info.ActivePlayers) ||
                    (this.LocalVault != Info.IsLocalVault) ||
                    (this.BuildNumber != Info.BuildNumber) ||
                    (this.PrivateServer != Info.HasPlayerPassword) ||
                    (this.ModuleName != Info.ModuleName))
                {
                    LastHeartbeat = Now;
                    ExpansionsMask = Info.ExpansionsMask;
                    MaximumPlayerCount = Info.MaximumPlayers;
                    ActivePlayerCount = Info.ActivePlayers;
                    LocalVault = Info.IsLocalVault;
                    this.PrivateServer = Info.HasPlayerPassword;
                    BuildNumber = Info.BuildNumber;
                    ModuleName = Info.ModuleName;

                    if (!Online)
                    {
                        Online = true;
                        StartHeartbeat();
                    }

                    Save();
                }
                else
                {
                    Timesave(Now);
                }
            }
        }

        /// <summary>
        /// Called when a server name update is available.
        /// </summary>
        /// <param name="ServerName">Supplies the new server name.</param>
        public void OnServerNameUpdate(string ServerName)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                if (this.ServerName != ServerName)
                {
                    LastHeartbeat = Now;
                    this.ServerName = ServerName;

                    if (!Online)
                    {
                        Online = true;
                        StartHeartbeat();
                    }

                    Save();
                }
                else
                {
                    Timesave(Now);
                }
            }
        }

        /// <summary>
        /// Initiate an auto save if necessary.  The server is assumed to be
        /// locked.
        /// </summary>
        /// <param name="Now">Supplies the time stamp to save at.</param>
        private void Timesave(DateTime Now)
        {
            if ((Now >= LastHeartbeat) &&
                (Now - LastHeartbeat) >= NWServerTracker.HeartbeatCutoffTimeSpan)
            {
                LastHeartbeat = Now;
                Save();
            }
            else
            {
                LastHeartbeat = Now;
            }
        }

        /// <summary>
        /// Handle a heartbeat timer elapse event, by requesting a heartbeat.
        /// </summary>
        /// <param name="sender">Unused.</param>
        /// <param name="e">Unused.</param>
        private void HeartbeatTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            DateTime Now = DateTime.UtcNow;
            NWServerTracker Tracker = MasterServer.Tracker;
            bool Expired = false;

            //
            // First, check for server activity expiration.  If the server has
            // exceeded the heartbeat cut off, mark it as offline.
            //

            lock (this)
            {
                if ((Now >= LastHeartbeat) &&
                    (Now - LastHeartbeat) >= NWServerTracker.HeartbeatCutoffTimeSpan)
                {
                    ActivePlayerCount = 0;
                    Online = false;
                    Expired = true;
                }
            }

            if (Expired)
            {
                Logger.Log(LogLevel.Normal, "NWGameServer.HeartbeatTimer_Elapsed(): Server {0} expired from online server list due to heartbeat timeout.", this);
                Save();
                return;
            }

            //
            // Request the tracker to initiate this heartbeat request.  If the
            // server is not shutting down, continue on to queue the next
            // heartbeat expiration timer.
            //

            if (!MasterServer.Tracker.RequestHeartbeat(this))
                return;

            HeartbeatTimer.Interval = (HEARTBEAT_INTERVAL + (Rng.Next() % HEARTBEAT_JITTER));
            HeartbeatTimer.Start();
        }

        /// <summary>
        /// Platform code ('W' - Windows, 'L' - Linux).
        /// </summary>
        public Byte Platform { get; set; }

        /// <summary>
        /// Game build number.
        /// </summary>
        public UInt16 BuildNumber { get; set; }

        /// <summary>
        /// The claimed, internal data port of the server.
        /// </summary>
        public UInt16 InternalDataPort { get; set; }

        /// <summary>
        /// Bitmask of expansion sets required to connect to the server.
        /// 0x1 - XP1 is required.
        /// 0x2 - XP2 is required.
        /// </summary>
        public Byte ExpansionsMask { get; set; }

        /// <summary>
        /// The name of the module that the server has loaded (if any).
        /// </summary>
        public string ModuleName { get; set; }

        /// <summary>
        /// The name of the server (if any).
        /// </summary>
        public string ServerName { get; set; }

        /// <summary>
        /// The count of active players on the server.
        /// </summary>
        public uint ActivePlayerCount { get; set; }

        /// <summary>
        /// The maximum count of players that are allowed on the server.
        /// </summary>
        public uint MaximumPlayerCount { get; set; }

        /// <summary>
        /// True if the server is local vault, false for server vault.
        /// </summary>
        public bool LocalVault { get; set; }

        /// <summary>
        /// The last datestamp that the server was observed active.
        /// </summary>
        public DateTime LastHeartbeat { get; set; }

        /// <summary>
        /// Retrieve the network address of the server.
        /// </summary>
        public IPEndPoint ServerAddress { get { return Address; } }

        /// <summary>
        /// True if the server is online.
        /// </summary>
        public bool Online { get; set; }

        /// <summary>
        /// True if the server has a player password.
        /// </summary>
        public bool PrivateServer { get; set; }

        /// <summary>
        /// The internal database ID of the server, or zero if the server has
        /// not had an ID assigned yet.
        /// </summary>
        public uint DatabaseId { get; set; }


        /// <summary>
        /// The interval, in milliseconds, between server heartbeats.
        /// </summary>
        private const int HEARTBEAT_INTERVAL = 15000;

        /// <summary>
        /// The amount of random jitter added to each server's heartbeat
        /// interval, in milliseconds.  This serves as a load spreading
        /// function to prevent heartbeats from immediately stacking up at the
        /// same time during server startup, when the server table is read from
        /// the database.
        /// </summary>
        private const int HEARTBEAT_JITTER = 30000;


        /// <summary>
        /// The address of the server.
        /// </summary>
        private IPEndPoint Address;

        /// <summary>
        /// Back link to the underlying master server instance.
        /// </summary>
        private NWMasterServer MasterServer;

        /// <summary>
        /// The heartbeat timer for the server.
        /// </summary>
        private Timer HeartbeatTimer = null;

        /// <summary>
        /// Random number generator.
        /// </summary>
        private static Random Rng = new Random();
    }
}

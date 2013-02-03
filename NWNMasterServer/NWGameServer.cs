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
    internal sealed class NWGameServer
    {
        public NWGameServer(NWMasterServer MasterServer, IPEndPoint ServerAddress)
        {
            Timer HbTimer;

            this.MasterServer = MasterServer;
            this.Address = ServerAddress;
            this.InitialHeartbeat = false;

            HbTimer = new Timer(HEARTBEAT_INTERVAL + (Rng.Next() % HEARTBEAT_JITTER));

            HbTimer.AutoReset = false;
            HbTimer.Elapsed += new ElapsedEventHandler(HeartbeatTimer_Elapsed);

            this.HeartbeatTimer = HbTimer;
        }

        /// <summary>
        /// Save the server to the database.  The server is assumed to be
        /// locked.
        /// </summary>
        /// <returns>True if the save was successful.</returns>
        public bool Save()
        {
            if (MasterServer.ConnectionString == null)
                return false;

            try
            {
                if (ModuleName == null)
                    ModuleName = "";

                if (ServerName == null)
                    ServerName = "";

                if (ModuleDescription == null)
                    ModuleDescription = "";

                if (ModuleUrl == null)
                    ModuleUrl = "";

                if (PWCUrl == null)
                    PWCUrl = "";

                if (ServerDescription == null)
                    ServerDescription = "";

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
    `server_description`)
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
    {12},
    '{13}',
    '{14}',
    {15},
    {16},
    {17},
    {18},
    {19},
    {20},
    {21},
    {22},
    '{23}',
    '{24}')
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
    `private_server` = {12},
    `module_description` = '{13}',
    `module_url` = '{14}',
    `game_type` = {15},
    `minimum_level` = {16},
    `maximum_level` = {17},
    `pvp_level` = {18},
    `player_pause` = {19},
    `one_party_only` = {20},
    `elc_enforced` = {21},
    `ilr_enforced` = {22},
    `pwc_url` = '{23}',
    `server_description` = '{24}'",
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
                PrivateServer,
                MySqlHelper.EscapeString(ModuleDescription.Length > 256 ? ModuleDescription.Substring(0, 256) : ModuleDescription),
                MySqlHelper.EscapeString(ModuleUrl.Length > 256 ? ModuleUrl.Substring(0, 256) : ModuleUrl),
                GameType,
                MinimumLevel,
                MaximumLevel,
                PVPLevel,
                PlayerPause,
                OnePartyOnly,
                ELCEnforced,
                ILREnforced,
                MySqlHelper.EscapeString(PWCUrl.Length > 256 ? PWCUrl.Substring(0, 256) : PWCUrl),
                MySqlHelper.EscapeString(ServerDescription.Length > 256 ? ServerDescription.Substring(0, 256) : ServerDescription)
                );

                MasterServer.ExecuteQueryNoReaderCombine(Query);

                LastSaveTick = (uint)Environment.TickCount;
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWGameServer.Save(): Failed to save server {0}: Exception: {1}",
                    ServerAddress,
                    e);
                return false;
            }

            return true;
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
                    ModuleDescription = Reader.GetString(12);
                    ModuleUrl = Reader.GetString(13);
                    GameType = Reader.GetUInt32(14);
                    MinimumLevel = Reader.GetUInt32(15);
                    MaximumLevel = Reader.GetUInt32(16);
                    PVPLevel = Reader.GetUInt32(17);
                    Server.PlayerPause = Reader.GetBoolean(18);
                    Server.OnePartyOnly = Reader.GetBoolean(19);
                    Server.ELCEnforced = Reader.GetBoolean(20);
                    Server.ILREnforced = Reader.GetBoolean(21);
                    PWCUrl = Reader.GetString(22);
                    ServerDescription = Reader.GetString(23);

                    if (Online)
                        StartHeartbeat();
                }
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.Load(): Failed to load server {0}: Exception: {1}", ServerAddress, e);
            }
        }

        /// <summary>
        /// This method attempts to determine whether a different server, homed
        /// at the same address, is a "NAT duplicate" of the current server.
        ///
        /// Some broken NAT devices will pick an ephemeral source port for the
        /// master heartbeat ping, and will (for a short time) even respond on
        /// this port, even though they also respond on the declared server
        /// internal port, which is the canonical address to select.
        ///
        /// However, it is legitimate for two different servers to reside at
        /// different ports on the same IP address.  In order to distinguish
        /// between these conditions, a test is made as to whether the data set
        /// returned by a BNXR message from both servers is identical, for the
        /// configuration settings.  If so, then a NAT duplicate is declared,
        /// otherwise both servers are considered to be legitimately distinct.
        /// 
        /// In the condition where a NAT duplicate is detected, the internal
        /// advertised address of the server is assumed to be the canonical
        /// address.  In such a case, the InternalServer argument corresponds
        /// to the canonical server, and the this pointer corresponds to the
        /// NAT duplicate.  The NAT duplicate is marked offline and its
        /// heartbeat is then descheduled.
        /// </summary>
        /// <param name="InternalServer">Supplies the potential canonical half
        /// of a NAT duplicate server, to be compared against.</param>
        /// <returns>True if the current server was marked offline as a NAT
        /// duplicate.</returns>
        public bool CheckForNATDuplicate(NWGameServer InternalServer)
        {
            lock (this)
            {
                lock (InternalServer)
                {
                    if ((ModuleName == InternalServer.ModuleName) &&
                        (PrivateServer == InternalServer.PrivateServer) &&
                        (MinimumLevel == InternalServer.MinimumLevel) &&
                        (MaximumLevel == InternalServer.MaximumLevel) &&
                        (MaximumPlayerCount == InternalServer.MaximumPlayerCount) &&
                        (LocalVault == InternalServer.LocalVault) &&
                        (PVPLevel == InternalServer.PVPLevel) &&
                        (PlayerPause == InternalServer.PlayerPause) &&
                        (OnePartyOnly == InternalServer.OnePartyOnly) &&
                        (ELCEnforced == InternalServer.ELCEnforced) &&
                        (ILREnforced == InternalServer.ILREnforced) &&
                        (ExpansionsMask == InternalServer.ExpansionsMask))
                    {
                        NATDuplicateTick = (uint)Environment.TickCount;

                        if (Online)
                        {
                            Online = false;
                            StopHeartbeat();
                        }

                        Save();

                        return true;
                    }
                }
            }

            return false;
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
                LastHeartbeat = Now;

                //
                // If it has been an extended duration since the last save event,
                // then save the server to the database so that it is persisted as
                // being online (even if idle with respect to players coming or
                // going).
                //

                if (Online)
                {
                    Timesave();
                }
                else
                {
                    if (MarkServerOnline())
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

                LastHeartbeat = Now;

                if (!Online)
                {
                    ActivePlayerCount = PlayerCount;

                    MarkServerOnline();
                    Save();
                }
                else if (ActivePlayerCount != PlayerCount)
                {
                    ActivePlayerCount = PlayerCount;
                    Save();
                }
                else
                {
                    Timesave();
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
                LastHeartbeat = Now;

                if ((this.Platform != Platform) ||
                    (this.BuildNumber != BuildNumber) ||
                    (Online == false))
                {
                    this.Platform = Platform;
                    this.BuildNumber = BuildNumber;
                    ActivePlayerCount = 0;

                    if (!Online)
                        MarkServerOnline();

                    Save();
                }
                else
                {
                    Timesave();
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
                LastHeartbeat = Now;

                if ((this.ExpansionsMask != ExpansionsMask) ||
                    (this.ModuleName != ModuleName) ||
                    (Online == false))
                {
                    this.ExpansionsMask = ExpansionsMask;
                    this.ModuleName = ModuleName;

                    if (!Online)
                        MarkServerOnline();

                    Save();
                }
                else
                {
                    Timesave();
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
                LastHeartbeat = Now;

                if ((ExpansionsMask != Info.ExpansionsMask) ||
                    (MaximumPlayerCount != Info.MaximumPlayers) ||
                    (ActivePlayerCount != Info.ActivePlayers) ||
                    (LocalVault != Info.IsLocalVault) ||
                    (BuildNumber != Info.BuildNumber) ||
                    (PrivateServer != Info.HasPlayerPassword) ||
                    (ModuleName != Info.ModuleName) ||
                    (MinimumLevel != Info.MinLevel) ||
                    (MaximumLevel != Info.MaximumPlayers) ||
                    (PVPLevel != Info.PVPLevel) ||
                    (PlayerPause != Info.IsPlayerPauseAllowed) ||
                    (OnePartyOnly != Info.IsOnePartyOnly) ||
                    (ELCEnforced != Info.IsELC) ||
                    (ILREnforced != Info.HasILR))
                {
                    ExpansionsMask = Info.ExpansionsMask;
                    MaximumPlayerCount = Info.MaximumPlayers;
                    ActivePlayerCount = Info.ActivePlayers;
                    LocalVault = Info.IsLocalVault;
                    PrivateServer = Info.HasPlayerPassword;
                    BuildNumber = Info.BuildNumber;
                    ModuleName = Info.ModuleName;
                    MinimumLevel = Info.MinLevel;
                    MaximumLevel = Info.MaxLevel;
                    PVPLevel = Info.PVPLevel;
                    PlayerPause = Info.IsPlayerPauseAllowed;
                    OnePartyOnly = Info.IsOnePartyOnly;
                    ELCEnforced = Info.IsELC;
                    ILREnforced = Info.HasILR;

                    if (!Online)
                        MarkServerOnline();

                    Save();
                }
                else
                {
                    Timesave();
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
                LastHeartbeat = Now;

                if (this.ServerName != ServerName)
                {
                    this.ServerName = ServerName;

                    if (!Online)
                        MarkServerOnline();

                    Save();
                }
                else
                {
                    Timesave();
                }
            }
        }

        /// <summary>
        /// Called when a module description update is available.
        /// </summary>
        /// <param name="ModuleDescription">Supplies the new module
        /// description.</param>
        /// <param name="ModuleUrl">Supplies the new module url.</param>
        /// <param name="GameType">Supplies the new game type.</param>
        /// <param name="PWCUrl">Supplies the new PWC url.</param>
        /// <param name="BuildNumber">Supplies the new build number.</param>
        /// <param name="GameDetails">Supplies the server description.</param>
        public void OnDescriptionInfoUpdate(string ModuleDescription, string ModuleUrl, uint GameType, string PWCUrl, UInt16 BuildNumber, string GameDetails)
        {
            DateTime Now = DateTime.UtcNow;

            lock (this)
            {
                LastHeartbeat = Now;

                if ((this.ModuleDescription != ModuleDescription) ||
                    (this.ModuleUrl != ModuleUrl) ||
                    (this.GameType != GameType) ||
                    (this.PWCUrl != PWCUrl) ||
                    (this.BuildNumber != BuildNumber) ||
                    (this.ServerDescription != GameDetails))
                {
                    this.ModuleDescription = ModuleDescription;
                    this.ModuleUrl = ModuleUrl;
                    this.GameType = GameType;
                    this.PWCUrl = PWCUrl;
                    this.BuildNumber = BuildNumber;
                    this.ServerDescription = GameDetails;

                    if (!Online)
                        MarkServerOnline();

                    Save();
                }
                else
                {
                    Timesave();
                }
            }
        }

        /// <summary>
        /// Initiate an auto save if necessary.  The server is assumed to be
        /// locked.
        /// </summary>
        private void Timesave()
        {
            uint Now = (uint)Environment.TickCount;

            if ((Now - LastSaveTick) >= TIMESAVE_INTERVAL)
                Save();
        }

        /// <summary>
        /// Attempt to mark a server as online, scheduling a heartbeat unless
        /// the server is already considered a NAT duplicate.
        /// </summary>
        /// <returns>True if the server was actually marked online.</returns>
        private bool MarkServerOnline()
        {
            uint Now = (uint)Environment.TickCount;

            if ((Now - NATDuplicateTick) >= NAT_DUPLICATE_INTERVAL)
            {
                Online = true;
                StartHeartbeat();
                return true;
            }
            else
            {
                return false;
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
                if (InitialHeartbeat)
                {
                    InitialHeartbeat = false;
                }
                else if ((Now >= LastHeartbeat) &&
                         (Now - LastHeartbeat) >= NWServerTracker.HeartbeatCutoffTimeSpan)
                {
                    ActivePlayerCount = 0;
                    Online = false;
                    Expired = true;

                    if (!Save())
                    {
                        Logger.Log(LogLevel.Error, "NWGameServer.HeartbeatTimer_Elapsed(): Server {0} could not be saved as expired.", this);
                        Expired = false;
                    }
                }
            }

            if (Expired)
            {
                Logger.Log(LogLevel.Normal, "NWGameServer.HeartbeatTimer_Elapsed(): Server {0} expired from online server list due to heartbeat timeout.", this);
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
        /// True if this is the initial on restore heartbeat.
        /// </summary>
        public bool InitialHeartbeat { get; set; }

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
        /// The module description.
        /// </summary>
        public string ModuleDescription { get; set; }

        /// <summary>
        /// The module URL.
        /// </summary>
        public string ModuleUrl { get; set; }

        /// <summary>
        /// The game type (category).
        /// </summary>
        public uint GameType { get; set; }

        /// <summary>
        /// The PWC URL.
        /// </summary>
        public string PWCUrl { get; set; }

        /// <summary>
        /// The server description ("GameDetails") field.
        /// </summary>
        public string ServerDescription { get; set; }

        /// <summary>
        /// The minimum acceptable character level.
        /// </summary>
        public uint MinimumLevel { get; set; }

        /// <summary>
        /// The maximum acceptable character level.
        /// </summary>
        public uint MaximumLevel { get; set; }

        /// <summary>
        /// The PVP level.
        /// </summary>
        public uint PVPLevel { get; set; }

        /// <summary>
        /// The player pause setting.
        /// </summary>
        public bool PlayerPause { get; set; }

        /// <summary>
        /// The one party only setting.
        /// </summary>
        public bool OnePartyOnly { get; set; }

        /// <summary>
        /// The ELC enforcement setting.
        /// </summary>
        public bool ELCEnforced;

        /// <summary>
        /// The ILR enforcement setting.
        /// </summary>
        public bool ILREnforced;

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
        /// The interval, in milliseconds, at which the heartbeat interval is
        /// updated in the database for servers that do not change player
        /// status information (i.e. which remain with a constant count of
        /// users, etc.).
        /// </summary>
        private const uint TIMESAVE_INTERVAL = 15 * 60 * 1000;

        /// <summary>
        /// The interval, in milliseconds, at which the server object is
        /// blacklisted and prevented from coming back online if it was marked
        /// as a NAT duplicate.
        /// </summary>
        private const uint NAT_DUPLICATE_INTERVAL = (4 * HEARTBEAT_INTERVAL);


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

        /// <summary>
        /// The last tick that a timesave occurred at.
        /// </summary>
        private uint LastSaveTick = (uint)Environment.TickCount - TIMESAVE_INTERVAL;

        /// <summary>
        /// The time at which at NAT duplicate was detected.
        /// </summary>
        private uint NATDuplicateTick = (uint)Environment.TickCount - NAT_DUPLICATE_INTERVAL;
    }
}

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
                if ((Now >= LastHeartbeat) ||
                    (Now - LastHeartbeat) < NWServerTracker.HeartbeatCutoffTimeSpan)
                {
                    Online = false;
                    Expired = true;
                }
            }

            if (Expired)
            {
                Logger.Log("NWGameServer.HeartbeatTimer_Elapsed(): Server {0} expired from online server list due to heartbeat timeout.", this);
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
        /// The count of active players on the server.
        /// </summary>
        public uint ActivePlayerCount { get; set; }

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
        /// The internal database ID of the server, or zero if the server has
        /// not had an ID assigned yet.
        /// </summary>
        public int DatabaseId { get; set; }


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

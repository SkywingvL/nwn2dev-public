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
            this.MasterServer = MasterServer;
            this.Address = ServerAddress;
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
        }

        /// <summary>
        /// Stop the periodic heartbeat, if it is running.  The server is
        /// assumed to be locked.
        /// </summary>
        public void StopHeartbeat()
        {
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
        /// The address of the server.
        /// </summary>
        private IPEndPoint Address;

        /// <summary>
        /// Back link to the underlying master server instance.
        /// </summary>
        private NWMasterServer MasterServer;
    }
}

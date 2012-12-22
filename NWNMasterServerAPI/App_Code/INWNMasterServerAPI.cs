/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    INWNMasterServerAPI.cs

Abstract:

    This module houses the interface definition for the INWNMasterServerAPI web
    service.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.ServiceModel.Web;
using System.Text;

namespace NWN
{
    /// <summary>
    /// This interface represents the primary API surface for the NWN Master
    /// Server system.
    /// </summary>
    [ServiceContract(Namespace = "http://api.mst.valhallalegends.com/NWNMasterServerAPI")]
    public interface INWNMasterServerAPI
    {

        /// <summary>
        /// Search the server database for a server by name, and return any
        /// matching online servers (zero or more).
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="ServerName">Supplies the server name.</param>
        /// <returns>A list of matching online servers is returned.</returns>
        [OperationContract]
        IList<NWGameServer> LookupServerByName(string Product, string ServerName);

        /// <summary>
        /// Search the server database for a server by address, and return stored
        /// information about it (the server may be offline).
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="ServerAddress">Supplies the server address.  This value
        /// takes the form of "ip:port".</param>
        /// <returns>The matching server.</returns>
        [OperationContract]
        NWGameServer LookupServerByAddress(string Product, string ServerAddress);

        /// <summary>
        /// Search the server database for all servers that are online for the
        /// given game product.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <returns>A list of matching online servers is returned.</returns>
        [OperationContract]
        IList<NWGameServer> GetOnlineServerList(string Product);

        /// <summary>
        /// Get a count of online users (players) for a given product.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <returns>The count of active users across all known servers for the
        /// given product.</returns>
        [OperationContract]
        uint GetOnlineUserCount(string Product);

        /// <summary>
        /// Get the list of supported product names, e.g. "NWN1", "NWN2".
        /// </summary>
        /// <returns>The list of supported product name values.</returns>
        [OperationContract]
        IList<string> GetSupportedProductList();

        /// <summary>
        /// Search the server database for a server by module, and return any
        /// matching servers (zero or more).
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="Module">Supplies the server name.</param>
        /// <returns>A list of matching servers is returned.</returns>
        [OperationContract]
        IList<NWGameServer> LookupServerByModule(string Product, string Module);

        /// <summary>
        /// Register a list of pending servers with the master server
        /// infrastructure, so that they will be queued for inspection as
        /// active game servers.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="ServerAddresses">Supplies the list of server addresses
        /// to register, up to 50 ip:port pairs accepted.</param>
        /// <returns>The count of servers processed.</returns>
        [OperationContract]
        uint RegisterPendingServers(string Product, string[] ServerAddresses);

        /// <summary>
        /// Search the server database for a server by game type, and return
        /// any matching servers (zero or more).
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="GameType">Supplies the GameType code to search by.</param>
        /// <returns>A list of matching servers is returned.</returns>
        [OperationContract]
        IList<NWGameServer> LookupServerByGameType(string Product, uint GameType);

        /// <summary>
        /// Get the Client Extension update and message of the day information.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="ClientExtensionVersion">Supplies the Client Extension
        /// version number in packed format.</param>
        /// <returns>The message of the day string, else an empty string if
        /// there was no string to display.</returns>
        [OperationContract]
        ClientExtensionUpdate GetClientExtensionUpdate(string Product, uint ClientExtensionVersion);

        /// <summary>
        /// Search the server database for a server by game type, and return
        /// any matching servers (zero or more).  This private API is intended
        /// for use by the Client Extension so that the results may be tailored
        /// to a particular Client Extension version if necessary.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="GameType">Supplies the GameType code to search by.</param>
        /// <param name="ClientExtensionVersion">Supplies the Client Extension
        /// version number in packed format.</param>
        /// <returns>A list of matching servers is returned.</returns>
        [OperationContract]
        IList<NWGameServer> LookupServerByGameTypeClientExtension(string Product, uint GameType, uint ClientExtensionVersion);

        /// <summary>
        /// Increment a Client Extension statistic.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="Statistic">Supplies the statistic name.</param>
        /// <returns>Reserved for future use; zero at present.</returns>
        [OperationContract]
        uint IncrementStatistic(string Product, string Statistic);
    }

    /// <summary>
    /// This class represents data information associated with a NWN game server.
    /// </summary>
    [DataContract]
    public class NWGameServer
    {
        /// <summary>
        /// The product name ("NWN1", "NWN2").
        /// </summary>
        [DataMember]
        public string Product { get; set; }

        /// <summary>
        /// Bitmask of expansions required.
        /// 0x00 - No expansions.
        /// 0x01 - XP1.
        /// 0x02 - XP2.
        /// </summary>
        [DataMember]
        public uint ExpansionsMask { get; set; }

        /// <summary>
        /// The game server software build number.
        /// </summary>
        [DataMember]
        public uint BuildNumber { get; set; }

        /// <summary>
        /// The name of the loaded module (if any).
        /// </summary>
        [DataMember]
        public string ModuleName { get; set; }

        /// <summary>
        /// The name of the server (if any).
        /// </summary>
        [DataMember]
        public string ServerName { get; set; }

        /// <summary>
        /// The current count of players logged on to the server.
        /// </summary>
        [DataMember]
        public uint ActivePlayerCount { get; set; }

        /// <summary>
        /// The maximum configured limit on concurrent players for the server.
        /// </summary>
        [DataMember]
        public uint MaximumPlayerCount { get; set; }

        /// <summary>
        /// True if the server is a local vault server, else false for a server
        /// vault server.
        /// </summary>
        [DataMember]
        public bool LocalVault { get; set; }

        /// <summary>
        /// The last UTC time that the server responded to a heartbeat message.
        /// </summary>
        [DataMember]
        public DateTime LastHeartbeat { get; set; }

        /// <summary>
        /// The hostname and port information used by player clients to connect to
        /// the server directly.  The information is returned in the format of
        /// "ip:port".
        /// </summary>
        [DataMember]
        public string ServerAddress { get; set; }

        /// <summary>
        /// True if the server is currently online.
        /// </summary>
        [DataMember]
        public bool Online { get; set; }

        /// <summary>
        /// True if the server is private and requires a player password in order
        /// to connect.
        /// </summary>
        [DataMember]
        public bool PrivateServer { get; set; }

        /// <summary>
        /// The module description.
        /// </summary>
        [DataMember]
        public string ModuleDescription { get; set; }

        /// <summary>
        /// The module URL.
        /// </summary>
        [DataMember]
        public string ModuleUrl { get; set; }

        /// <summary>
        /// The game type (category) selected for the server.
        /// </summary>
        [DataMember]
        public uint GameType { get; set; }

        /// <summary>
        /// The minimum supported character level on the server.
        /// </summary>
        [DataMember]
        public uint MinimumLevel { get; set; }

        /// <summary>
        /// The maximum supported character level on the server.
        /// </summary>
        [DataMember]
        public uint MaximumLevel { get; set; }

        /// <summary>
        /// The PVP level enabled on the server.
        /// </summary>
        [DataMember]
        public uint PVPLevel { get; set; }

        /// <summary>
        /// True if players are permitted to pause the server.
        /// </summary>
        [DataMember]
        public bool PlayerPause { get; set; }

        /// <summary>
        /// True if only one party is permitted on the server.
        /// </summary>
        [DataMember]
        public bool OnePartyOnly { get; set; }

        /// <summary>
        /// True if ELC is enforced on the server.
        /// </summary>
        [DataMember]
        public bool ELCEnforced { get; set; }

        /// <summary>
        /// True if ILR is enforced on the server.
        /// </summary>
        [DataMember]
        public bool ILREnforced { get; set; }

        /// <summary>
        /// The persistent world client file URL for the server.
        /// </summary>
        [DataMember]
        public string PWCUrl { get; set; }
    }

    /// <summary>
    /// This class represents update and message of the day parameters for the
    /// NWN2 Client Extension.
    /// </summary>
    [DataContract]
    public class ClientExtensionUpdate
    {
        /// <summary>
        /// The non-update message of the day to display, or an empty string if
        /// no message should be shown.  The message is shown on CE attach, or
        /// client startup.
        /// </summary>
        [DataMember]
        public string MOTD { get; set; }

        /// <summary>
        /// The download URL for an available update, or an empty string if no
        /// update should be pushed to the client.  Only used if UpdateVersion
        /// is set.
        /// </summary>
        [DataMember]
        public string UpdateUrl { get; set; }

        /// <summary>
        /// The version number of the new Client Extension version that is now
        /// available, or an empty string if no new update is available.
        /// </summary>
        [DataMember]
        public string UpdateVersion { get; set; }

        /// <summary>
        /// An informational URL to show to the client for an update, instead
        /// of an automatic update.  Only used if UpdateVersion is set and
        /// UpdateUrl is empty, or if the update fails.
        /// </summary>
        [DataMember]
        public string InfoUrl { get; set; }

        /// <summary>
        /// A human-readable description to display for the new update message.
        /// </summary>
        [DataMember]
        public string UpdateDescription { get; set; }
    }
}

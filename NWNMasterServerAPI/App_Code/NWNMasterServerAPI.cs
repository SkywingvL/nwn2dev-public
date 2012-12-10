/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NWNMasterServerAPI.cs

Abstract:

    This module houses logic to implement the INWNMasterServerAPI web service.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.Serialization;
using System.ServiceModel;
using System.ServiceModel.Web;
using System.Text;
using MySql.Data.MySqlClient;
using System.Configuration;

/// <summary>
/// This class implements the master server communication API.
/// </summary>
[ServiceBehavior(Namespace = "http://api.mst.valhallalegends.com/NWNMasterServerAPI")]
public class NWNMasterServerAPI : INWNMasterServerAPI
{

    /// <summary>
    /// Search the server database for a server by name, and return any
    /// matching online servers (zero or more).
    /// </summary>
    /// <param name="Product">Supplies the product name, such as NWN2.</param>
    /// <param name="ServerName">Supplies the server name.</param>
    /// <returns>A list of matching online servers is returned.</returns>
    public IList<NWGameServer> LookupServerByName(string Product, string ServerName)
    {
        List<NWGameServer> Servers = new List<NWGameServer>();

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
AND `server_name` = '{1}'
",
        ProductNameToId(Product),
        MySqlHelper.EscapeString(ServerName));

        using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
        {
            while (Reader.Read())
            {
                NWGameServer Server = new NWGameServer();

                Server.ExpansionsMask = Reader.GetUInt32(1);
                Server.BuildNumber = Reader.GetUInt32(2);
                Server.ModuleName = Reader.GetString(3);
                Server.ServerName = Reader.GetString(4);
                Server.ActivePlayerCount = Reader.GetUInt32(5);
                Server.MaximumPlayerCount = Reader.GetUInt32(6);
                Server.LocalVault = Reader.GetBoolean(7);
                Server.LastHeartbeat = Reader.GetDateTime(8);
                Server.ServerAddress = Reader.GetString(9);
                Server.Online = Reader.GetBoolean(10);
                Server.PrivateServer = Reader.GetBoolean(11);

                Servers.Add(Server);
            }
        }

        return Servers;
    }

    /// <summary>
    /// Search the server database for a server by address, and return stored
    /// information about it (the server may be offline).
    /// </summary>
    /// <param name="Product">Supplies the product name, such as NWN2.</param>
    /// <param name="ServerAddress">Supplies the server address.  This value
    /// takes the form of "ip:port".</param>
    /// <returns>The matching server.</returns>
    public NWGameServer LookupServerByAddress(string Product, string ServerAddress)
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
AND `server_address` = '{1}'
",
        ProductNameToId(Product),
        MySqlHelper.EscapeString(ServerAddress));

        using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
        {
            if (Reader.Read())
            {
                NWGameServer Server = new NWGameServer();

                Server.ExpansionsMask = Reader.GetUInt32(1);
                Server.BuildNumber = Reader.GetUInt32(2);
                Server.ModuleName = Reader.GetString(3);
                Server.ServerName = Reader.GetString(4);
                Server.ActivePlayerCount = Reader.GetUInt32(5);
                Server.MaximumPlayerCount = Reader.GetUInt32(6);
                Server.LocalVault = Reader.GetBoolean(7);
                Server.LastHeartbeat = Reader.GetDateTime(8);
                Server.ServerAddress = Reader.GetString(9);
                Server.Online = Reader.GetBoolean(10);
                Server.PrivateServer = Reader.GetBoolean(11);

                return Server;
            }
        }

        return null;
    }

    /// <summary>
    /// Search the server database for all servers that are online for the
    /// given game product.
    /// </summary>
    /// <param name="Product">Supplies the product name, such as NWN2.</param>
    /// <returns>A list of matching online servers is returned.</returns>
    public IList<NWGameServer> GetOnlineServerList(string Product)
    {
        List<NWGameServer> Servers = new List<NWGameServer>();

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
",
        ProductNameToId(Product));

        using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
        {
            while (Reader.Read())
            {
                NWGameServer Server = new NWGameServer();

                Server.ExpansionsMask = Reader.GetUInt32(1);
                Server.BuildNumber = Reader.GetUInt32(2);
                Server.ModuleName = Reader.GetString(3);
                Server.ServerName = Reader.GetString(4);
                Server.ActivePlayerCount = Reader.GetUInt32(5);
                Server.MaximumPlayerCount = Reader.GetUInt32(6);
                Server.LocalVault = Reader.GetBoolean(7);
                Server.LastHeartbeat = Reader.GetDateTime(8);
                Server.ServerAddress = Reader.GetString(9);
                Server.Online = Reader.GetBoolean(10);
                Server.PrivateServer = Reader.GetBoolean(11);

                Servers.Add(Server);
            }
        }

        return Servers;
    }

    /// <summary>
    /// Map a product name to product ID code in the database.
    /// </summary>
    /// <param name="Product">Supplies the product name.</param>
    /// <returns>The corresponding product id, else zero if the product was not
    /// valid.</returns>
    private int ProductNameToId(string Product)
    {
        if (Product == "NWN2")
            return 2;
        else if (Product == "NWN1")
            return 1;
        else
            return 0;
    }

    /// <summary>
    /// The connection string for the web service.
    /// </summary>
    private static string ConnectionString = ((DatabaseSettings)ConfigurationManager.GetSection("NWNMasterServerAPISettings/DatabaseSettings")).ConnectionString;
}


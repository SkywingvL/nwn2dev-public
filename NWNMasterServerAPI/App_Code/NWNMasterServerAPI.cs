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
using System.Net;

namespace NWN
{
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
                StandardServerQueryPrefix +
    @"WHERE `product_id` = {0}
    AND `online` = true
    AND `server_name` = '{1}'
    ",
            ProductNameToId(Product),
            MySqlHelper.EscapeString(ServerName));

            using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
            {
                while (Reader.Read())
                {
                    NWGameServer Server = LoadGameServerFromQuery(Reader);

                    Server.Product = Product;

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
                StandardServerQueryPrefix +
    @"WHERE `product_id` = {0}
    AND `server_address` = '{1}'
    ",
            ProductNameToId(Product),
            MySqlHelper.EscapeString(ServerAddress));

            using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
            {
                if (Reader.Read())
                {
                    NWGameServer Server = LoadGameServerFromQuery(Reader);

                    Server.Product = Product;

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
                StandardServerQueryPrefix + 
    @"WHERE `product_id` = {0}
    AND `online` = true
    ",
            ProductNameToId(Product));

            using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
            {
                while (Reader.Read())
                {
                    NWGameServer Server = LoadGameServerFromQuery(Reader);

                    Server.Product = Product;

                    Servers.Add(Server);
                }
            }

            return Servers;
        }

        /// <summary>
        /// Get a count of online users (players) for a given product.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <returns>The count of active users across all known servers for the
        /// given product.</returns>
        public uint GetOnlineUserCount(string Product)
        {
            string Query = String.Format(
    @"SELECT
        SUM(active_player_count)
    FROM
        `game_servers`
    WHERE `product_id` = {0} 
    AND `online` = true
    ",
            ProductNameToId(Product));

            using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
            {
                if (!Reader.Read())
                    return 0;

                return Reader.GetUInt32(0);
            }
        }

        /// <summary>
        /// Get the list of supported product names, e.g. "NWN1", "NWN2".
        /// </summary>
        /// <returns>The list of supported product name values.</returns>
        public IList<string> GetSupportedProductList()
        {
            return new List<string> { "NWN1", "NWN2" };
        }

        /// <summary>
        /// Search the server database for a server by module, and return any
        /// matching servers (zero or more).
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="Module">Supplies the server name.</param>
        /// <returns>A list of matching servers is returned.</returns>
        public IList<NWGameServer> LookupServerByModule(string Product, string Module)
        {
            List<NWGameServer> Servers = new List<NWGameServer>();

            string Query = String.Format(
                StandardServerQueryPrefix +
    @"WHERE `product_id` = {0}
    AND `online` = true
    AND `module_name` = '{1}'
    ",
            ProductNameToId(Product),
            MySqlHelper.EscapeString(Module));

            using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query))
            {
                while (Reader.Read())
                {
                    NWGameServer Server = LoadGameServerFromQuery(Reader);

                    Server.Product = Product;

                    Servers.Add(Server);
                }
            }

            return Servers;
        }

        /// <summary>
        /// Register a list of pending servers with the master server
        /// infrastructure, so that they will be queued for inspection as
        /// active game servers.
        /// </summary>
        /// <param name="Product">Supplies the product name, such as NWN2.</param>
        /// <param name="ServerAddresses">Supplies the list of server addresses
        /// to register, up to 50 ip:port pairs accepted.</param>
        /// <returns>The count of servers processed.</returns>
        public uint RegisterPendingServers(string Product, string[] ServerAddresses)
        {
            int ProductId;
            uint Processed;

            if (ServerAddresses == null || ServerAddresses.Length == 0 || ServerAddresses.Length > 50)
                return 0;

            if (Product == null)
                return 0;

            ProductId = ProductNameToId(Product);

            if (ProductId == 0)
                return 0;

            //
            // Process each server in the request list by inserting it into the
            // pending_game_servers table.  The server address is round-tripped
            // through the IPAddress parser to ensure that it can be properly
            // handled by the master server service.
            //

            StringBuilder Query = new StringBuilder();
            Processed = 0;

            foreach (string Address in ServerAddresses)
            {
                int i = Address.IndexOf(':');
                IPEndPoint ServerAddress;

                if (i == -1)
                {
                    continue;
                }

                try
                {
                    ServerAddress = new IPEndPoint(
                        IPAddress.Parse(Address.Substring(0, i)),
                        Convert.ToInt32(Address.Substring(i + 1)));
                }
                catch
                {
                    continue;
                }

                Query.AppendFormat(
@"INSERT INTO `pending_game_servers` (
    `pending_game_server_id`,
    `product_id`,
    `server_address`)
VALUES (
    0,
    {0},
    '{1}')
ON DUPLICATE KEY UPDATE
    `pending_game_server_id` = `pending_game_server_id`;",
                    ProductId,
                    MySqlHelper.EscapeString(ServerAddress.ToString()));
                Processed += 1;
            }

            if (Processed > 0)
            {
                using (MySqlDataReader Reader = MySqlHelper.ExecuteReader(ConnectionString, Query.ToString()))
                {
                }
            }

            return Processed;
        }


        /// <summary>
        /// Read server parameters from the standard database query column
        /// layout and construct and return a NWGameServer corresponding to the
        /// data in question.
        /// </summary>
        /// <param name="Reader">Supplies a reader containing the results of a
        /// query that correspond to the standard column set.</param>
        /// <returns>A NWGameServer object describing the current row.</returns>
        private NWGameServer LoadGameServerFromQuery(MySqlDataReader Reader)
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

            return Server;
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
        /// The standard query prefix for queries to return a server.  For use
        /// with LoadGameServerFromQuery().
        /// </summary>
        private static string StandardServerQueryPrefix = @"
SELECT `game_server_id`,
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
        `pwc_url`
    FROM `game_servers` ";

        /// <summary>
        /// The connection string for the web service.
        /// </summary>
        private static string ConnectionString = ((DatabaseSettings)ConfigurationManager.GetSection("NWNMasterServerAPISettings/DatabaseSettings")).ConnectionString;
    }
}

/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    NWMasterServer.cs

Abstract:

    This module houses master server logic for handling incoming requests and
    dispatching them as appropriate.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Timers;
using NWN;
using NWN.Parsers;
using MySql.Data.MySqlClient;

namespace NWNMasterServer
{
    internal class NWMasterServer
    {

        /// <summary>
        /// Instantiate a new master server instance.
        /// </summary>
        /// <param name="ServiceObject">Optionally supplies the associated
        /// service instance, else null if the program is not running in
        /// service mode.</param>
        public NWMasterServer(ServiceController ServiceObject)
        {
            this.ServiceObject = ServiceObject;
            QueryCombineTimer = new System.Timers.Timer(QUERY_COMBINE_INTERVAL);

            QueryCombineTimer.AutoReset = false;
            QueryCombineTimer.Elapsed += new ElapsedEventHandler(QueryCombineTimer_Elapsed);
        }

        /// <summary>
        /// Synchronously execute the server.
        /// </summary>
        public void Run()
        {
            //
            // Set the log file first, if one existed.
            //

            if (!String.IsNullOrEmpty(ServerSettings.Default.LogFileName))
                Logger.OpenLogFile(ServerSettings.Default.LogFileName);

            //
            // Set the log filter level.
            //

            Logger.LogFilterLevel = (LogLevel)Enum.Parse(typeof(LogLevel),
                ServerSettings.Default.LogLevel);

            try
            {
                bool IsShutdown = false;

                //
                // Bind the socket and prepare it for I/O.
                //

                SetupSockets();

                //
                // Create the underlying server tracker.
                //

                ServerTracker = new NWServerTracker(this);

                //
                // Queue the initial block of receive buffers.
                //

                MasterServerNATDuplicateSocket.InitiateAllReceives();
                GameSpySocket.InitiateAllReceives();
                MasterServerSocket.InitiateAllReceives();

                //
                // Inform the server tracker that it is clear to begin
                // heartbeat operations.
                //

                ServerTracker.QueueInitialHeartbeats();

                Logger.Log(LogLevel.Normal, "NWMasterServer.Run(): Master server version {0} initialized (game build advertised: {1}).",
                    Assembly.GetExecutingAssembly().GetName().Version,
                    BuildNumber);

                //
                // Block waiting for a quit request.  Once requested, initiate
                // shutdown on the socket and wait for outstanding receive
                // operations to drain.  Once these have drained, close the
                // socket and terminate the server.
                //

                for (; ; )
                {
                    QuitEvent.WaitOne();

                    if (!IsShutdown)
                    {
                        //
                        // Shut down future receives and inform the server
                        // tracker that new heartbeat requests should be
                        // drained out.
                        //

                        MasterServerSocket.Shutdown();
                        MasterServerNATDuplicateSocket.Shutdown();
                        GameSpySocket.Shutdown();

                        IsShutdown = true;
                        ServerTracker.DrainHeartbeats();
                    }

                    if (PendingBuffers == 0)
                        break;
                }

                //
                // Finally, flush any lingering pending queries.
                //

                lock (QueryCombineBuffer)
                {
                    if (QueryCombineBuffer.Length != 0)
                    {
                        ExecuteQueryNoReader(QueryCombineBuffer.ToString());
                        QueryCombineBuffer.Clear();
                    }
                }

                Logger.Log(LogLevel.Normal, "NWMasterServer.Run(): Main loop exiting.");

                if (ServiceObject != null)
                    ServiceObject.RequestStop();
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.Run(): Exception: {0}", e);
            }
        }

        /// <summary>
        /// Request a service stop in an asynchronous fashion.
        /// </summary>
        public void Stop()
        {
            QuitRequested = true;
            QuitEvent.Set();
        }

        /// <summary>
        /// Execute a query.
        /// </summary>
        /// <param name="Query">Supplies the query to execute.</param>
        /// <returns>A reader for the query results is returned.</returns>
        public MySqlDataReader ExecuteQuery(string Query)
        {
            try
            {
                return MySqlHelper.ExecuteReader(ConnectionString, Query);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.ExecuteQueryNoReader(): Exception in query '{0}': '{1}'.",
                    Query,
                    e);
                throw;
            }
        }

        /// <summary>
        /// Execute a query that returns nothing.
        /// </summary>
        /// <param name="Query">Supplies the query to execute.</param>
        public void ExecuteQueryNoReader(string Query)
        {
            try
            {
                MySqlHelper.ExecuteNonQuery(ConnectionString, Query);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.ExecuteQueryNoReader(): Exception in query '{0}': '{1}'.",
                    Query,
                    e);
                throw;
            }
        }

        /// <summary>
        /// Convert a DateTime to MySQL-compatible format.
        /// </summary>
        /// <param name="Date">The date to convert.</param>
        /// <returns>The MySQL-compatible date string.</returns>
        public string DateToSQLDate(DateTime Date)
        {
            return String.Format("{0:D4}-{1:D2}-{2:D2} {3:D2}:{4:D2}:{5:D2}",
                Date.Year,
                Date.Month,
                Date.Day,
                Date.Hour,
                Date.Minute,
                Date.Second);
        }

        /// <summary>
        /// Execute a query that returns nothing, allowing the query to be held
        /// for query combining.
        /// </summary>
        /// <param name="Query">Supplies the query to execute.</param>
        public void ExecuteQueryNoReaderCombine(string Query)
        {
            string DirectQuery = null;

            try
            {
                bool StartTimer = false;

                //
                // Attempt to perform query combining.  If the maximum limit is
                // reached, flush the queue directly.  If this was the first
                // query being pushed on to the queue, set the timer.
                //

                lock (QueryCombineBuffer)
                {
                    int OldLength = QueryCombineBuffer.Length;

                    QueryCombineBuffer.Append(Query);
                    QueryCombineBuffer.Append(";");

                    if (QueryCombineBuffer.Length > QUERY_COMBINE_LENGTH)
                    {
                        DirectQuery = QueryCombineBuffer.ToString();
                        QueryCombineBuffer.Clear();
                    }
                    else if (OldLength == 0)
                    {
                        StartTimer = true;
                    }
                }

                if (StartTimer)
                    QueryCombineTimer.Start();

                if (DirectQuery != null)
                    MySqlHelper.ExecuteNonQuery(ConnectionString, Query);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.ExecuteQueryNoReaderCombine(): Exception in query '{0}': '{1}'.",
                    String.IsNullOrEmpty(DirectQuery) ? Query : DirectQuery,
                    e);
                throw;
            }
        }

        /// <summary>
        /// This timer callback runs when the query combine timer elapses.  It
        /// checks whether there is a pending query queue, and if so, flushes
        /// it as appropriate.
        /// </summary>
        /// <param name="sender">Unused.</param>
        /// <param name="e">Unused.</param>
        private void QueryCombineTimer_Elapsed(object sender, ElapsedEventArgs e)
        {
            string DirectQuery;

            lock (QueryCombineBuffer)
            {
                if (QueryCombineBuffer.Length == 0)
                    return;

                DirectQuery = QueryCombineBuffer.ToString();
                QueryCombineBuffer.Clear();
            }

            ExecuteQueryNoReader(DirectQuery);
        }

        /// <summary>
        /// Setup the various server sockets.
        /// </summary>
        private void SetupSockets()
        {
            IPAddress BindIP;
            IPAddress NATDuplicateBindIP;

            if (String.IsNullOrEmpty(BindAddress))
                BindIP = IPAddress.Any;
            else
                BindIP = IPAddress.Parse(BindAddress);

            if (String.IsNullOrEmpty(NATDuplicateBindAddress))
                NATDuplicateBindIP = IPAddress.Any;
            else
                NATDuplicateBindIP = IPAddress.Parse(NATDuplicateBindAddress);

            MasterServerSocket = new SocketInfo(this, BindIP, (int)MasterServerPort, SocketUse.MasterServer, OnRecvMstDatagram);
            MasterServerNATDuplicateSocket = new SocketInfo(this, NATDuplicateBindIP, 0, SocketUse.MasterServerNATDuplicate, OnRecvMstDatagram);
            GameSpySocket = new SocketInfo(this, BindIP, GAMESPY_SERVER_PORT, SocketUse.GameSpy, OnRecvGameSpyDatagram);
        }

        /// <summary>
        /// Initiate an asynchronous receive operation on a buffer.
        /// </summary>
        /// <param name="Buffer">Supplies the buffer to receive from.</param>
        private void InitiateReceive(SocketRecvBuffer Buffer)
        {
            //
            // If shutdown is in process, then prevent new receive operations
            // from being initiated and wake the main thread if the last
            // pending receive has been drained.
            //

            if (QuitRequested)
            {
                if (PendingBuffers == 0)
                    QuitEvent.Set();

                return;
            }

            IncrementPendingBuffers();

            try
            {
                EndPoint RecvEndPoint = (EndPoint)Buffer.Sender;
                IAsyncResult Result = Buffer.SocketDescriptor.Socket.BeginReceiveFrom(
                    Buffer.Buffer,
                    0,
                    Buffer.Buffer.Length,
                    SocketFlags.None,
                    ref RecvEndPoint,
                    RecvCompletionCallback,
                    Buffer);
            }
            catch (Exception e)
            {
                DecrementPendingBuffers();

                Logger.Log(LogLevel.Error, "NWMasterServer.InitiateReceive(): BeginReceiveFrom failed: Exception: {0}", e);
                Stop();
                return;
            }
        }

        /// <summary>
        /// This callback delegate handles socket I/O completion; when the I/O
        /// has finished, it performs data processing as required.
        /// </summary>
        /// <param name="Result">Supplies the I/O result.</param>
        private void RecvCompletionCallback(IAsyncResult Result)
        {
            SocketRecvBuffer Buffer = (SocketRecvBuffer)Result.AsyncState;
            EndPoint RecvEndPoint = (EndPoint)Buffer.Sender;
            int RecvLen;

            if (!QuitRequested)
            {
                try
                {
                    RecvLen = Buffer.SocketDescriptor.Socket.EndReceiveFrom(Result, ref RecvEndPoint);
                }
                catch (SocketException e)
                {
                    RecvLen = -1;

                    if (e.SocketErrorCode == SocketError.ConnectionReset)
                    {
                        RecvLen = 0;
                    }
                    else
                    {
                        Logger.Log(LogLevel.Error, "NWMasterServer.RecvCompletionCallback(): Unexpected receive error: {0} (Exception: {1})",
                            e.SocketErrorCode,
                            e);
                    }
                }
                catch (Exception e)
                {
                    RecvLen = -1;

                    Logger.Log(LogLevel.Error, "NWMasterServer.RecvCompletionCallback(): EndReceiveFrom failed: Exception: {0}", e);
                }
            }
            else
            {
                RecvLen = -1;
            }

            try
            {
                if (RecvLen > 0)
                {
                    SocketInfo SocketDescriptor = Buffer.SocketDescriptor;

                    SocketDescriptor.OnRecvSocketDatagram(Buffer.Buffer, RecvLen, (IPEndPoint)RecvEndPoint, SocketDescriptor);
                }
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.RecvCompletionCallback(): Exception: {0}", e);
            }

            DecrementPendingBuffers();

            InitiateReceive(Buffer);
        }

        /// <summary>
        /// Send completion callback.
        /// </summary>
        /// <param name="Result">Supplies the associated async result.</param>
        private void SendCompletionCallback(IAsyncResult Result)
        {
            int SentBytes;
            SocketSendState State = (SocketSendState)Result.AsyncState;
            IPEndPoint Recipient = State.Recipient;

            try
            {
                SentBytes = State.Socket.EndSendTo(Result);
            }
            catch (SocketException e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.SendCompletionCallback(): Unexpected receive error for host {0}: {1} (Exception: {2})",
                    Recipient,
                    e.SocketErrorCode,
                    e);
            }
            catch (Exception e)
            {
                Logger.Log(LogLevel.Error, "NWMasterServer.SendCompletionCallback(): Exception: {0}", e);
            }
        }

        /// <summary>
        /// This method handles a received datagram on the master server
        /// socket.  It examines the request and dispatches it as appropriate.
        /// </summary>
        /// <param name="Buffer">Supplies the received datagram.</param>
        /// <param name="RecvLen">Supplies the length of the received
        /// datagram.</param>
        /// <param name="Sender">Supplies the datagram sender.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstDatagram(byte[] Buffer, int RecvLen, IPEndPoint Sender, SocketInfo Socket)
        {
            if (RecvLen < 4)
                return;

            unsafe
            {
                uint Cmd;

                Cmd = ((uint)Buffer[0] <<  0) |
                      ((uint)Buffer[1] <<  8) |
                      ((uint)Buffer[2] << 16) |
                      ((uint)Buffer[3] << 24);

                fixed (void *ByteData = &Buffer[4])
                {
                    ExoParseBuffer ParseBuffer;

                    ParseBuffer = new ExoParseBuffer(ByteData, (uint)RecvLen - 4, null, 0);

                    OnRecvMstMessage(Cmd, ParseBuffer, Sender, Socket);
                }
            }
        }

        /// <summary>
        /// This method handles a received datagram on the GameSpy server
        /// socket.  It examines the request and dispatches it as appropriate.
        /// </summary>
        /// <param name="Buffer">Supplies the received datagram.</param>
        /// <param name="RecvLen">Supplies the length of the received
        /// datagram.</param>
        /// <param name="Sender">Supplies the datagram sender.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvGameSpyDatagram(byte[] Buffer, int RecvLen, IPEndPoint Sender, SocketInfo Socket)
        {
            if (RecvLen < 1)
                return;

            unsafe
            {
                uint Cmd = (uint)Buffer[0];

                fixed (void* ByteData = &Buffer[1])
                {
                    ExoParseBuffer ParseBuffer;

                    ParseBuffer = new ExoParseBuffer(ByteData, (uint)RecvLen - 1, null, 0);

                    OnRecvGameSpyMessage(Cmd, ParseBuffer, Sender, Socket);
                }
            }
        }

        /// <summary>
        /// This method handles a received master server communication protocol
        /// message.  The message is examined and then dispatched to a handler.
        /// </summary>
        /// <param name="Cmd">Supplies the command code of the message.</param>
        /// <param name="ParseBuffer">Supplies the message body.</param>
        /// <param name="Sender">Supplies the reply address for the
        /// sender.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstMessage(uint Cmd, ExoParseBuffer ParseBuffer, IPEndPoint Sender, SocketInfo Socket)
        {
            switch (Cmd)
            {

                case (uint)MstCmd.CommunityAuthorizationRequest:
                    OnRecvMstCommunityAuthorizationRequest(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.CDKeyAuthorizationRequest:
                    OnRecvMstCDKeyAuthorizationRequest(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.Heartbeat:
                    OnRecvMstHeartbeat(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.DisconnectNotify:
                    OnRecvMstDisconnectNotify(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.StartupNotify:
                    OnRecvMstStartupNotify(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.ModuleLoadNotify:
                    OnRecvMstModuleLoadNotify(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.MOTDRequest:
                    OnRecvMstMOTDRequest(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.VersionRequest:
                    OnRecvMstVersionRequest(ParseBuffer, Sender, Socket);
                    break;

                case (uint)MstCmd.StatusRequest:
                    OnRecvMstStatusRequest(ParseBuffer, Sender, Socket);
                    break;

                case (uint)ConnAuthCmd.ServerInfoResponse:
                    OnRecvServerInfoResponse(ParseBuffer, Sender, Socket);
                    break;

                case (uint)ConnAuthCmd.ServerNameResponse:
                    OnRecvServerNameResponse(ParseBuffer, Sender, Socket);
                    break;

                case (uint)ConnAuthCmd.ServerDescriptionResponse:
                    OnRecvServerDescriptionResponse(ParseBuffer, Sender, Socket);
                    break;

            }
        }

        /// <summary>
        /// This method parses and handles a community authorization request
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstCommunityAuthorizationRequest(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            UInt16 Length;
            byte[] ServerChallenge;
            string AccountName;
            byte[] ClientVerifier;
            UInt16 Language;
            Byte Platform;
            Byte IsPlayer;

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadWORD(out Length))
                return;
            if ((ServerChallenge = Parser.ReadBytes(Length)) == null)
                return;
            if (!Parser.ReadSmallString(out AccountName, 16))
                return;
            if (!Parser.ReadWORD(out Length))
                return;
            if ((ClientVerifier = Parser.ReadBytes(Length)) == null)
                return;
            if (!Parser.ReadWORD(out Language))
                return;
            if (!Parser.ReadBYTE(out Platform))
                return;
            if (!Parser.ReadBYTE(out IsPlayer))
                return;

            SendMstCommunityAccountAuthorization(Sender, AccountName, ConnectStatus.CONNECT_ERR_SUCCESS);
        }

        /// <summary>
        /// This method parses and handles a CD-Key authorization request from
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstCDKeyAuthorizationRequest(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            UInt16 EntryCount;
            UInt32 ClientIP;
            UInt16 ClientPort;
            byte[] ServerChallenge;
            UInt16 Length;
            List<CDKeyInfo> CDKeyHashes;
            CDKeyInfo CDKeyHash;
            string AccountName;
            int KeyIndex;
            int PortNumberHbo;

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadWORD(out EntryCount))
                return;
            if (EntryCount != 1)
                return;
            if (!Parser.ReadDWORD(out ClientIP))
                return;
            if (!Parser.ReadWORD(out ClientPort))
                return;

            PortNumberHbo = IPAddress.NetworkToHostOrder(ClientPort);

            if ((PortNumberHbo == 0) || ((PortNumberHbo & 0xFFFF) != PortNumberHbo))
                PortNumberHbo = 5120;

            IPEndPoint ClientEndpoint = new IPEndPoint((long)ClientIP, PortNumberHbo);

            if (!Parser.ReadWORD(out Length))
                return;
            if ((ServerChallenge = Parser.ReadBytes(Length)) == null)
                return;
            if (!Parser.ReadWORD(out Length))
                return;

            CDKeyHashes = new List<CDKeyInfo>();
            CDKeyHash = new CDKeyInfo();

            KeyIndex = 0;
            while (Length-- != 0)
            {
                UInt16 HashLength;

                if (!Parser.ReadSmallString(out CDKeyHash.PublicCDKey, 16))
                    return;
                if (!Parser.ReadWORD(out HashLength))
                    return;
                if ((CDKeyHash.CDKeyHash = Parser.ReadBytes(HashLength)) == null)
                    return;

                //
                // N.B.  The following is somewhat of a hack in that we are not
                //       bothering to extract the (real) product type from the
                //       CD-Key.  As a result, it is possible that the wrong
                //       answer could be provided for clients without all of
                //       the expansions; this situation is considered unlikely
                //       in the current state of affairs.
                //
                //       A better fit could be had by examining the expansion
                //       mask required by the server in the data table, but
                //       this does not appear worthwhile at this stage.
                //

                if (KeyIndex == 0)
                    CDKeyHash.Product = 0;
                else
                    CDKeyHash.Product = (UInt16) (1 << (KeyIndex - 1));

                CDKeyHash.AuthStatus = (UInt16)ConnectStatus.CONNECT_ERR_SUCCESS;

                CDKeyHashes.Add(CDKeyHash);
            }

            if (!Parser.ReadSmallString(out AccountName, 16))
                return;

            SendMstCDKeyAuthorization(Sender, CDKeyHashes);
        }

        /// <summary>
        /// This method parses and handles a heartbeat message from a game
        /// server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstHeartbeat(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 PlayerCount;
            List<List<string>> PlayerCDKeyList = new List<List<string>>();

            if (!Parser.ReadWORD(out PlayerCount))
                return;

            while (PlayerCount-- != 0)
            {
                UInt16 CDKeyCount;
                List<string> CDKeyList = new List<string>();

                if (!Parser.ReadWORD(out CDKeyCount))
                    return;

                while (CDKeyCount-- != 0)
                {
                    string CDKey;

                    if (!Parser.ReadSmallString(out CDKey, 16))
                        return;

                    CDKeyList.Add(CDKey);
                }

                PlayerCDKeyList.Add(CDKeyList);
            }

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvMstHeartbeat(): Server {0} ActivePlayerCount={1}.", Sender, PlayerCDKeyList.Count);
        }

        /// <summary>
        /// This method parses and handles a player disconnect notify message
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstDisconnectNotify(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            UInt16 EntryCount;
            UInt16 CDKeyCount;
            List<string> CDKeyList;
            NWGameServer Server;

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadWORD(out EntryCount))
                return;

            if (EntryCount == 0)
            {
                //
                // Server shutdown.
                //

                Server = ServerTracker.LookupServerByAddress(Sender, false);

                if (Server == null || Server.Online == false)
                    return;

                //
                // Record the server shutdown.
                //

                Server.OnShutdownNotify();
                Logger.Log(LogLevel.Normal, "NWMasterServer.OnRecvMstShutdownNotify(): Server {0} shut down.", Sender);
                return;
            }
            else if (EntryCount != 1)
            {
                return;
            }

            if (!Parser.ReadWORD(out CDKeyCount))
                return;

            CDKeyList = new List<string>();

            while (CDKeyCount-- != 0)
            {
                string CDKey;

                if (!Parser.ReadSmallString(out CDKey, 16))
                    return;

                CDKeyList.Add(CDKey);
            }

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvMstDisconnectNotify()");
        }

        /// <summary>
        /// This method parses and handles a server startup notify message from
        /// a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstStartupNotify(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            Byte Platform;
            UInt16 BuildNumber;
            Byte Unknown0; // 0
            Byte Unknown1; // 0
            Byte Unknown2; // 1
            Byte Unknown3; // 0
            Byte Unknown4; // 3

            if (!Parser.ReadBYTE(out Platform))
                return;
            if (!Parser.ReadWORD(out BuildNumber))
                return;
            if (!Parser.ReadBYTE(out Unknown0))
                return;
            if (!Parser.ReadBYTE(out Unknown1))
                return;
            if (!Parser.ReadBYTE(out Unknown2))
                return;
            if (!Parser.ReadBYTE(out Unknown3))
                return;
            if (!Parser.ReadBYTE(out Unknown4))
                return;

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            //
            // Record the server startup.
            //

            Server.OnStartupNotify(Platform, BuildNumber);
            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvMstStartupNotify(): Server {0} Platform={1} BuildNumber={2}.", Sender, (char)Platform, BuildNumber);
        }

        /// <summary>
        /// This method parses and handles a module load notify message from a
        /// game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstModuleLoadNotify(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            Byte ExpansionsMask;
            string ModuleName;

            if (!Parser.ReadBYTE(out ExpansionsMask))
                return;
            if (!Parser.ReadSmallString(out ModuleName, 16))
                return;

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            //
            // Record the module load and request an updated player count.
            //

            Server.OnModuleLoad(ExpansionsMask, ModuleName);
            RefreshServerStatus(Sender);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvMstModuleLoadNotify(): Server {0} ModuleName={1} ExpansionsMask={2}.", Sender, ModuleName, ExpansionsMask);
        }

        /// <summary>
        /// This method parses and handles a MOTD request from a game server or
        /// game client.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstMOTDRequest(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            Byte Unknown0; // 0
            Byte Unknown1; // 0

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadBYTE(out Unknown0))
                return;
            if (!Parser.ReadBYTE(out Unknown1))
                return;

            SendMstMOTD(Sender, MOTD);
        }

        /// <summary>
        /// This method parses and handles a version request from a game server
        /// or game client.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstVersionRequest(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            Byte Unknown0; // 0
            Byte Unknown1; // 0
            Byte Unknown2; // 1

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadBYTE(out Unknown0))
                return;
            if (!Parser.ReadBYTE(out Unknown1))
                return;
            if (!Parser.ReadBYTE(out Unknown2))
                return;

            SendMstVersion(Sender, BuildNumber);
        }

        /// <summary>
        /// This method parses and handles a status request from a game server
        /// or game client.
        /// </summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvMstStatusRequest(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;

            if (!Parser.ReadWORD(out DataPort))
                return;

            //
            // Do not enter the server into the server list until
            // bidirectional communication has been established.  However,
            // some broken NATs may respond briefly to the current source
            // port and then choose another (different!) source port later
            // for future pings.  To handle this case, send a BNXI probe to
            // both internal and external addresses.  If the responses are
            // the same, then assume that the server is actually located at
            // the internal address; otherwise, create server records for
            // both servers.
            //

            if (DataPort != (UInt16)Sender.Port)
            {
                IPEndPoint InternalAddress = new IPEndPoint(Sender.Address, (int)DataPort);

                SendServerInfoRequest(InternalAddress);
            }

            SendServerInfoRequest(Sender);

            SendMstStatusResponse(Sender, MstStatus.MST_STATUS_ONLINE);
        }

        /// <summary>
        /// This method parses a server info response from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvServerInfoResponse(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            Byte Reserved; // 0xFC
            Byte HasPlayerPassword;
            Byte MinLevel;
            Byte MaxLevel;
            Byte ActivePlayers;
            Byte MaximumPlayers;
            Byte IsLocalVault;
            Byte PVPLevel;
            Byte IsPlayerPauseAllowed;
            Byte IsOnePartyOnly;
            Byte IsELC;
            Byte HasILR;
            Byte ExpansionsMask;
            string ModuleName;
            string BuildNumber;
            ServerInfo Info = new ServerInfo();

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadBYTE(out Reserved))
                return;

            if (Mode == GameMode.NWN2)
            {
                if (Reserved != 0xFC)
                    return;
            }
            else
            {
                if (Reserved != 0xFD)
                    return;
            }

            if (!Parser.ReadBYTE(out HasPlayerPassword))
                return;
            if (!Parser.ReadBYTE(out MinLevel))
                return;
            if (!Parser.ReadBYTE(out MaxLevel))
                return;
            if (!Parser.ReadBYTE(out ActivePlayers))
                return;
            if (!Parser.ReadBYTE(out MaximumPlayers))
                return;
            if (!Parser.ReadBYTE(out IsLocalVault))
                return;
            if (!Parser.ReadBYTE(out PVPLevel))
                return;
            if (!Parser.ReadBYTE(out IsPlayerPauseAllowed))
                return;
            if (!Parser.ReadBYTE(out IsOnePartyOnly))
                return;
            if (!Parser.ReadBYTE(out IsELC))
                return;
            if (!Parser.ReadBYTE(out HasILR))
                return;
            if (!Parser.ReadBYTE(out ExpansionsMask))
                return;
            if (!Parser.ReadSmallString(out ModuleName))
                return;

            if (Mode == GameMode.NWN2)
            {
                if (!Parser.ReadSmallString(out BuildNumber))
                    return;
            }
            else
            {
                BuildNumber = "0";
            }

            try
            {
                Info.BuildNumber = Convert.ToUInt16(BuildNumber);
            }
            catch
            {
                Info.BuildNumber = 0;
            }

            Info.HasPlayerPassword = (HasPlayerPassword != 0);
            Info.MinLevel = MinLevel;
            Info.MaxLevel = MaxLevel;
            Info.ActivePlayers = ActivePlayers;
            Info.MaximumPlayers = MaximumPlayers;
            Info.IsLocalVault = (IsLocalVault != 0);
            Info.PVPLevel = PVPLevel;
            Info.IsPlayerPauseAllowed = (IsPlayerPauseAllowed != 0);
            Info.IsOnePartyOnly = (IsOnePartyOnly != 0);
            Info.IsELC = (IsELC != 0);
            Info.HasILR = (HasILR != 0);
            Info.ExpansionsMask = ExpansionsMask;
            Info.ModuleName = ModuleName;

            //
            // Look up the server and update the current server information.
            // Since the BNXR reply is used to differentiate between broken
            // NATs and endpoints with multiple servers on the same IP address,
            // carefully check for whether a duplicate server record exists on
            // the server internal port before creating a new server record.
            //

            NWGameServer Server;

            Server = ServerTracker.LookupServerByAddress(Sender);

            if (Mode != GameMode.NWN2)
                Info.BuildNumber = Server.BuildNumber;

            Server.OnServerInfoUpdate(Info);

            if (DataPort == (UInt16)Sender.Port)
            {
                //
                // Both internal and external ports match; the sender is not
                // likely behind a NAT.  No action is necessary behind the
                // creation of the server record above.
                //
            }
            else
            {
                NWGameServer ServerInternal;
                IPEndPoint InternalAddress = new IPEndPoint(Sender.Address, (int)DataPort);
                
                ServerInternal = ServerTracker.LookupServerByAddress(InternalAddress, false);

                if (ServerInternal == null)
                {
                    //
                    // No record of a server existing at the internal address
                    // is yet known.  Proceed to create the server record at
                    // the external address (as was already performed above).
                    //
                }
                else
                {
                    //
                    // A record exists for both internal and external
                    // addresses for the server.  If the configuration values
                    // between both servers are the same, then mark the
                    // external address version as offline and prefer the
                    // internal server address as authoritative (since it must
                    // be globally reachable for a response to have been
                    // received).
                    //

                    if (ServerInternal.CheckForNATDuplicate(Server))
                    {
                        Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvServerInfoResponse(): Removing NAT duplicate server {0} in preference of server {1}.", Sender, InternalAddress);
                        return;
                    }
                }
            }

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvServerInfoResponse(): Server {0} has {1}/{2} players ({3}).", Sender, Info.ActivePlayers, Info.MaximumPlayers,
                 Info.ModuleName);
        }

        /// <summary>
        /// This method parses a server name response from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvServerNameResponse(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            Byte UpdateType;
            UInt16 DataPort;
            Byte RequestCorrelationCookie;
            string ServerName;

            if (!Parser.ReadBYTE(out UpdateType))
                return;
            if (UpdateType != 'U')
                return;
            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadBYTE(out RequestCorrelationCookie))
                return;
            if (RequestCorrelationCookie != 0)
                return;
            if (!Parser.ReadSmallString(out ServerName))
                return;

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            Server.OnServerNameUpdate(ServerName);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvServerNameResponse(): Server {0} name is {1}.", Sender, ServerName);
        }

        /// <summary>
        /// This method parses a server name response from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvServerDescriptionResponse(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            UInt16 DataPort;
            string GameDetails;
            string ModuleDescription;
            string BuildNumber;
            UInt16 GameType;
            string ModuleUrl;
            string PWCUrl;
            UInt16 Build;

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadSmallString(out GameDetails, 32))
                return;
            if (!Parser.ReadSmallString(out ModuleDescription, 32))
                return;
            if (!Parser.ReadSmallString(out BuildNumber, 32))
                return;
            if (!Parser.ReadWORD(out GameType))
                return;

            if (Mode == GameMode.NWN2)
            {
                if (!Parser.ReadSmallString(out ModuleUrl, 32))
                    return;
                if (!Parser.ReadSmallString(out PWCUrl, 32))
                    return;
            }
            else
            {
                ModuleUrl = "";
                PWCUrl = "";
            }

            try
            {
                Build = Convert.ToUInt16(BuildNumber);
            }
            catch
            {
                Build = 0;
            }

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            Server.OnDescriptionInfoUpdate(ModuleDescription, ModuleUrl, GameType, PWCUrl, Build, GameDetails);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.OnRecvServerDescriptionResponse(): Server {0} description '{1}' URL '{2}' has game type {3}.",
                Sender,
                ModuleDescription,
                ModuleUrl,
                GameType);
        }


        /// <summary>
        /// This method sends a community account authorization response to a
        /// game server, or game client (for initial login).
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="AccountName">Supplies the account name.</param>
        /// <param name="Status">Supplies the authorization status
        /// code.</param>
        public void SendMstCommunityAccountAuthorization(IPEndPoint Address, string AccountName, ConnectStatus Status)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.CommunityAuthorization);
            Builder.WriteSmallString(AccountName, 16);
            Builder.WriteWORD((ushort)Status);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstCommunityAccountAuthorization(): Authorizing account {0} for server {1} with status {2}.", AccountName, Address, Status);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends a CD-Key authorization response to a game server,
        /// or game client (for initial login).
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="CDKeys">Supplies the CD-Key list.</param>
        public void SendMstCDKeyAuthorization(IPEndPoint Address, IList<CDKeyInfo> CDKeys)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.CDKeyAuthorization);
            Builder.WriteWORD((ushort)CDKeys.Count);

            foreach (CDKeyInfo CDKey in CDKeys)
            {
                Builder.WriteSmallString(CDKey.PublicCDKey, 16);
                Builder.WriteWORD(CDKey.AuthStatus);
                Builder.WriteWORD(CDKey.Product);
            }

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstCDKeyAuthorization(): Authorizing {0} CD-Keys for server {1}.", CDKeys, Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends a heartbeat request to a game server, requesting
        /// that it reply with a list of current players.  This confirms that
        /// the server is, in fact, still up and responding.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        public void SendMstDemandHeartbeat(IPEndPoint Address)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.DemandHeartbeat);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstDemandHeartbeat(): Requesting heartbeat for server {0}.", Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends the message of the day to a game server, or game
        /// client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Message">Supplies the announcement message.</param>
        public void SendMstMOTD(IPEndPoint Address, string Message)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.MOTDResponse);
            Builder.WriteSmallString(Message, 16);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstMOTD(): Sending MOTD to {0}.", Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends the current build number to a game server, or
        /// game client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="BuildNumber">Supplies the build number string.</param>
        public void SendMstVersion(IPEndPoint Address, string BuildNumber)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.VersionResponse);
            Builder.WriteSmallString(BuildNumber, 16);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstVersion(): Sending version to {0}.", Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends a master server status response message to a game
        /// server, or game client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="StatusFlags">Supplies the master server status flags.
        /// </param>
        public void SendMstStatusResponse(IPEndPoint Address, MstStatus StatusFlags)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)MstCmd.StatusResponse);
            Builder.WriteDWORD((ushort)StatusFlags);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendMstStatusResponse(): Sending status response to {0}.", Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method sends a server info request to a server.
        /// </summary>
        /// <param name="Address">Supplies the game server address.</param>
        public void SendServerInfoRequest(IPEndPoint Address)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)ConnAuthCmd.ServerInfoRequest);
            Builder.WriteWORD((ushort)MasterServerPort);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendServerInfoRequest(): Sending server info request to {0}.", Address);

            SendRawDataToMstClientNATDuplicate(Address, Builder);
        }

        /// <summary>
        /// This method sends a server name request to a server.
        /// </summary>
        /// <param name="Address">Supplies the game server address.</param>
        public void SendServerNameRequest(IPEndPoint Address)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)ConnAuthCmd.ServerNameRequest);
            Builder.WriteWORD((ushort)MasterServerPort);
            Builder.WriteBYTE(0); // Request correlation cookie.

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendServerNameRequest(): Sending server name request to {0}.", Address);

            SendRawDataToMstClientNATDuplicate(Address, Builder);
        }

        /// <summary>
        /// This method sends a server description request to a server.
        /// </summary>
        /// <param name="Address">Supplies the game server address.</param>
        public void SendServerDescriptionRequest(IPEndPoint Address)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteDWORD((uint)ConnAuthCmd.ServerDescriptionRequest);
            Builder.WriteWORD((ushort)MasterServerPort);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendServerDescriptionRequest(): Sending server description request to {0}.", Address);

            SendRawDataToMstClientNATDuplicate(Address, Builder);
        }


        /// <summary>
        /// This method sends status requests to a server to refresh server
        /// status information.
        /// </summary>
        /// <param name="Address">Supplies the server address.</param>
        public void RefreshServerStatus(IPEndPoint Address)
        {
            SendServerInfoRequest(Address);
            SendServerNameRequest(Address);
            SendServerDescriptionRequest(Address);
        }


        /// <summary>
        /// This method handles a received GameSpy server communication
        /// protocol message.  The message is examined and then dispatched to
        /// a handler.
        /// </summary>
        /// <param name="Cmd">Supplies the command code of the message.</param>
        /// <param name="ParseBuffer">Supplies the message body.</param>
        /// <param name="Sender">Supplies the reply address for the
        /// sender.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvGameSpyMessage(uint Cmd, ExoParseBuffer ParseBuffer, IPEndPoint Sender, SocketInfo Socket)
        {
            switch (Cmd)
            {

                case (uint)GameSpyCmd.CheckServerStatus:
                    OnRecvGameSpyCheckServerStatus(ParseBuffer, Sender, Socket);
                    break;

            }
        }

        /// <summary>
        /// This method parses a GameSpy check server status request from a
        /// client.</summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        /// <param name="Socket">Supplies the associated socket descriptor
        /// upon which the message was received.</param>
        private void OnRecvGameSpyCheckServerStatus(ExoParseBuffer Parser, IPEndPoint Sender, SocketInfo Socket)
        {
            //
            // The message contains:
            // (DWORD) 0
            // (STRING) ProductName "nwn2"
            //

            SendGameSpyCheckServerStatusResponse(Sender);
        }

        /// <summary>
        /// This method sends a GameSpy server status acknowledgement back to
        /// a GameSpy client, informing the client that GameSpy services should
        /// be considered as available and operational.
        /// </summary>
        /// <param name="Address">Supplies the recipient address.</param>
        public void SendGameSpyCheckServerStatusResponse(IPEndPoint Address)
        {
            ExoBuildBuffer Builder = new ExoBuildBuffer();

            Builder.WriteWORD(0xFDFE);
            Builder.WriteBYTE((byte)GameSpyCmd.CheckServerStatus);
            Builder.WriteDWORD(0);

            Logger.Log(LogLevel.Verbose, "NWMasterServer.SendGameSpyCheckServerStatusResponse(): Sending GameSpy aliveness acknowledgement to {0}.", Address);

            SendRawDataToGameSpyClient(Address, Builder);
        }




        /// <summary>
        /// This method transmits a raw datagram to a master server client,
        /// such as a game server or game client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Builder">Supplies the message body.</param>
        private void SendRawDataToMstClient(IPEndPoint Address, ExoBuildBuffer Builder)
        {
            SendRawDataToSocket(Address, Builder, MasterServerSocket);
        }

        /// <summary>
        /// This method transmits a raw datagram to a master server client,
        /// such as a game server or game client.  The message is sent using
        /// the NAT duplicate detector socket.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Builder">Supplies the message body.</param>
        private void SendRawDataToMstClientNATDuplicate(IPEndPoint Address, ExoBuildBuffer Builder)
        {
            SendRawDataToSocket(Address, Builder, MasterServerNATDuplicateSocket);
        }

        /// <summary>
        /// This method transmits a raw datagram to a GameSpy server client,
        /// such as a game client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Builder">Supplies the message body.</param>
        private void SendRawDataToGameSpyClient(IPEndPoint Address, ExoBuildBuffer Builder)
        {
            SendRawDataToSocket(Address, Builder, GameSpySocket);
        }

        /// <summary>
        /// This method transmits a raw datagram to a socket.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Builder">Supplies the message body.</param>
        /// <param name="Socket">Supplies the socket to send to.</param>
        private void SendRawDataToSocket(IPEndPoint Address, ExoBuildBuffer Builder, SocketInfo Socket)
        {
            unsafe
            {
                byte* ByteStream;
                uint ByteStreamLength;
                byte* BitStream;
                uint BitStreamLength;
                byte[] CompositeBuffer;
                uint CompositeBufferLength;

                Builder.GetBuffer(
                    out ByteStream,
                    out ByteStreamLength,
                    out BitStream,
                    out BitStreamLength);

                CompositeBufferLength = ByteStreamLength + BitStreamLength;

                if (CompositeBufferLength == 0)
                    return;

                //
                // Allocate a contiguous message buffer, and fail here and now
                // if we can't.
                //

                try
                {
                    CompositeBuffer = new byte[CompositeBufferLength];

                    //
                    // Assemble the discrete components into a single
                    // contiguous composite buffer for outbound trnasmission.
                    //

                    Marshal.Copy(
                        (IntPtr)ByteStream,
                        CompositeBuffer,
                        0,
                        (int)ByteStreamLength);

                    for (uint i = 0; i < BitStreamLength; i += 1)
                    {
                        CompositeBuffer[ByteStreamLength + i] = BitStream[i];
                    }

                    //
                    // Transmit the underlying message now that we have
                    // captured it into a flat buffer consumable by the I/O
                    // system.
                    //

                    IAsyncResult Result = Socket.Socket.BeginSendTo(
                        CompositeBuffer,
                        0,
                        (int)CompositeBufferLength,
                        SocketFlags.None,
                        Address,
                        SendCompletionCallback,
                        new SocketSendState(Address, Socket.Socket));
                }
                catch (Exception e)
                {
                    Logger.Log(LogLevel.Error, "NWMasterServer.SendRawDataToSocket(): Failed to send data to Mst client {0}: Exception: {1}", Address, e);
                }
            }
        }


#pragma warning disable 420 // a reference to a volatile field will not be treated as volatile

        /// <summary>
        /// Increment the pending buffer count.
        /// </summary>
        private void IncrementPendingBuffers()
        {
            Interlocked.Increment(ref PendingBuffers);
        }

        /// <summary>
        /// Decrement the pending buffer count.
        /// </summary>
        private void DecrementPendingBuffers()
        {
            Interlocked.Decrement(ref PendingBuffers);
        }

#pragma warning restore 420


        /// <summary>
        /// A receive buffer that can contain data for a single datagram sent
        /// by a client or game server.
        /// </summary>
        private class SocketRecvBuffer
        {
            /// <summary>
            /// The buffer to receive on.
            /// </summary>
            public byte[] Buffer;

            /// <summary>
            /// Storage for the sender for an asynchronous receive.
            /// </summary>
            public IPEndPoint Sender;

            /// <summary>
            /// Back link to the associated socket descriptor.
            /// </summary>
            public SocketInfo SocketDescriptor;
        }

        /// <summary>
        /// Command codes for master server communicaiton.
        /// </summary>
        private enum MstCmd : uint
        {
            //
            // Game server (or game client) to master server requests.
            //

            CommunityAuthorizationRequest = 0x41504d42, // BMPA
            CDKeyAuthorizationRequest = 0x55414d42, // BMAU
            Heartbeat = 0x42484d42, // BMHB
            DisconnectNotify = 0x43444d42, // BMDC
            StartupNotify = 0x55534d42, // BMSU
            ModuleLoadNotify = 0x4f4d4d42, // BMMO
            MOTDRequest = 0x414d4d42, // BMMA
            VersionRequest = 0x41524d42, // BMRA
            StatusRequest = 0x54534d42, // BMST

            //
            // Master server to game server (or game client) requests.
            //

            CommunityAuthorization = 0x52504d42, // BMPR
            CDKeyAuthorization = 0x52414d42, // BMAR
            DemandHeartbeat = 0x48444d42, // BMDH
            MOTDResponse = 0x424d4d42, // BMMB
            VersionResponse = 0x42524d42, // BMRB
            StatusResponse = 0x52534d42, // BMSR
        }

        /// <summary>
        /// Command codes for connection authorization protocol.
        /// </summary>
        private enum ConnAuthCmd : uint
        {
            //
            // Client to server requests.
            //

            ServerInfoRequest = 0x49584e42, // BNXI
            ServerNameRequest = 0x53454e42, // BNES
            ServerDescriptionRequest = 0x53444e42, // BNDS

            //
            // Server to client requests.
            //

            ServerInfoResponse = 0x52584e42, // BNXR
            ServerNameResponse = 0x52454e42, // BNER
            ServerDescriptionResponse = 0x52444e42, // BNDR
        }

        /// <summary>
        /// Command codes for GameSpy protocol.
        /// </summary>
        public enum GameSpyCmd : uint
        {
            Query = 0x00,
            Challenge = 0x01,
            Heartbeat = 0x03,
            Keepalive = 0x08,
            CheckServerStatus = 0x09,

            RegStatus = 0x0A,
        }

        /// <summary>
        /// Connection response codes for authorization requests.
        /// </summary>
        public enum ConnectStatus : uint
        {
            CONNECT_ERR_SUCCESS                  = 0x00,
            CONNECT_ERR_UNKNOWN                  = 0x01,
            CONNECT_ERR_SERVER_VERSIONMISMATCH   = 0x02,
            CONNECT_ERR_PASSWORD_INCORRECT       = 0x03,
            CONNECT_ERR_SERVER_NOT_MULTIPLAYER   = 0x04,
            CONNECT_ERR_SERVER_FULL              = 0x05,
            CONNECT_ERR_PLAYER_NAME_IN_USE       = 0x06,
            CONNECT_ERR_REPLY_TIMEOUT            = 0x07,
            CONNECT_ERR_PLAYER_NAME_REFUSED      = 0x08,
            CONNECT_ERR_CD_KEY_IN_USE            = 0x09,
            CONNECT_ERR_BANNED                   = 0x0A,
            CONNECT_ERR_EXPANSION_PACK_WRONG     = 0x0B,
            CONNECT_ERR_CDKEY_UNAUTHORIZED       = 0x0C,
            CONNECT_ERR_DM_CONNECTION_REFUSED    = 0x0D,
            CONNECT_ERR_ADMIN_CONNECTION_REFUSED = 0x0E,
            CONNECT_ERR_LANGUAGE_VERSIONMISMATCH = 0x0F,
            CONNECT_ERR_MASTERSERVER_CD_IN_USE   = 0x10,
        }

        /// <summary>
        /// Master server status flags.
        /// </summary>
        public enum MstStatus : ushort
        {
            MST_STATUS_ONLINE = 0x0040,
        }

        /// <summary>
        /// CD-Key verification information.
        /// </summary>
        public struct CDKeyInfo
        {
            public string PublicCDKey;
            public byte[] CDKeyHash;
            public UInt16 AuthStatus;
            public UInt16 Product;
        }

        /// <summary>
        /// Server information block.
        /// </summary>
        public struct ServerInfo
        {
            public bool HasPlayerPassword;
            public uint MinLevel;
            public uint MaxLevel;
            public uint ActivePlayers;
            public uint MaximumPlayers;
            public bool IsLocalVault;
            public uint PVPLevel;
            public bool IsPlayerPauseAllowed;
            public bool IsOnePartyOnly;
            public bool IsELC;
            public bool HasILR;
            public byte ExpansionsMask;
            public string ModuleName;
            public UInt16 BuildNumber;
        }

        /// <summary>
        /// The game mode supported by the server.
        /// </summary>
        private enum GameMode
        {
            NWN1,
            NWN2
        }

        /// <summary>
        /// Types of a socket descriptor.
        /// </summary>
        private enum SocketUse
        {
            /// <summary>
            /// Master server protocol socket.
            /// </summary>
            MasterServer,

            /// <summary>
            /// Master server NAT duplicate detector socket.
            /// </summary>
            MasterServerNATDuplicate,

            /// <summary>
            /// GameSpy protocol socket.
            /// </summary>
            GameSpy,
        }

        /// <summary>
        /// Descriptor structure for a UDP socket.
        /// </summary>
        private class SocketInfo
        {
            /// <summary>
            /// Set up a SocketInfo descriptor for a socket and bind it to a
            /// local endpoint.
            /// </summary>
            /// <param name="MasterServer">Supplies the associated master
            /// server object.</param>
            /// <param name="BindAddress">Supplies the bind address.</param>
            /// <param name="BindPort">Supplies the bind port, else 0 if there
            /// is to be no specific local bind port.</param>
            /// <param name="SocketUse">Supplies the usage type of the
            /// socket.</param>
            /// <param name="OnRecvSocketDatagram">Supplies the high level
            /// socket receive message callback for the socket.</param>
            public SocketInfo(NWMasterServer MasterServer, IPAddress BindAddress, int BindPort, SocketUse SocketUse, OnRecvSocketDatagramDelegate OnRecvSocketDatagram)
            {
                this.MasterServer = MasterServer;
                this.SocketUse = SocketUse;
                this.OnRecvSocketDatagram = OnRecvSocketDatagram;

                for (int i = 0; i < BUFFER_COUNT; i += 1)
                {
                    SocketRecvBuffer Buffer = new SocketRecvBuffer();

                    Buffers[i] = Buffer;

                    Buffer.Buffer = new byte[MAX_FRAME_SIZE];
                    Buffer.Sender = new IPEndPoint(0, 0);
                    Buffer.SocketDescriptor = this;
                }

                Socket.Blocking = false;
                Socket.Bind(new IPEndPoint(BindAddress, BindPort));
                Socket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.PacketInformation, true);
            }

            /// <summary>
            /// Initiate receives on all receive buffers.
            /// </summary>
            public void InitiateAllReceives()
            {
                for (int i = 0; i < BUFFER_COUNT; i += 1)
                {
                    SocketRecvBuffer Buffer = Buffers[i];

                    MasterServer.InitiateReceive(Buffer);
                }
            }

            /// <summary>
            /// Shut down activity on the socket.
            /// </summary>
            public void Shutdown()
            {
                Socket.Shutdown(SocketShutdown.Receive);
                Socket.Close(15);
            }

            /// <summary>
            /// This delegate handles a received datagram on the socket.
            /// </summary>
            /// <param name="Buffer">Supplies the received datagram.</param>
            /// <param name="RecvLen">Supplies the length of the received
            /// datagram.</param>
            /// <param name="Sender">Supplies the datagram sender.</param>
            /// <param name="Socket">Supplies the associated socket descriptor
            /// upon which the message was received.</param>
            public delegate void OnRecvSocketDatagramDelegate(byte[] Buffer, int RecvLen, IPEndPoint Sender, SocketInfo Socket);

            /// <summary>
            /// Receive buffers for the socket.
            /// </summary>
            public SocketRecvBuffer[] Buffers = new SocketRecvBuffer[BUFFER_COUNT];

            /// <summary>
            /// The type of the socket.
            /// </summary>
            public SocketUse SocketUse;

            /// <summary>
            /// The underlying socket to issue I/O on.
            /// </summary>
            public Socket Socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

            /// <summary>
            /// Back link to the master server object itself.
            /// </summary>
            public NWMasterServer MasterServer;

            /// <summary>
            /// The high level socket receive callback for the socket
            /// descriptor.
            /// </summary>
            public OnRecvSocketDatagramDelegate OnRecvSocketDatagram;
        }

        /// <summary>
        /// State holder for socket send operations, used to field a socket
        /// send completion callback..
        /// </summary>
        private class SocketSendState
        {
            /// <summary>
            /// Create a new SocketSendState.
            /// </summary>
            /// <param name="Recipient">Supplies the message recipient.</param>
            /// <param name="Socket">Supplies the send socket.</param>
            public SocketSendState(IPEndPoint Recipient, Socket Socket)
            {
                this.Recipient = Recipient;
                this.Socket = Socket;
            }

            /// <summary>
            /// The message recipient.
            /// </summary>
            public IPEndPoint Recipient;

            /// <summary>
            /// The socket to send on.
            /// </summary>
            public Socket Socket;
        }
        
        /// <summary>
        /// The underlying interface with the SCM.
        /// </summary>
        private ServiceController ServiceObject;

        /// <summary>
        /// The event used to signal an exit request to worker threads.
        /// </summary>
        private EventWaitHandle QuitEvent = new EventWaitHandle(false, EventResetMode.AutoReset);

        /// <summary>
        /// True if shutdown has been requested.
        /// </summary>
        private volatile bool QuitRequested = false;

        /// <summary>
        /// Count of buffers to simultaneously attempt to fill.
        /// </summary>
        private const int BUFFER_COUNT = 5;

        /// <summary>
        /// The master server port number.
        /// </summary>
        private const int MASTER_SERVER_PORT = 6121;

        /// <summary>
        /// The GameSpy master server port number.
        /// </summary>
        private const int GAMESPY_SERVER_PORT = 27900;

        /// <summary>
        /// Maximum datagram frame size allowed.
        /// </summary>
        private const int MAX_FRAME_SIZE = 3200;

        /// <summary>
        /// The amount of time, in milliseconds, that queries should be held
        /// for combining.
        /// </summary>
        private const int QUERY_COMBINE_INTERVAL = 60000;

        /// <summary>
        /// The maximum length of a queued query string that is being held for
        /// combining, in characters.
        /// </summary>
        private const int QUERY_COMBINE_LENGTH = 256 * 1024;

        /// <summary>
        /// The combine buffer for query management.
        /// </summary>
        private StringBuilder QueryCombineBuffer = new StringBuilder();

        /// <summary>
        /// The timer for managing query combining.
        /// </summary>
        private System.Timers.Timer QueryCombineTimer = null;

        /// <summary>
        /// Receive buffers available to pull data into from the network.
        /// </summary>
        private SocketRecvBuffer[] Buffers = new SocketRecvBuffer[BUFFER_COUNT];

        /// <summary>
        /// The master server socket.
        /// </summary>
        private SocketInfo MasterServerSocket = null;

        /// <summary>
        /// The socket used to work around buggy NAT gateways that do not
        /// use the right source port when a static port forward is in use.
        /// </summary>
        private SocketInfo MasterServerNATDuplicateSocket = null;

        /// <summary>
        /// The GameSpy socket.
        /// </summary>
        private SocketInfo GameSpySocket = null;

        /// <summary>
        /// The count of buffers that have a receive pending.
        /// </summary>
        private volatile int PendingBuffers = 0;

        /// <summary>
        /// The server tracker instance that retains state about known servers.
        /// </summary>
        private NWServerTracker ServerTracker = null;

        /// <summary>
        /// The server bind address.
        /// </summary>
        private string BindAddress = ServerSettings.Default.BindAddress;

        /// <summary>
        /// The server bind address for the NAT duplicate detector socket.
        /// </summary>
        private string NATDuplicateBindAddress = ServerSettings.Default.NATDuplicateBindAddress;

        /// <summary>
        /// The server game build number.
        /// </summary>
        private string BuildNumber = ServerSettings.Default.BuildNumber;

        /// <summary>
        /// The server message of the day.
        /// </summary>
        private string MOTD = ServerSettings.Default.MOTD;

        /// <summary>
        /// The connection string for the database.
        /// </summary>
        private string DatabaseConnectionString = ServerSettings.Default.DatabaseConnectionString;

        /// <summary>
        /// The product ID for the game_servers table.
        /// </summary>
        private uint ServerProductID = ServerSettings.Default.ProductID;

        /// <summary>
        /// The default port number for the master server.
        /// </summary>
        private uint MasterServerPort = ServerSettings.Default.MasterServerPort;

        /// <summary>
        /// The game mode (e.g. protocol) that is supported by this server
        /// instance, such as NWN1 or NWN2.
        /// </summary>
        private GameMode Mode = (GameMode)Enum.Parse(typeof(GameMode), ServerSettings.Default.GameMode);

        /// <summary>
        /// Timekeeping for query combining.
        /// </summary>
        private uint LastQueryTick = (uint)Environment.TickCount - QUERY_COMBINE_INTERVAL;

        /// <summary>
        /// Return the server tracker object for external users.
        /// </summary>
        public NWServerTracker Tracker { get { return ServerTracker; } }

        /// <summary>
        /// Return the database connection string.
        /// </summary>
        public string ConnectionString { get { return DatabaseConnectionString; } }

        /// <summary>
        /// Return the product ID for the game_servers table.
        /// </summary>
        public uint ProductID { get { return ServerProductID; } }
    }
}

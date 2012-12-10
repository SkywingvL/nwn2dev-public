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
using NWN;
using NWN.Parsers;

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

            try
            {
                bool IsShutdown = false;

                //
                // Bind the socket and prepare it for I/O.
                //

                BindSocket();

                //
                // Create the underlying server tracker.
                //

                ServerTracker = new NWServerTracker(this);

                //
                // Queue the initial block of receive buffers.
                //

                for (int i = 0; i < BUFFER_COUNT; i += 1)
                {
                    Buffers[i].Buffer = new byte[MAX_FRAME_SIZE];
                    Buffers[i].Sender = new IPEndPoint(0, 0);
                    InitiateReceive(Buffers[i]);
                }

                //
                // Inform the server tracker that it is clear to begin
                // heartbeat operations.
                //

                ServerTracker.QueueInitialHeartbeats();

                Logger.Log("NWMasterServer.Run(): Master server version {0} initialized (game build advertised: {1}).",
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

                        ServerSocket.Shutdown(SocketShutdown.Receive);
                        ServerSocket.Close(15);
                        IsShutdown = true;
                        ServerTracker.DrainHeartbeats();
                    }

                    if (PendingBuffers == 0)
                        break;
                }
            }
            catch (Exception e)
            {
                Logger.Log("NWMasterServer.Run(): Exception: {0}", e);
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
        /// Bind the socket to the local listener endpoint.
        /// </summary>
        private void BindSocket()
        {
            EndPoint LocalEndPoint;

            if (String.IsNullOrEmpty(BindAddress))
                LocalEndPoint = new IPEndPoint(IPAddress.Any, MASTER_SERVER_PORT);
            else
                LocalEndPoint = new IPEndPoint(IPAddress.Parse(BindAddress), MASTER_SERVER_PORT);

            ServerSocket.Blocking = false;
            ServerSocket.Bind(LocalEndPoint);
            ServerSocket.SetSocketOption(SocketOptionLevel.IP, SocketOptionName.PacketInformation, true);
        }

        /// <summary>
        /// Initiate an asynchronous receive operation on a buffer.
        /// </summary>
        /// <param name="Buffer">Supplies the buffer to receive from.</param>
        private void InitiateReceive(RecvBuffer Buffer)
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

            try
            {
                PendingBuffers += 1;
                EndPoint RecvEndPoint = (EndPoint)Buffer.Sender;
                IAsyncResult Result = ServerSocket.BeginReceiveFrom(
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
                PendingBuffers -= 1;
                Logger.Log("NWMasterServer.InitiateReceive(): BeginReceiveFrom failed: Exception: {0}", e);
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
            RecvBuffer Buffer = (RecvBuffer)Result.AsyncState;
            EndPoint RecvEndPoint = (EndPoint)Buffer.Sender;
            int RecvLen;

            if (!QuitRequested)
            {
                try
                {
                    RecvLen = ServerSocket.EndReceiveFrom(Result, ref RecvEndPoint);
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
                        Logger.Log("NWMasterServer.RecvCompletionCallback(): Unexpected receive error: {0} (Exception: {1})",
                            e.SocketErrorCode,
                            e);
                    }
                }
                catch (Exception e)
                {
                    RecvLen = -1;

                    Logger.Log("NWMasterServer.RecvCompletionCallback(): EndReceiveFrom failed: Exception: {0}", e);
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
                    OnRecvDatagram(Buffer.Buffer, RecvLen, (IPEndPoint)RecvEndPoint);
                }
            }
            catch (Exception e)
            {
                Logger.Log("NWMasterServer.RecvCompletionCallback(): Exception: {0}", e);
            }

            PendingBuffers -= 1;

            InitiateReceive(Buffer);
        }

        private void SendCompletionCallback(IAsyncResult Result)
        {
            int SentBytes;
            IPEndPoint Recipient = (IPEndPoint)Result.AsyncState;

            try
            {
                SentBytes = ServerSocket.EndSendTo(Result);
            }
            catch (SocketException e)
            {
                Logger.Log("NWMasterServer.SendCompletionCallback(): Unexpected receive error for host {0}: {1} (Exception: {2})",
                    Recipient,
                    e.SocketErrorCode,
                    e);
            }
            catch (Exception e)
            {
                Logger.Log("NWMasterServer.SendCompletionCallback(): Exception: {0}", e);
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
        private void OnRecvDatagram(byte[] Buffer, int RecvLen, IPEndPoint Sender)
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

                    OnRecvMstMessage(Cmd, ParseBuffer, Sender);
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
        private void OnRecvMstMessage(uint Cmd, ExoParseBuffer ParseBuffer, IPEndPoint Sender)
        {
            switch (Cmd)
            {

                case (uint)MstCmd.CommunityAuthorizationRequest:
                    OnRecvMstCommunityAuthorizationRequest(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.CDKeyAuthorizationRequest:
                    OnRecvMstCDKeyAuthorizationRequest(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.Heartbeat:
                    OnRecvMstHeartbeat(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.DisconnectNotify:
                    OnRecvMstDisconnectNotify(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.StartupNotify:
                    OnRecvMstStartupNotify(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.ModuleLoadNotify:
                    OnRecvMstModuleLoadNotify(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.MOTDRequest:
                    OnRecvMstMOTDRequest(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.VersionRequest:
                    OnRecvMstVersionRequest(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.StatusRequest:
                    OnRecvMstStatusRequest(ParseBuffer, Sender);
                    break;

                case (uint)ConnAuthCmd.ServerInfoResponse:
                    OnRecvServerInfoResponse(ParseBuffer, Sender);
                    break;

            }
        }

        /// <summary>
        /// This method parses and handles a community authorization request
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstCommunityAuthorizationRequest(ExoParseBuffer Parser, IPEndPoint Sender)
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
        private void OnRecvMstCDKeyAuthorizationRequest(ExoParseBuffer Parser, IPEndPoint Sender)
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

            IPEndPoint ClientEndpoint = new IPEndPoint((long)ClientIP, IPAddress.NetworkToHostOrder((int)ClientPort));

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

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            //
            // Record activity for the server and request an updated player
            // count.
            //

            Server.RecordActivity();
            SendServerInfoRequest(Sender);
        }

        /// <summary>
        /// This method parses and handles a heartbeat message from a game
        /// server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstHeartbeat(ExoParseBuffer Parser, IPEndPoint Sender)
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

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);
            Server.OnHeartbeat((uint)PlayerCDKeyList.Count);

            Logger.Log("NWMasterServer.OnRecvMstHeartbeat(): Server {0} ActivePlayerCount={1}.", Sender, PlayerCDKeyList.Count);
        }

        /// <summary>
        /// This method parses and handles a player disconnect notify message
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstDisconnectNotify(ExoParseBuffer Parser, IPEndPoint Sender)
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

                if (Server == null)
                    return;

                //
                // Record the server shutdown.
                //

                Server.OnShutdownNotify();
                Logger.Log("NWMasterServer.OnRecvMstShutdownNotify(): Server {0} shut down.", Sender);
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

            Server = ServerTracker.LookupServerByAddress(Sender);

            //
            // Record activity for the server and request an updated player
            // count.
            //

            Server.RecordActivity();
            SendServerInfoRequest(Sender);
            Logger.Log("NWMasterServer.OnRecvMstDisconnectNotify()");
        }

        /// <summary>
        /// This method parses and handles a server startup notify message from
        /// a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstStartupNotify(ExoParseBuffer Parser, IPEndPoint Sender)
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
            Logger.Log("NWMasterServer.OnRecvMstStartupNotify(): Server {0} Platform={1} BuildNumber={2}.", Sender, (char)Platform, BuildNumber);
        }

        /// <summary>
        /// This method parses and handles a module load notify message from a
        /// game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstModuleLoadNotify(ExoParseBuffer Parser, IPEndPoint Sender)
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
            SendServerInfoRequest(Sender);

            Logger.Log("NWMasterServer.OnRecvMstModuleLoadNotify(): Server {0} ModuleName={1} ExpansionsMask={2}.", Sender, ModuleName, ExpansionsMask);
        }

        /// <summary>
        /// This method parses and handles a MOTD request from a game server or
        /// game client.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstMOTDRequest(ExoParseBuffer Parser, IPEndPoint Sender)
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
        private void OnRecvMstVersionRequest(ExoParseBuffer Parser, IPEndPoint Sender)
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
        private void OnRecvMstStatusRequest(ExoParseBuffer Parser, IPEndPoint Sender)
        {
            UInt16 DataPort;

            if (!Parser.ReadWORD(out DataPort))
                return;

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            //
            // Record module activity.
            //

            Server.RecordActivity();
            SendMstStatusResponse(Sender, MstStatus.MST_STATUS_ONLINE);
        }

        /// <summary>
        /// This method parses a server info response from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parser context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvServerInfoResponse(ExoParseBuffer Parser, IPEndPoint Sender)
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
            if (Reserved != 0xFC)
                return;
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
            if (!Parser.ReadSmallString(out BuildNumber))
                return;

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
            //

            NWGameServer Server = ServerTracker.LookupServerByAddress(Sender);

            Server.OnServerInfoUpdate(Info);

            Logger.Log("NWMasterServer.OnRecvServerInfoResponse(): Server {0} has {1}/{2} players ({3}).", Sender, Info.ActivePlayers, Info.MaximumPlayers,
                 Info.ModuleName);
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

            Logger.Log("NWMasterServer.SendMstCommunityAccountAuthorization(): Authorizing account {0} for server {1} with status {2}.", AccountName, Address, Status);

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

            Logger.Log("NWMasterServer.SendMstCDKeyAuthorization(): Authorizing {0} CD-Keys for server {1}.", CDKeys, Address);

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

            Logger.Log("NWMasterServer.SendMstDemandHeartbeat(): Requesting heartbeat for server {0}.", Address);

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

            Logger.Log("NWMasterServer.SendMstMOTD(): Sending MOTD to {0}.", Address);

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

            Logger.Log("NWMasterServer.SendMstVersion(): Sending version to {0}.", Address);

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

            Logger.Log("NWMasterServer.SendMstStatusResponse(): Sending status response to {0}.", Address);

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
            Builder.WriteWORD((ushort)MASTER_SERVER_PORT);

            Logger.Log("NWMasterServer.SendServerInfoRequest(): Sending server info request to {0}.", Address);

            SendRawDataToMstClient(Address, Builder);
        }

        /// <summary>
        /// This method transmits a raw datagram to a master server client,
        /// such as a game server or game client.
        /// </summary>
        /// <param name="Address">Supplies the message recipient.</param>
        /// <param name="Builder">Supplies the message body.</param>
        private void SendRawDataToMstClient(IPEndPoint Address, ExoBuildBuffer Builder)
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

                    IAsyncResult Result = ServerSocket.BeginSendTo(
                        CompositeBuffer,
                        0,
                        (int)CompositeBufferLength,
                        SocketFlags.None,
                        Address,
                        SendCompletionCallback,
                        Address);
                }
                catch (Exception e)
                {
                    Logger.Log("NWMasterServer.SendRawDataToMstClient(): Failed to send data to Mst client {0}: Exception: {1}", Address, e);
                }
            }
        }

        /// <summary>
        /// This method marks a game server as still alive (because a message
        /// was received).
        /// </summary>
        /// <param name="Sender">Supplies the game server address.</param>
        private void RecordGameServerActivity(IPEndPoint Sender)
        {
            Logger.Log("NWMasterServer.RecordGameServerActivity(): Server active: {0}", Sender);
        }

        /// <summary>
        /// This method marks a game server as shut down.
        /// </summary>
        /// <param name="Sender">Supplies the game server address.</param>
        private void RecordGameServerShutdown(IPEndPoint Sender)
        {
            Logger.Log("NWMasterServer.RecordGameServerActivity(): Server shutdown: {0}", Sender);
        }

        /// <summary>
        /// A receive buffer that can contain data for a single datagram sent
        /// by a client or game server.
        /// </summary>
        private struct RecvBuffer
        {
            public byte[] Buffer;
            public IPEndPoint Sender;
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

            ServerInfoRequest = 0x49584e42,

            //
            // Server to client requests.
            //

            ServerInfoResponse = 0x52584e42,
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
        /// Maximum datagram frame size allowed.
        /// </summary>
        private const int MAX_FRAME_SIZE = 1472;

        /// <summary>
        /// Receive buffers available to pull data into from the network.
        /// </summary>
        private RecvBuffer[] Buffers = new RecvBuffer[BUFFER_COUNT];

        /// <summary>
        /// The count of buffers that have a receive pending.
        /// </summary>
        private volatile int PendingBuffers = 0;

        /// <summary>
        /// The UDP socket to receive master server messages from (and to send
        /// replies with).
        /// </summary>
        private Socket ServerSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);

        /// <summary>
        /// The server tracker instance that retains state about known servers.
        /// </summary>
        private NWServerTracker ServerTracker = null;

        /// <summary>
        /// The server bind address.
        /// </summary>
        private string BindAddress = ServerSettings.Default.BindAddress;

        /// <summary>
        /// The server game build number.
        /// </summary>
        private string BuildNumber = ServerSettings.Default.BuildNumber;

        /// <summary>
        /// The server message of the day.
        /// </summary>
        private string MOTD = ServerSettings.Default.MOTD;

        /// <summary>
        /// Return the server tracker object for external users.
        /// </summary>
        public NWServerTracker Tracker { get { return ServerTracker;  } }
    }
}

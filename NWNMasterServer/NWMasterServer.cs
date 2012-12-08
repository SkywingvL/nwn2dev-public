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
            try
            {
                bool IsShutdown = false;

                //
                // Bind the socket and initiate the initial batch of overlapped
                // receives.
                //

                BindSocket();

                for (int i = 0; i < BUFFER_COUNT; i += 1)
                {
                    Buffers[i].Buffer = new byte[MAX_FRAME_SIZE];
                    Buffers[i].Sender = new IPEndPoint(0, 0);
                    InitiateReceive(Buffers[i]);
                }

                Logger.Log("NWMasterServer.Run(): Master server initialized.");

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
                        ServerSocket.Shutdown(SocketShutdown.Receive);
                        IsShutdown = true;
                    }

                    if (PendingBuffers == 0)
                        break;
                }

                ServerSocket.Close(15);
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
            IPEndPoint LocalEndPoint = new IPEndPoint(IPAddress.Any, MASTER_SERVER_PORT);

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
            // from being initiated.
            //

            if (QuitRequested)
                return;

            try
            {
                PendingBuffers += 1;
                EndPoint RecvEndPoint = (EndPoint)Buffer.Sender;
                ServerSocket.BeginReceiveFrom(
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

            PendingBuffers -= 1;

            try
            {
                if (RecvLen > 0)
                {
                    OnRecvDatagram(Buffer.Buffer, RecvLen, Buffer.Sender);
                }
            }
            catch (Exception e)
            {
                Logger.Log("NWMasterServer.RecvCompletionCallback(): Exception: {0}", e);
            }

            InitiateReceive(Buffer);
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

                Cmd = ((uint)Buffer[0] <<  0)  |
                      ((uint)Buffer[1] <<  8)  |
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

                case (uint)MstCmd.ShutdownNotify:
                    OnRecvMstShutdownNotify(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.StartupNotify:
                    OnRecvMstStartupNotify(ParseBuffer, Sender);
                    break;

                case (uint)MstCmd.ModuleLoadNotify:
                    OnRecvMstModuleLoadNotify(ParseBuffer, Sender);
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

            RecordGameServerActivity(Sender);
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

            while (Length-- != 0)
            {
                UInt16 HashLength;

                if (!Parser.ReadSmallString(out CDKeyHash.PublicCDKey, 16))
                    return;
                if (!Parser.ReadWORD(out HashLength))
                    return;
                if ((CDKeyHash.CDKeyHash = Parser.ReadBytes(HashLength)) == null)
                    return;

                CDKeyHashes.Add(CDKeyHash);
            }

            if (!Parser.ReadSmallString(out AccountName, 16))
                return;

            RecordGameServerActivity(Sender);
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

            RecordGameServerActivity(Sender);
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

            if (!Parser.ReadWORD(out DataPort))
                return;
            if (!Parser.ReadWORD(out EntryCount))
                return;
            if (EntryCount != 1)
                return;
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

            RecordGameServerActivity(Sender);
        }

        /// <summary>
        /// This method parses and handles a server shutdown notify message
        /// from a game server.
        /// </summary>
        /// <param name="Parser">Supplies the message parse context.</param>
        /// <param name="Sender">Supplies the game server address.</param>
        private void OnRecvMstShutdownNotify(ExoParseBuffer Parser, IPEndPoint Sender)
        {
            UInt16 DataPort;

            if (!Parser.ReadWORD(out DataPort))
                return;

            RecordGameServerShutdown(Sender);
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

            RecordGameServerActivity(Sender);
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

            RecordGameServerActivity(Sender);
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
            CommunityAuthorizationRequest = 0x41504d42, // BMPA
            CDKeyAuthorizationRequest = 0x55414d42, // BMAU
            Heartbeat = 0x42484d42, // BMHB
            DisconnectNotify = 0x43444d42, // BMDC
            ShutdownNotify = 0x54534d42, // BMST
            StartupNotify = 0x55534d42, // BMSU
            ModuleLoadNotify = 0x4f4d4d42, // BMMO
        }

        /// <summary>
        /// CD-Key verification information.
        /// </summary>
        private struct CDKeyInfo
        {
            public string PublicCDKey;
            public byte[] CDKeyHash;
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
    }
}

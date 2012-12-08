/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    ServiceController.cs

Abstract:

    This module contains the logic for interfacing with the service control
    manager.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceProcess;
using System.Threading;

namespace NWNMasterServer
{
    /// <summary>
    /// This class manages the lifetime and SCM interactions for the service.
    /// </summary>
    internal class ServiceController : ServiceBase
    {

        /// <summary>
        /// Connect to the SCM and start the service control dispatcher.
        /// </summary>
        /// <param name="ServiceName">Supplies the name of the service.</param>
        public static void ExecuteService(string ServiceName)
        {
            ServiceController ServiceObject = new ServiceController();

            ServiceObject.CanStop = true;
            ServiceObject.ServiceName = ServiceName;

            ServiceBase.Run(ServiceObject);
        }

        /// <summary>
        /// Execute the server program.
        /// </summary>
        /// <param name="args">Supplies arguments from the SCM.</param>
        protected override void OnStart(string[] args)
        {
            //
            // Spin up the server object.  Once the constructor has finished,
            // the object is ready to receive stop requests at any time.
            //

            try
            {
                NWMasterServer ServerObject = new NWMasterServer(this);

                Thread.MemoryBarrier();

                Server = ServerObject;

                if (StopRequested)
                    Server.Stop();

                Server.Run();
            }
            catch (Exception e)
            {
                Logger.Log("ServiceController.OnStart(): Exception: {0}", e);
            }
        }

        protected override void OnStop()
        {
            //
            // Mark stop requested (in case stop is requested before the
            // service has completed initialization).
            //

            Logger.Log("ServiceController.OnStop(): Service stop requested.");

            StopRequested = true;

            //
            // If the server object exists, then inform it that it should begin
            // to shut down.
            //

            NWMasterServer ServerObject = Server;

            if (ServerObject != null)
                ServerObject.Stop();
        }

        /// <summary>
        /// The associated server instance.
        /// </summary>
        private NWMasterServer Server;

        /// <summary>
        /// True if a stop request was received.
        /// </summary>
        volatile bool StopRequested;

    }
}

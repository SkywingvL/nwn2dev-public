/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Program.cs

Abstract:

    This module houses the main entry point of the server application.  It is
    responsible for parsing initial arguments and transferring control to the
    server object proper.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using NWN.Parsers;

namespace NWNMasterServer
{
    /// <summary>
    /// This is the program entry point class.
    /// </summary>
    class Program
    {
        /// <summary>
        /// Execute the program.
        /// </summary>
        /// <param name="args">Supplies arguments.  The only valid argument is
        /// the -service argument, which directs the program to attempt to
        /// connect to the SCM for service-mode startup; otherwise, the program
        /// executes as a console application.</param>
        static void Main(string[] args)
        {
            //
            // Scan and process arguments.
            //

            for (int i = 0; i < args.Length; i += 1)
            {
                string Arg = args[i];

                if (Arg == "-service")
                {
                    //
                    // Execute the server in service mode.  This method returns
                    // once service stop has been requested and the server has
                    // shut down.
                    //

                    string ServiceName = "NWMasterServerSvc";

                    if ((i + 1) < args.Length)
                    {
                        i += 1;
                        ServiceName = args[i];
                    }

                    ServiceController.ExecuteService(ServiceName);
                    return;
                }
            }

            //
            // Execute the server as an interactive, console application.
            //

            NWMasterServer ServerObject = new NWMasterServer(null);

            ServerObject.Run();
        }
    }
}

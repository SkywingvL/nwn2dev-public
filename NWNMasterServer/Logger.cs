/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    Logger.cs

Abstract:

    This module houses diagnostics logging support.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace NWNMasterServer
{
    /// <summary>
    /// This class manages logging diagnostic events.
    /// </summary>
    internal static class Logger
    {

        /// <summary>
        /// Log a diagnostics message.
        /// </summary>
        /// <param name="Format">Supplies the format string.</param>
        /// <param name="Inserts">Supplies optional format inserts.</param>
        public static void Log(string Format, params object[] Inserts)
        {
            DateTime LogTime = DateTime.UtcNow;
            string Formatted = String.Format(
                "[{0:D4}-{1:D2}-{2:D2} {3:D2}:{4:D2}:{5:D2}]: {6}",
                LogTime.Year,
                LogTime.Month,
                LogTime.Day,
                LogTime.Hour,
                LogTime.Minute,
                LogTime.Second,
                String.Format(Format, Inserts));

            Console.WriteLine(Formatted);

            if (LogFile != null)
            {
                LogFile.WriteLine(Formatted);
                LogFile.Flush();
            }
        }

        /// <summary>
        /// Set a global log file to log to.
        /// </summary>
        /// <param name="LogFileName">Supplies the log file name.</param>
        public static void OpenLogFile(string LogFileName)
        {
            try
            {
                StreamWriter Writer = new StreamWriter(LogFileName, true, Encoding.UTF8);

                Writer.AutoFlush = false;

                LogFile = Writer;
            }
            catch (Exception)
            {
                return;
            }
        }

        private static StreamWriter LogFile = null;

    }
}

/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

    DatabaseSettings.cs

Abstract:

    This module houses logic to support database config file settings.

--*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Configuration;

namespace NWN
{
    /// <summary>
    /// This class define database configuration settings.
    /// </summary>
    public class DatabaseSettings : ConfigurationSection
    {
        /// <summary>
        /// The MySQL connection string.
        /// </summary>
        [ConfigurationProperty("ConnectionString", IsRequired = true)]
        public string ConnectionString
        {
            get { return (string)this["ConnectionString"]; }
            set { this["ConnectionString"] = value; }
        }
    }
}

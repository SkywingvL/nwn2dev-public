﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.18010
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace NWNMasterServer {
    
    
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("Microsoft.VisualStudio.Editors.SettingsDesigner.SettingsSingleFileGenerator", "10.0.0.0")]
    internal sealed partial class ServerSettings : global::System.Configuration.ApplicationSettingsBase {
        
        private static ServerSettings defaultInstance = ((ServerSettings)(global::System.Configuration.ApplicationSettingsBase.Synchronized(new ServerSettings())));
        
        public static ServerSettings Default {
            get {
                return defaultInstance;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("0.0.0.0")]
        public string BindAddress {
            get {
                return ((string)(this["BindAddress"]));
            }
            set {
                this["BindAddress"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("1765")]
        public string BuildNumber {
            get {
                return ((string)(this["BuildNumber"]));
            }
            set {
                this["BuildNumber"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("Welcome to Neverwinter Nights 2!\r\n \r\nPlease join us at the Official Neverwinter N" +
            "ights 2 forums at:\r\nhttp://nwn2forums.bioware.com\r\n \r\nHaving trouble creating or" +
            " joining games?  Please visit Atari\'s Customer Support at:\r\nhttp://www.atarisupp" +
            "ort.com")]
        public string MOTD {
            get {
                return ((string)(this["MOTD"]));
            }
            set {
                this["MOTD"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("")]
        public string LogFileName {
            get {
                return ((string)(this["LogFileName"]));
            }
            set {
                this["LogFileName"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("")]
        public string DatabaseConnectionString {
            get {
                return ((string)(this["DatabaseConnectionString"]));
            }
            set {
                this["DatabaseConnectionString"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("Normal")]
        public string LogLevel {
            get {
                return ((string)(this["LogLevel"]));
            }
            set {
                this["LogLevel"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("2")]
        public uint ProductID {
            get {
                return ((uint)(this["ProductID"]));
            }
            set {
                this["ProductID"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("6121")]
        public uint MasterServerPort {
            get {
                return ((uint)(this["MasterServerPort"]));
            }
            set {
                this["MasterServerPort"] = value;
            }
        }
        
        [global::System.Configuration.UserScopedSettingAttribute()]
        [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
        [global::System.Configuration.DefaultSettingValueAttribute("NWN2")]
        public string GameMode {
            get {
                return ((string)(this["GameMode"]));
            }
            set {
                this["GameMode"] = value;
            }
        }
    }
}

/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	SettingsManager.cs

Abstract:

    This module houses the settings manager, which is responsible for loading
    and saving configuration data.

--*/

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using OEIShared.Utils;
using OEIShared.IO;
using OEIShared.IO.GFF;

namespace NWN2ToolsetCompilerPlugin
{
    public class SettingsManager
    {

        const int DEFAULT_COMPILER_VERSION = 169; // NWN1 v1.69 drop

        //
        // Construct a new instance.
        //

        public SettingsManager()
        {
            CompilerVersion = DEFAULT_COMPILER_VERSION;
            EnableExtensions = false;
            EnableDebugSymbols = false;

            NeedSettingsLoad = true;
        }

        //
        // Compiler version (e.g. 169 for compatible mode).
        //

        public int CompilerVersion { get; set; }

        //
        // Extension enable flag.
        //

        public bool EnableExtensions { get; set; }

        //
        // Enable debug symbols flag.
        //

        public bool EnableDebugSymbols { get; set; }

        //
        // If true, the next LoadSettings call goes through to disk.  Otherwise
        // cached value are used.
        //

        public bool NeedSettingsLoad { get; set; }

        //
        // Save settings to the module.
        //

        public void SaveSettings()
        {
            ResourceManager ResMan = ResourceManager.Instance;
            IResourceEntry Entry;
            GFFFile File;
            Stream ResStream;

            Entry = ResMan.GetEntry(new OEIResRef("CompilerSettings"), (ushort)ResTypes.ResGFF);

            //
            // If the settings file did not exist yet, create it in the module proper.
            //

            if (Entry == null || Entry is MissingResourceEntry)
            {
                Entry = NWN2Toolset.NWN2ToolsetMainForm.App.Module.Repository.CreateResource(
                    new OEIResRef("CompilerSettings"),
                    (ushort)ResTypes.ResGFF);
            }

            ResStream = Entry.GetStream(true);

            try
            {
                File = new GFFFile();

                GFFStruct Settings = new GFFStruct();
                GFFStructField SettingsField = new GFFStructField();
                GFFIntField CompilerVersionField = new GFFIntField();
                GFFByteField EnableExtensionsField = new GFFByteField();
                GFFByteField EnableDebugSymbolsField = new GFFByteField();

                CompilerVersionField.ValueInt = CompilerVersion;
                CompilerVersionField.StringLabel = "CompilerVersion";
                EnableExtensionsField.ValueByte = (Byte)(EnableExtensions == true ? 1 : 0);
                EnableExtensionsField.StringLabel = "EnableExtensions";
                EnableDebugSymbolsField.ValueByte = (Byte)(EnableDebugSymbols == true ? 1 : 0);
                EnableDebugSymbolsField.StringLabel = "EnableDbgSymbols";

                Settings.Fields.Add(CompilerVersionField);
                Settings.Fields.Add(EnableExtensionsField);
                Settings.Fields.Add(EnableDebugSymbolsField);

                SettingsField.ValueStruct = Settings;
                SettingsField.StringLabel = "Settings";
                File.TopLevelStruct.Fields.Add(SettingsField);

                File.Save(ResStream);
            }
            finally
            {
                Entry.Release();
            }
        }

        //
        // Load settings from the module.
        //

        public void LoadSettings()
        {
            if (NeedSettingsLoad == false)
                return;

            ResourceManager ResMan = ResourceManager.Instance;
            IResourceEntry Entry;
            GFFFile File;
            Stream ResStream;

            Entry = ResMan.GetEntry(new OEIResRef("CompilerSettings"), (ushort)ResTypes.ResGFF);

            if (Entry == null || Entry is MissingResourceEntry)
                return;

            ResStream = Entry.GetStream(false);

            try
            {
                File = new GFFFile(ResStream);

                GFFStruct Settings = File.TopLevelStruct.GetStructSafe("Settings");

                CompilerVersion = Settings.GetIntSafe("CompilerVersion", DEFAULT_COMPILER_VERSION);
                EnableExtensions = Settings.GetByteSafe("EnableExtensions", 0) != 0 ? true : false;
                EnableDebugSymbols = Settings.GetByteSafe("EnableDbgSymbols", 0) != 0 ? true : false;
            }
            finally
            {
                Entry.Release();
            }

            NeedSettingsLoad = false;
        }

        ///////////////////////////////////////////////////////////////////////


        ///////////////////////////////////////////////////////////////////////

    }
}

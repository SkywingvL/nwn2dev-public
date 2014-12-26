/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ScriptCompilerHook.cs

Abstract:

    This module houses the script compiler hook, which is responsible for
    calling the replacement script compiler on compilation requests.

--*/

using System;
using System.Collections.Generic;
using System.Text;
using OEIShared.Utils;

namespace NWN2ToolsetCompilerPlugin
{
    public class ScriptCompilerHook : IScriptCompiler,
                                      IDisposable
    {

        //
        // Construct a new instance with the given stock compiler that is to be
        // attached.
        //

        public ScriptCompilerHook(IScriptCompiler StandardCompiler,
            ResourceAccessor ResourceAccessor,
            SettingsManager SettingsMgr)
        {
            m_Disposed = false;
            m_StandardCompiler = StandardCompiler;
            m_ResourceAccessor = ResourceAccessor;
            m_SettingsManager = SettingsMgr;
            m_NscCompiler = new NscCompiler(ResourceAccessor, SettingsMgr);
        }

        //
        // Disposer logic.
        //

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool Disposing)
        {
            if (m_Disposed)
                return;

            if (m_StandardCompiler != null)
            {
                m_StandardCompiler.Dispose();
                m_StandardCompiler = null;
            }

            if (m_NscCompiler != null)
            {
                m_NscCompiler.Dispose();
                m_NscCompiler = null;
            }
        }

        //
        // OEIShared.Utils.IScriptCompiler.
        //

        public virtual bool GenerateDebugInfo
        {
            get
            {
                return m_StandardCompiler.GenerateDebugInfo;
            }
            set
            {
                m_StandardCompiler.GenerateDebugInfo = value;
            }
        }

        //
        // Compile a source file.
        //

        public virtual string CompileFile(string Name, string OutputDirectory)
        {
            //
            // Make sure that the resource index is up to date, then attempt
            // the compilation.
            //

            m_ResourceAccessor.EnsureIndexValid();

            try
            {
                string Warnings;
                string Errors;

                m_SettingsManager.LoadSettings();

                Errors = m_NscCompiler.CompileScript(
                    Name,
                    OutputDirectory,
                    m_StandardCompiler.GenerateDebugInfo,
                    out Warnings);

                if (Warnings.Length != 0)
                    ShowCompilerMessages(Name, Warnings);
                else if (Errors.Length != 0)
                    ShowCompilerMessages(Name, Errors);

                //
                // If we have errors, find the first error and make it the
                // one reported as the output control only shows one line
                // anyway.  The rest are dumped to the verify window.
                //

                if (Errors.Length != 0)
                {
                    foreach (string Msg in Errors.Split('\n'))
                    {
                        if (Msg.IndexOf("Error: ") == -1)
                            continue;

                        Errors = Msg;
                        Errors += " (see verify window for full context)";
                        break;
                    }
                }
                else
                {
                    //
                    // GUI scripts must compile with the stock compiler, as
                    // they are parsed at runtime.  Not all of the quirks of
                    // the stock compiler throw errors with the new compiler,
                    // so verify that these are no errors with the stock
                    // compiler (too).  Since this is a performance hit and is
                    // only necessary for GUI scripts, don't do it for other
                    // ones.
                    //

                    if (m_NscCompiler.LastCompiledScriptNeedsRuntimeCompilation(Name) &&
                        !m_SettingsManager.EnableExtensions)
                    {
                        System.IO.Directory.CreateDirectory(OutputDirectory + "\\__tempscript__");

                        Errors = m_StandardCompiler.CompileFile(Name, OutputDirectory + "\\__tempscript__");

                        if (Errors.Length != 0)
                        {
                            Errors += "Error: Script " + Name + " requires runtime compilation and errors were detected with the standard compiler.\n";
                            ShowCompilerMessages(Name, Errors);
                        }

                        System.IO.File.Delete(OutputDirectory + "\\__tempscript__\\" + Name + ".ncs");
                        System.IO.File.Delete(OutputDirectory + "\\__tempscript__\\" + Name + ".ndb");
                        System.IO.Directory.Delete(OutputDirectory + "\\__tempscript__", false);
                    }
                }

                return Errors;
            }
            catch (Exception e)
            {
                //
                // If calling the native compiler failed, i.e. the support DLL
                // wasn't found, then just fall back to the built-in compiler.
                //

                PluginUI.AddVerifyResult(
                    NWN2Toolset.Plugins.NWN2VerifyOutputType.Warning,
                    "Advanced script compiler: NscCompileScript failed to execute, falling back to standard script compiler - exception: " + e.ToString());

                return m_StandardCompiler.CompileFile(Name, OutputDirectory);
            }
        }

        public virtual NWN2ScriptParameterCollection GetParametersForScript(string Name,
            string OutputDirectory)
        {
            try
            {
                m_SettingsManager.LoadSettings();

                //
                // Make sure the resource index is valid in case, for example,
                // we have opened the dialog editor before opening the first
                // script.  Otherwise the user would be unable to actually set
                // parameters on scripts in the dialog editor until they had
                // opened a script file.
                //

                m_ResourceAccessor.EnsureIndexValid();

                return m_NscCompiler.GetScriptEntryPointParameters(Name, OutputDirectory);
            }
            catch
            {
                return null;
            }
        }

        //
        // Retrieve (or set) the internal stock compiler instance.
        //

        public IScriptCompiler Compiler
        {
            get
            {
                return m_StandardCompiler;
            }
            set
            {
                m_StandardCompiler = value;
            }
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Display compilation messages.
        //

        void ShowCompilerMessages(string Script, string Message)
        {
            bool First = false;

            if (Message.IndexOf('\n') != 0)
                Message += "\n";

            PluginUI.AddVerifyResult(
                NWN2Toolset.Plugins.NWN2VerifyOutputType.Information,
                "Compiler diagnostics for script " + Script + ":");

            foreach (string Msg in Message.Split('\n'))
            {
                bool NonWhitespaceFound = false;
                NWN2Toolset.Plugins.NWN2VerifyOutputType MessageType;

                for (int i = 0; i < Msg.Length; i += 1)
                {
                    if (!Char.IsWhiteSpace(Msg, i))
                    {
                        NonWhitespaceFound = true;
                        break;
                    }
                }

                if (!NonWhitespaceFound)
                    continue;

                if (Msg.IndexOf("Warning: ") != -1)
                {
                    MessageType = NWN2Toolset.Plugins.NWN2VerifyOutputType.Warning;
                }
                else
                {
                    MessageType = NWN2Toolset.Plugins.NWN2VerifyOutputType.Error;
                }

                PluginUI.AddVerifyResult(
                    MessageType,
                    Msg,
                    First);

                First = false;
            }
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Define whether we've disposed already.
        //

        private bool m_Disposed;

        //
        // Define the standard compiler implementation, which is used to handle
        // requests to pull script prototypes for dialogs.
        //

        private IScriptCompiler m_StandardCompiler;

        //
        // Define the resource accessor link, used to ensure that the resource
        // system is indexed before a compilation is requested.
        //

        private ResourceAccessor m_ResourceAccessor;

        //
        // Define the native NWScript compiler that is invoked for script
        // source files in lieu of the stock compiler.
        //

        private NscCompiler m_NscCompiler;

        //
        // Define the settings manager instance that is used to read
        // configuration data.
        //

        private SettingsManager m_SettingsManager;
    }
}

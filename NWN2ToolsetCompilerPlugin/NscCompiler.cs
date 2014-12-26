/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NscCompiler.cs

Abstract:

    This module houses the interop code necessary to invoke the native NWScript
    compiler from managed code, backened against the toolset resource system
    through the ResourceAccessor class.

--*/

using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using OEIShared.Utils;

namespace NWN2ToolsetCompilerPlugin
{
    public class NscCompiler
    {
        //
        // Construct a new compiler instance, backed against a given resource
        // accessor.
        //
        // N.B.  The native compiler object is allocated on demand the first
        //       time the user compiles a script.
        //

        public NscCompiler(ResourceAccessor ResourceAccessor,
            SettingsManager SettingsMgr)
        {
            m_Disposed = false;
            m_Compiler = IntPtr.Zero;
            m_ResourceAccessor = ResourceAccessor;
            m_SettingsManager = SettingsMgr;
        }

        //
        // Compile a script using the specified resource system.  Any errors
        // are returned; if the empty string is returned, the compilation
        // succeeded (but potentially with warnings).
        //

        public string CompileScript(string Name, string OutputDirectory, bool GenerateDebugInfo, out string WarningOutput)
        {
            bool Status;
            string CompilerDiagnosticOutput;
            NSC_COMPILER_DISPATCH_TABLE DispatchTable = CreateNscCompilerDispatchTable();

            WarningOutput = "";

            if (m_Compiling)
                return "Compiler is busy.";

            //
            // Setup the dispatch table and invoke the script compiler.
            //

            if (m_Compiler == IntPtr.Zero)
            {
                m_Compiler = NscCreateCompiler(true);

                if (m_Compiler == IntPtr.Zero)
                {
                    return "Failed to instantiate NscCompiler.";
                }
            }

            m_CompilerDiagnosticsLog = null;

            m_Compiling = true;

            try
            {
                Status = NscCompileScriptExternal(
                    m_Compiler,
                    Name,
                    OutputDirectory,
                    m_ResourceAccessor.IsIndexInvalidationPending(),
                    GenerateDebugInfo || m_SettingsManager.EnableDebugSymbols,
                    true,
                    true,
                    m_SettingsManager.CompilerVersion, // Compiler version (NWN1 v1.69 drop)
                    ref DispatchTable);
            }
            catch
            {
                m_Compiling = false;
                throw;
            }

            m_Compiling = false;

            m_ResourceAccessor.AcknowledgeIndexInvalidation();

            CompilerDiagnosticOutput = m_CompilerDiagnosticsLog;

            if (CompilerDiagnosticOutput == null)
                CompilerDiagnosticOutput = "";

            m_CompilerDiagnosticsLog = null;

            //
            // If we succeeded but have diagnostics still, record them as
            // warning output.  Otherwise consider the output error output.
            //

            if (Status)
            {
                WarningOutput = CompilerDiagnosticOutput;
                return "";
            }
            else
            {
                WarningOutput = "";

                if (CompilerDiagnosticOutput == "")
                    CompilerDiagnosticOutput = "ERROR: INTERNAL COMPILER ERROR, NO DIAGNOSTICS RETURNED";

                return CompilerDiagnosticOutput;
            }
        }

        //
        // Determine whether the last compiled script is likely to need runtime
        // compilation (and thus must compile with the stock script compiler).
        //

        public bool LastCompiledScriptNeedsRuntimeCompilation(string ScriptName)
        {
            string Entrypoint;

            if (m_Compiler == IntPtr.Zero)
                return false;

            //
            // All GUI scripts are compiled by the stock server at runtime, so
            // always flag them.
            //

            if (ScriptName.StartsWith("gui_"))
                return true;

            if (m_Compiling)
                return false;

            m_Compiling = true;

            try
            {
                Entrypoint = Marshal.PtrToStringAnsi(NscGetEntrypointSymbolName(m_Compiler));
            }
            catch
            {
                m_Compiling = false;
                throw;
            }

            m_Compiling = false;

            //
            // All StartingConditional scripts are generally dialog nodes and
            // also need to be compiled at runtime.  Also, a few other scripts
            // do take parameters but might be "main" scripts -- try and detect
            // these by checking whether they had any parameters.  This isn't
            // quite foolproof (as the script could have no parameters but be
            // called with several), but it's about as close as we'll get.
            //

            if (Entrypoint == null)
                return false;
            else if (Entrypoint == "StartingConditional")
                return true;
            else
            {
                m_Compiling = true;

                try
                {
                    if (NscGetFunctionParameterCount(m_Compiler, Entrypoint) != 0)
                    {
                        m_Compiling = false;
                        return true;
                    }
                }
                catch
                {
                    m_Compiling = false;
                    throw;
                }

                m_Compiling = false;
            }

            return false;
        }

        //
        // Return parameter information for a script entrypoint.
        //

        public NWN2ScriptParameterCollection GetScriptEntryPointParameters(string ScriptName, string OutputDirectory)
        {
            string Entrypoint;
            string Warnings;
            NWN2ScriptParameterCollection Parameters;
            NWN2ScriptParameter Parameter;
            int ParameterCount;

            //
            // First, compile the script and check that we succeeded at that.
            //

            if (!String.IsNullOrEmpty(CompileScript(ScriptName, OutputDirectory, false, out Warnings)))
                return null;

            if (m_Compiler == IntPtr.Zero)
                return null;

            Parameters = new NWN2ScriptParameterCollection();

            if (m_Compiling)
                return null;

            m_Compiling = true;

            try
            {
                Entrypoint = Marshal.PtrToStringAnsi(NscGetEntrypointSymbolName(m_Compiler));
                ParameterCount = NscGetFunctionParameterCount(m_Compiler, Entrypoint);

                for (int ParamIdx = 0; ParamIdx < ParameterCount; ParamIdx += 1)
                {
                    Parameter = new NWN2ScriptParameter();

                    switch ((NWACTION_TYPE) NscGetFunctionParameterType(m_Compiler, Entrypoint, ParamIdx))
                    {

                        case NWACTION_TYPE.ACTIONTYPE_INT:
                            Parameter.ParameterType = NWN2ScriptParameterType.Int;
                            break;

                        case NWACTION_TYPE.ACTIONTYPE_FLOAT:
                            Parameter.ParameterType = NWN2ScriptParameterType.Float;
                            break;

                        case NWACTION_TYPE.ACTIONTYPE_STRING:
                            Parameter.ParameterType = NWN2ScriptParameterType.String;
                            break;

                        default:
                            throw new ApplicationException("Invalid parameter type for script entrypoint.");

                    }

                    Parameters.Add(Parameter);
                }
            }
            catch
            {
                m_Compiling = false;
                throw;
            }

            m_Compiling = false;

            return Parameters;
        }


        ///////////////////////////////////////////////////////////////////////

        //
        // Define the NscCompiler dispatch table to pass to the native compiler
        // DLL.
        //

        public enum NWACTION_TYPE : int
        {
            ACTIONTYPE_VOID = 0,
            ACTIONTYPE_INT,
            ACTIONTYPE_FLOAT,
            ACTIONTYPE_STRING,
            ACTIONTYPE_OBJECT,
            ACTIONTYPE_VECTOR,
            ACTIONTYPE_ACTION,
            ACTIONTYPE_EFFECT,
            ACTIONTYPE_EVENT,
            ACTIONTYPE_LOCATION,
            ACTIONTYPE_TALENT,
            ACTIONTYPE_ITEMPROPERTY,

            ACTIONTYPE_ENGINE_0 = ACTIONTYPE_EFFECT,
            ACTIONTYPE_ENGINE_1,
            ACTIONTYPE_ENGINE_2,
            ACTIONTYPE_ENGINE_3,
            ACTIONTYPE_ENGINE_4,
            ACTIONTYPE_ENGINE_5,
            ACTIONTYPE_ENGINE_6,
            ACTIONTYPE_ENGINE_7,
            ACTIONTYPE_ENGINE_8,
            ACTIONTYPE_ENGINE_9,

            LASTACTIONTYPE
        };

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate ulong ResOpenFileProc(ref ResRef32 ResRef, ushort ResType, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate ulong ResOpenFileByIndexProc(ulong FileIndex, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate bool ResCloseFileProc(ulong FileHandle, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate bool ResReadEncapsulatedFileProc(ulong File, IntPtr Offset, IntPtr BytesToRead, out IntPtr BytesRead, [MarshalAs(UnmanagedType.LPArray, SizeParamIndex=2), Out] byte[] Buffer, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate IntPtr ResGetEncapsulatedFileSizeProc(ulong File, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate ushort ResGetEncapsulatedFileTypeProc(ulong FileHandle, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate bool ResGetEncapsulatedFileEntryProc(ulong FileIndex, out ResRef32 ResRef, out ushort ResType, IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate ulong ResGetEncapsulatedFileCountProc(IntPtr Context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi, SetLastError = true)]
        public delegate void NscCompilerDiagnosticOutputProc(string DiagnosticMessage, IntPtr Context);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        private struct NSC_COMPILER_DISPATCH_TABLE
        {
            public uint Size;
            public IntPtr Context;
            public ResOpenFileProc ResOpenFile;
            public ResOpenFileByIndexProc ResOpenFileByIndex;
            public ResCloseFileProc ResCloseFile;
            public ResReadEncapsulatedFileProc ResReadEncapsulatedFile;
            public ResGetEncapsulatedFileSizeProc ResGetEncapsulatedFileSize;
            public ResGetEncapsulatedFileTypeProc ResGetEncapsulatedFileType;
            public ResGetEncapsulatedFileEntryProc ResGetEncapsulatedFileEntry;
            public ResGetEncapsulatedFileCountProc ResGetEncapsulatedFileCount;
            public NscCompilerDiagnosticOutputProc NscCompilerDiagnosticOutput;
        };

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr NscCreateCompiler(
            bool EnableExtensions);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern bool NscCompileScriptExternal(
            IntPtr Compiler,
            string ScriptFileName,
            string OutputDirectory,
            bool FlushResources,
            bool GenerateDebugInfo,
            bool Optimize,
            bool IgnoreIncludes,
            int CompilerVersion,
            ref NSC_COMPILER_DISPATCH_TABLE DispatchTable);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern bool NscDeleteCompiler(
            IntPtr Compiler);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr NscGetEntrypointSymbolName(
            IntPtr Compiler);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern int NscGetFunctionParameterCount(
            IntPtr Compiler,
            string FunctionName);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern IntPtr NscGetCompilerBuildDate(
            IntPtr Compiler);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern int NscGetFunctionParameterType(
            IntPtr Compiler,
            string FunctionName,
            int ParameterIndex);

        [DllImport("NWNScriptCompilerDll.ndl", CallingConvention = CallingConvention.StdCall, CharSet = CharSet.Ansi, ExactSpelling = true, SetLastError = true)]
        private static extern int NscGetFunctionReturnType(
            IntPtr Compiler,
            string FunctionName);

        ///////////////////////////////////////////////////////////////////////

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

            if (m_Compiler != IntPtr.Zero)
            {
                NscDeleteCompiler(m_Compiler);
                m_Compiler = IntPtr.Zero;
            }
        }

        //
        // General utilities.
        //

        //
        // Convert a ResRef32 to a string.
        //

        private string ResRef32ToString(ResRef32 ResRef)
        {
            return System.Text.Encoding.Default.GetString(ResRef.RefStr);
        }

        //
        // Convert a string to a ResRef32.
        //

        private ResRef32 StringToResRef32(string Str)
        {
            ResRef32 ResRef = new ResRef32();
            byte[] StrBytes = System.Text.Encoding.Default.GetBytes(Str);
            int i;

            ResRef.RefStr = new byte[32];

            for (i = 0; i < StrBytes.Length && i < 32; i += 1)
                ResRef.RefStr[i] = StrBytes[i];

            for (; i < 32; i += 1)
                ResRef.RefStr[i] = 0;

            return ResRef;
        }

        private NSC_COMPILER_DISPATCH_TABLE CreateNscCompilerDispatchTable()
        {
            NSC_COMPILER_DISPATCH_TABLE DispatchTable = new NSC_COMPILER_DISPATCH_TABLE();

            DispatchTable.Size = (uint)Marshal.SizeOf(DispatchTable);
            DispatchTable.Context = IntPtr.Zero;
            DispatchTable.ResOpenFile = this.OpenFile;
            DispatchTable.ResOpenFileByIndex = this.OpenFileByIndex;
            DispatchTable.ResCloseFile = this.CloseFile;
            DispatchTable.ResReadEncapsulatedFile = this.ReadEncapsulatedFile;
            DispatchTable.ResGetEncapsulatedFileSize = this.GetEncapsulatedFileSize;
            DispatchTable.ResGetEncapsulatedFileType = this.GetEncapsulatedFileType;
            DispatchTable.ResGetEncapsulatedFileEntry = this.GetEncapsulatedFileEntry;
            DispatchTable.ResGetEncapsulatedFileCount = this.GetEncapsulatedFileCount;
            DispatchTable.NscCompilerDiagnosticOutput = this.CompilerDiagnosticOutput;

            return DispatchTable;
        }

        //
        // Define thunks to call the resource accessor (invoked by native code
        // via reverse P/Invoke).
        //


        //
        // Open an encapsulated file by resref.
        //

        private ulong OpenFile(ref ResRef32 ResRef, ushort ResType, IntPtr Context)
        {
            return m_ResourceAccessor.OpenFile(ResRef32ToString(ResRef), ResType);
        }

        //
        // Open an encapsulated file by file index.
        //

        private ulong OpenFileByIndex(ulong FileIndex, IntPtr Context)
        {
            return m_ResourceAccessor.OpenFileByIndex(FileIndex);
        }

        //
        // Close an encapsulated file.
        //

        private bool CloseFile(ulong FileHandle, IntPtr Context)
        {
            return m_ResourceAccessor.CloseFile(FileHandle);
        }

        //
        // Read an encapsulated file by handle.  The routine is optimized to
        // operate for sequential file reads.
        //

        private bool ReadEncapsulatedFile(ulong File, IntPtr Offset, IntPtr BytesToRead, out IntPtr BytesRead, [Out] byte[] Buffer, IntPtr Context)
        {
            return m_ResourceAccessor.ReadEncapsulatedFile(File, Offset, BytesToRead, out BytesRead, Buffer);
        }

        //
        // Return the size of a file.
        //

        private IntPtr GetEncapsulatedFileSize(ulong File, IntPtr Context)
        {
            return m_ResourceAccessor.GetEncapsulatedFileSize(File);
        }

        //
        // Return the resource type of a file.
        //

        private ushort GetEncapsulatedFileType(ulong File, IntPtr Context)
        {
            return m_ResourceAccessor.GetEncapsulatedFileType(File);
        }

        //
        // Iterate through resources in this resource accessor.  The routine
        // return false on failure.
        //

        private bool GetEncapsulatedFileEntry(ulong FileIndex, out ResRef32 ResRef, out ushort ResType, IntPtr Context)
        {
            string ResRefStr;

            if (!m_ResourceAccessor.GetEncapsulatedFileEntry(FileIndex, out ResRefStr, out ResType))
            {
                ResRef = StringToResRef32("");

                return false;
            }

            ResRef = StringToResRef32(ResRefStr);
            return true;
        }

        //
        // Return the count of encapsulated files in this accessor.
        //

        private ulong GetEncapsulatedFileCount(IntPtr Context)
        {
            return m_ResourceAccessor.GetEncapsulatedFileCount();
        }

        //
        // Record a diagnostic message issued by the compiler.
        //

        private void CompilerDiagnosticOutput(string DiagnosticMessage, IntPtr Context)
        {
            m_CompilerDiagnosticsLog += DiagnosticMessage;
        }

        ///////////////////////////////////////////////////////////////////////


        //
        // Define whether we've disposed already.
        //

        private bool m_Disposed;

        //
        // Define the native compiler handle instance.
        //

        private IntPtr m_Compiler;

        //
        // Define the active resource accessor.
        //

        private ResourceAccessor m_ResourceAccessor;

        //
        // Define the settings manager instance that is used to read
        // configuration data.
        //

        private SettingsManager m_SettingsManager;

        //
        // Define any accumulated diagnostics output from the currently active
        // compilation session on the call stack.
        //

        private string m_CompilerDiagnosticsLog;

        //
        // Define whether we're compiling currently.
        //

        private bool m_Compiling;
    }
}

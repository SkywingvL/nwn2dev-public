Overview
--------

This NWNX4 plugin improves script performance by transparently executing
compiled NWScript files as native code instead of running them in an
interpreter.  No changes are required to scripts in order to use the
plugin, but there are some installation requirements.

In addition, this plugin allows scripts to be written in CLR enabled
languages, like C#.  The "CLR Script Support" section has more details.

Installation
------------

To install the plugin, you will need to do the following:

1) Ensure that the server computer has the .NET Framework v4.0 installed.  The
   download link for the .NET Framework v4.0 installer is:

http://www.microsoft.com/downloads/details.aspx?FamilyID=9cfb2d51-5ff4-4491-b0e5-b386f32c0992&displaylang=en

2) Ensure that the server computer has updated Visual C++ 2010 SP1 x86 runtimes
   installed.  The download link for the Visual C++ 2010 SP1 x86 installer
   package is:

http://www.microsoft.com/downloads/en/details.aspx?FamilyID=c32f406a-f8fc-4164-b6eb-5328b8578f03

3) Place xp_AuroraServerNWScript.dll from the plugin package into the NWNX4
   installation directory.  This DLL must be placed in the NWNX4 directory.
   Place AuroraServerNWScript.ini in the NWNX4 directory as well.

4) Place NWNScriptJIT.dll and NWNScriptJITIntrinsics.dll from the plugin
   package into the NWN2 installation directory (i.e. under Program Files).
   These DLLs must be placed in the game installation directory.

5) Start the server with NWNX4 as normal and check that everything appeared to
   work.  The plugin writes an "AuroraServerNWScript.log" log file to the NWNX4
   installation directory.  If everything worked, the log file should contain
   several lines like so, for each script that has ever been run:

NWScriptRuntime::LoadScript: Generated code for script 'k_mod_load' (10640
bytes compiled script) in 47ms.

6) Optionally, configure settings in AuroraServerNWScript.ini.  The settings
   are documented in the INI file and relate to configuring how much memory the
   plugin should be allowed to use.

CLR Script Support
------------------

Scripts authored in CLR languages, such as C#, can be used with the NWScript
Accelerator plugin in use.  To enable these scripts, you must first edit the
configuration file (AuroraServerNWScript.ini), and set ManagedScriptSupport=1.

Full details about how to write a CLR script, including a link to the current
SDK, can be found here:

http://www.nwnx.org/phpBB2/viewtopic.php?p=14922#14922

Profiling
---------

The plugin has an integrated profiling capability.  To access this capability,
call the included "gui_nwscriptvm_log_stats" script (or integrate the script's
functionality into your own scripts).  The NWScriptVM_LogStatistics function
writes profiling data out to the plugin log file.

An automatic profiling script, "nwnx_nwscriptvm_autoprofiler", is also included
with the plugin package.  This script will log profiling information about
scripts to the plugin log once a day if the script is called once (such as
during module initialization).

Troubleshooting
---------------

- If no scripts are firing after installation of the plugin, then check for the
  following:

  - Are NWNScriptJIT.dll and NWNScriptJITIntrinsics.dll in the game install
    directory?  If the game install directory is not where you have placed
    NWNX4, these DLLs must be placed in the game install directory.

  - Is the .NET Framework v4.0 installed?

  - Is the Visual C++ 2010 SP1 runtime installed?

- If there is no AuroraServerNWScript.log file in the NWNX4 directory, then
  check for the following:

  - Is xp_AuroraServerNWScript.dll present in the NWNX4 directory?

- When playing Act I of the original campaign, some scripts fail to load with a
  "mismatched stack on control flow" error, such as 10b_cw_bldggd_hb.  An
  example error message is:

NWScriptGenerateCode: Exception 'mismatched stack on control flow at
PC=0000A101 (SP=0000000C, FlowSP=00000008)' generating code for script
'10b_cw_bldggd_hb'.
ServerNWScriptPlugin::ExecuteScriptForServer:  ERROR: Failed to execute
script '' at PC 00000000 (exception: 'Failed to generate code for script.').

  - Several scripts in Act I were compiled against an old nwscript.nss where
    ActionUseSkill returned void (instead of an int).  These scripts no longer
    function correctly with or without xp_AuroraServerNWScript.dll; they would
    silently fail to work properly previously.  To fix this problem, the
    scripts need to be recompiled with a current nwscript.nss.

Source code
-----------

The latest source code is periodically published as a part of the NWN2 Datafile
Accessor Library, which is available here:

http://www.nynaeve.net/Skywing/nwn2/nwn2dev/public_nwn2dev.zip

Release updates for the library are posted here:

http://social.bioware.com/forum/Neverwinter-Nights-2/NwN-2-Tools-and-Plugin-Developers/NWN2-Datafile-Accessor-Library-3116521-1.html


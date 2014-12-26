/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	NWN2ToolsetCompilerPlugin.cs

Abstract:

    This module houses the main class of the compiler plugin.  The
    NWN2ToolsetCompilerPlugin object supports communication with the toolset
    core and UI.

--*/

using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using NWN2Toolset;
using NWN2Toolset.Plugins;
using NWN2Toolset.NWN2.UI;
using NWN2Toolset.NWN2.Views;
using NWN2Toolset.NWN2.Data;
using NWN2Toolset.NWN2.Data.Campaign;
using TD.SandBar;
using System.Reflection;
using OEIShared.IO;
using OEIShared.UI;
using OEIShared.Utils;

namespace NWN2ToolsetCompilerPlugin
{
    public class NWN2ToolsetCompilerPlugin : INWN2VersionControlPlugin
    {

        public static NWN2ToolsetCompilerPlugin Plugin;

        //
        // Standard INWN2Plugin methods.
        //

        //
        // Called during late stage toolset initialization.
        //

        virtual public void Load(INWN2PluginHost Host)
        {
            ListView Lv;

            m_SettingsManager = new SettingsManager();
            m_SettingsManager.NeedSettingsLoad = true;

            //
            // By the time Load is invoked, the compiler is present.  Install
            // the compiler hook now.
            //

            m_CompilerField = GetMainFormCompilerField();

            m_ResourceAccessor = new ResourceAccessor(this);

            HookScriptCompiler();

            //
            // Hook the version control system for resource change notification
            // distribution if preferences were available at this point.
            //
            // Otherwise, wait for a change notification to install the hook.
            //
            //

            if (NWN2ToolsetMainForm.VersionControlManager.Preferences != null)
            {
                m_OriginalVCPlugin = NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider;

                if (m_OriginalVCPlugin == this)
                    m_OriginalVCPlugin = null;

                NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider = this;
            }
            else
            {
                NWN2Toolset.Plugins.NWN2VersionControlManagerPreferences.PreferencesChanged += this.OnVersionControlManagerPrefsChanged;
            }

            Lv = PluginUI.GetVerifyOutputListView();

            if (Lv != null)
                Lv.ItemActivate += this.OnVerifyOutputListViewItemActivated;
        }

        //
        // Called during early stage toolset deinitialization.
        //

        virtual public void Unload(INWN2PluginHost Host)
        {
            ListView Lv;

            //
            // Early toolset teardown; deinitialize the script compiler hook so
            // that we may unload cleanly.
            //

            Lv = PluginUI.GetVerifyOutputListView();

            if (Lv != null)
                Lv.ItemActivate -= this.OnVerifyOutputListViewItemActivated;

            UnhookScriptCompiler();

            if (NWN2ToolsetMainForm.VersionControlManager.Preferences != null)
            {
                if (NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider == this)
                    NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider = m_OriginalVCPlugin;

                m_OriginalVCPlugin = null;
            }

            m_SettingsManager = null;
        }

        //
        // Called during early stage toolset initialization.
        //

        virtual public void Startup(INWN2PluginHost Host)
        {
            //
            // Spin up the plugin menu UI.
            //

            Plugin = this;

            m_MenuItem = Host.GetMenuForPlugin(this);
            m_MenuItem.Activate += new EventHandler(this.OnPluginMenuActivated);
        }

        //
        // Called during late stage toolset deinitialization.
        //

        virtual public void Shutdown(INWN2PluginHost Host)
        {
            if (Plugin == this)
                Plugin = null;
        }

        //
        // Standard INWN2Plugin properties.
        //

        virtual public MenuButtonItem PluginMenuItem
        {
            get
            {
                return m_MenuItem;
            }
        }

        virtual public string DisplayName
        {
            get
            {
                return "Advanced Script Compiler";
            }
        }

        virtual public string MenuName
        {
            get
            {
                return "Advanced Script Compiler";
            }
        }

        virtual public string Name
        {
            get
            {
                return "Advanced Script Compiler";
            }
        }

        virtual public object Preferences
        {
            get
            {
                return null;
            }
            set
            {
            }
        }


        //
        // Called when the resource system is indexing resources.
        //

        public void OnResourceIndexingStart(ResourceAccessor Accessor)
        {
            /*
            PluginUI.AddVerifyResult(
                    NWN2VerifyOutputType.Information,
                    "Advanced script compiler: Indexing resources...");
             */
        }

        public void OnResourceIndexingFinish(ResourceAccessor Accessor)
        {
            PluginUI.AddVerifyResult(
                    NWN2VerifyOutputType.Information,
                    "Advanced script compiler: Completed resource indexing (" + Accessor.GetEncapsulatedFileCount().ToString() + " resources in resource system).");
        }

        //
        // Standard INWN2VersionControlManager methods.
        //


        //
        // INWN2VersionControlPlugin interface methods.  Reflect everything
        // back to the original implementation (if we had one), except for
        // resource load, resource remove, and resource rename, which we'll use
        // as hints to invalidate the local index cache.
        //

        public virtual bool ActivateProvider() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.ActivateProvider(); else return true; }
        public virtual bool BindCampaign() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.BindCampaign(); else return false; }
        public virtual bool BindModule() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.BindModule(); else return false;  }
        public virtual bool DeactivateProvider() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.DeactivateProvider(); else return true; }
        public virtual void OnCampaignClosed() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnCampaignClosed(); }
        public virtual bool OnCampaignClosing() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnCampaignClosing(); else return true; }
        public virtual void OnCampaignLoaded() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnCampaignLoaded(); }
        public virtual bool OnCampaignLoading() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnCampaignLoading(); else return true; }
        public virtual void OnCampaignSaved() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnCampaignSaved(); }
        public virtual bool OnCampaignSaving() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnCampaignSaving(); else return true; }
        public virtual void OnModuleClosed() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnModuleClosed(); }
        public virtual bool OnModuleClosing() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnModuleClosing(); else return true; }
        public virtual void OnModuleLoaded()
        {
            m_ResourceAccessor.InvalidateResourceIndexes();

            //
            // N.B.  We cannot open a resource at this stage of the module load
            //       process.  Attempting to do so may cause the toolset to
            //       fail to load the module completely.  Thus, loading of
            //       CompilerSettings.gff is deferred until first use.
            //

            m_SettingsManager.NeedSettingsLoad = true;

            if (m_OriginalVCPlugin != null)
                m_OriginalVCPlugin.OnModuleLoaded();
        }
        public virtual bool OnModuleLoading() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnModuleLoading(); else return true; }
        public virtual void OnModuleSaved() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnModuleSaved(); }
        public virtual bool OnModuleSaving() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnModuleSaving(); else return true; }
        public virtual void OnModuleStartLocationChanged() { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnModuleStartLocationChanged(); }
        public virtual void OnResourceClosed(object oResource) { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnResourceClosed(oResource); }
        public virtual bool OnResourceClosing(object oResource) { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnResourceClosing(oResource); else return true; }
        public virtual void OnResourceLoaded(object oResource)
        {
            m_ResourceAccessor.InvalidateResourceIndexes();

            if (m_OriginalVCPlugin != null)
                m_OriginalVCPlugin.OnResourceLoaded(oResource);
        }
        public virtual bool OnResourceLoading(object oResource) { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnResourceLoading(oResource); else return true; }
        public virtual void OnResourcePropertiesOpened(object oResource) { if (m_OriginalVCPlugin != null) m_OriginalVCPlugin.OnResourcePropertiesOpened(oResource);  }
        public virtual void OnResourceRemoved(object oResource)
        {
            m_ResourceAccessor.InvalidateResourceIndexes();

            if (m_OriginalVCPlugin != null)
                m_OriginalVCPlugin.OnResourceRemoved(oResource);
        }
        public virtual void OnResourceRenamed(IResourceEntry cEntry, string sOldName)
        {
            m_ResourceAccessor.InvalidateResourceIndexes();

            if (m_OriginalVCPlugin != null)
                m_OriginalVCPlugin.OnResourceRenamed(cEntry, sOldName);
        }
        public virtual OEIMessageBoxResult OnResourceSaved(object oResource, OEIMessageBoxResult globalResult) { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnResourceSaved(oResource, globalResult); else return globalResult; }
        public virtual bool OnResourceSaving(object oResource) { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.OnResourceSaving(oResource); else return true; }
        public virtual bool UnbindCampaign() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.UnbindCampaign(); else return false; }
        public virtual bool UnbindModule() { if (m_OriginalVCPlugin != null) return m_OriginalVCPlugin.UnbindModule(); else return false; }

        //
        // Retrieve (or set) the original version control plugin.
        //

        public INWN2VersionControlPlugin OriginalVCPlugin
        {
            get
            {
                return m_OriginalVCPlugin;
            }
            set
            {
                m_OriginalVCPlugin = value;
            }
        }

        //
        // Retrieve the settings manager.
        //

        public SettingsManager Settings
        {
            get { return m_SettingsManager; }
        }


        ///////////////////////////////////////////////////////////////////////

        //
        // Search for the Compiler field in the toolset main form (to hook).
        //

        private FieldInfo GetMainFormCompilerField()
        {
            FieldInfo[] StaticFields = typeof(NWN2ToolsetMainForm).GetFields(BindingFlags.NonPublic | BindingFlags.Static);
            FieldInfo CompilerField = null;

            //
            // Search through the static private fields of the main form class,
            // searching for the Compiler field.
            //
            // N.B.  This must be done based on type as the name information is
            //       stripped.
            //

            foreach (FieldInfo Field in StaticFields)
            {
                if (Field.FieldType == typeof(IScriptCompiler))
                {
                    CompilerField = Field;
                    break;
                }
            }

            return CompilerField;
        }

        //
        // Establish the compiler hook (if we haven't already).
        //

        private void HookScriptCompiler()
        {
            if (m_Compiler != null)
                return;

            if (m_CompilerField == null)
            {
                MessageBox.Show(NWN2ToolsetMainForm.App,
                    "Failed to hook script compiler (compiler field not found).  The standard compiler will be used.");

                return;
            }

            IScriptCompiler Compiler = (IScriptCompiler)m_CompilerField.GetValue(null);

            if (Compiler == null)
            {
                MessageBox.Show(NWN2ToolsetMainForm.App,
                    "Failed to hook script compiler (compiler not initialized).  The standard compiler will be used.");

                return;
            }

            m_Compiler = new ScriptCompilerHook(Compiler, m_ResourceAccessor, m_SettingsManager);

            m_CompilerField.SetValue(null, m_Compiler);
        }

        //
        // Deestablish the compiler hook, if it was setup.
        //

        private void UnhookScriptCompiler()
        {
            if (m_Compiler == null)
                return;
            else if (m_CompilerField == null)
                return;

            m_CompilerField.SetValue(null, m_Compiler.Compiler);
            m_Compiler.Compiler = null;

            m_Compiler = null;
        }

        //
        // Called when the menu item is activated by the user.
        //

        private void OnPluginMenuActivated(object sender, EventArgs e)
        {
            if (m_Compiler == null)
                MessageBox.Show(NWN2ToolsetMainForm.App, "The standard compiler is in use because the plugin failed to initialize properly.");
            else
            {
                Settings.LoadSettings();
                SettingsForm Form = new SettingsForm(this);
                Form.ShowDialog(NWN2ToolsetMainForm.App);
            }
        }

        //
        // Called when the version control manager preferences have changed,
        // for purposes of allowing us to insert our hook if preferences were
        // not available at initialization time.
        //

        private void OnVersionControlManagerPrefsChanged(object sender, EventArgs e)
        {
            if (NWN2ToolsetMainForm.VersionControlManager.Preferences == null)
                return;

            m_OriginalVCPlugin = NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider;

            if (m_OriginalVCPlugin == this)
                m_OriginalVCPlugin = null;

            NWN2ToolsetMainForm.VersionControlManager.Preferences.ActiveProvider = this;

            //
            // Remove the change notification registration now as we no longer
            // have need of it.
            //

            NWN2Toolset.Plugins.NWN2VersionControlManagerPreferences.PreferencesChanged -= this.OnVersionControlManagerPrefsChanged;
        }

        //
        // Called when the verify output listview has an item activated.  The
        // item text is parsed to discern whenther the message activated was a
        // compiler message; if so, an editor window is opened up to the script
        // namd in the message.
        //

        private void OnVerifyOutputListViewItemActivated(object sender, EventArgs e)
        {
            ListView OutputListView = (ListView)sender;

            foreach (object Item in OutputListView.SelectedItems)
            {
                INWN2Viewer Viewer;
                NWN2ScriptViewer ScriptViewer;
                NWN2GameScript Script;
                IResourceEntry ResEntry;
                ListViewItem LvItem = (ListViewItem)Item;
                string ItemText = LvItem.Text;
                string ResName;
                string LineString;
                string MsgString;
                int Line;
                int i;

                //
                // Try and parse out the file name and line number of the error
                // message, if it appears to be a compiler error.
                //

                // script1.nss(5): Warning: Usage of switch blocks inside of do/while scopes generates incorrect code with the standard compiler; consider avoiding the use of do/while constructs to ensure correct code generation with the standard compiler

                i = ItemText.IndexOf('.');

                if (i == -1)
                    continue;

                ResName = ItemText.Substring(0, i);

                LineString = ItemText.Substring(i+1).ToLower();

                if (!LineString.StartsWith("nss("))
                    continue;

                i = LineString.IndexOf(')', 4);

                if (i < 5)
                    continue;

                MsgString = LineString.Substring(i+1);

                if (!MsgString.StartsWith(": error: ") &&
                    !MsgString.StartsWith(": warning: "))
                {
                    continue;
                }

                try
                {
                    Line = Convert.ToInt32(LineString.Substring(4, i - 4));
                }
                catch
                {
                    continue;
                }

                //
                // Look up the resource in the resource system.
                //

                ResEntry = ResourceManager.Instance.GetEntry(
                    new OEIResRef(ResName),
                    (ushort) ResTypes.ResNSS);

                if (ResEntry is MissingResourceEntry)
                    continue;

                //
                // Find the toolset data item for the script in question.
                //

                Script = null;

                try
                {
                    Viewer = NWN2ToolsetMainForm.App.GetViewerForResource(
                        ResEntry.FullName);

                    if (Viewer != null)
                        Script = (NWN2GameScript)Viewer.ViewedResource;

                    if (Script == null)
                    {
                        Script = NWN2ToolsetMainForm.App.Module.Scripts[ResName];

                        if (Script == null)
                        {
                            NWN2Campaign Campaign = NWN2CampaignManager.Instance.ActiveCampaign;

                            if (Campaign != null)
                                Script = Campaign.Scripts[ResName];
                        }

                        if (Script == null)
                            Script = new NWN2GameScript(ResEntry);
                    }
                }
                catch
                {
                    Script = null;
                }

                if (Script == null)
                    continue;
                
                //
                // If a viewer isn't open for the script yet, then open one
                // now.
                //

                Viewer = NWN2ToolsetMainForm.App.GetViewerForResource(Script);

                if (Viewer == null)
                {
                    NWN2ToolsetMainForm.App.ShowResource(Script);

                    Viewer = NWN2ToolsetMainForm.App.GetViewerForResource(Script);

                    if (Viewer == null)
                        continue;
                }
                else
                {
                    //
                    // Bring the viewer to the front.
                    //

                    NWN2ToolsetMainForm.App.ShowResource(Script);
                }

                if (!(Viewer is NWN2ScriptViewer))
                    continue;

                ScriptViewer = (NWN2ScriptViewer)Viewer;

                //
                // Highlight the error line.
                //

                ScriptViewer.HighlightLine(Line);
            }
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Define the compiler hook, which mediates requests between the
        // toolset and the actual script compiler.
        //

        private ScriptCompilerHook m_Compiler;

        //
        // Define the resource accessor that mediates access to the internal
        // resource system of the toolset.
        //

        private ResourceAccessor m_ResourceAccessor;

        //
        // Define the original (real) version control plugin, if any.
        //

        private INWN2VersionControlPlugin m_OriginalVCPlugin;

        //
        // Define the Compiler field descriptor within the toolset main form.
        //

        private FieldInfo m_CompilerField;

        //
        // Define the current GUI menu item.
        //

        private MenuButtonItem m_MenuItem;

        //
        // Define the settings manager.
        //

        private SettingsManager m_SettingsManager;

    }

    class PluginUI
    {

        public static void AddVerifyResult(NWN2VerifyOutputType Type, string Result, bool ScrollToEnd = true)
        {
            NWN2VerifyOutputControl VerifyOutput = NWN2ToolsetMainForm.App.VerifyOutput;

            VerifyOutput.Show();
            VerifyOutput.AddResult(Type, Result);

            if (!ScrollToEnd)
                return;

            ListView VerifyOutputListView = GetVerifyOutputListView();

            if (VerifyOutputListView == null)
                return;

            VerifyOutputListView.EnsureVisible(VerifyOutputListView.Items.Count - 1);
        }

        //
        // Search for the ListView field in the verify output window control.
        //

        private static FieldInfo GetVerifyOutputListViewField()
        {
            FieldInfo[] StaticFields = typeof(NWN2VerifyOutputControl).GetFields(BindingFlags.NonPublic | BindingFlags.Instance);
            FieldInfo ListViewField = null;

            //
            // Search through the instance fields of the main form class,
            // searching for the ListView field.
            //
            // N.B.  This must be done based on type as the name information is
            //       stripped.
            //

            foreach (FieldInfo Field in StaticFields)
            {
                if (Field.FieldType == typeof(ListView))
                {
                    ListViewField = Field;
                    break;
                }
            }

            return ListViewField;
        }

        //
        // Return the ListView field value out of the verify window.
        //

        public static ListView GetVerifyOutputListView()
        {
            if (m_sVerifyListView != null)
                return m_sVerifyListView;

            FieldInfo Field = GetVerifyOutputListViewField();

            if (Field == null)
                return null;

            m_sVerifyListView = (ListView)Field.GetValue(NWN2ToolsetMainForm.App.VerifyOutput);
            return m_sVerifyListView;
        }

        private static ListView m_sVerifyListView;

    }
}

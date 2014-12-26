using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace NWN2ToolsetCompilerPlugin
{
    public partial class SettingsForm : Form
    {
        public SettingsForm(NWN2ToolsetCompilerPlugin Plugin)
        {
            SettingsManager Settings;

            m_Plugin = Plugin;
            Settings = m_Plugin.Settings;

            InitializeComponent();

            CompilerVersionTextBox.Text = Convert.ToString(Settings.CompilerVersion);
            EnableExtensionsCheckBox.Checked = Settings.EnableExtensions;
            EnableDbgSymbolsCheckBox.Checked = Settings.EnableDebugSymbols;

            EnableExtensionsCheckBox.Focus();
        }

        private void OKButton_Click(object sender, EventArgs e)
        {
            SettingsManager Settings;
            int CompilerVersion;

            Settings = m_Plugin.Settings;

            try
            {
                CompilerVersion = Convert.ToInt32(CompilerVersionTextBox.Text);
            }
            catch
            {
                MessageBox.Show(this, "Invalid compiler version (must be numeric value).", "Invalid compiler version");
                return;
            }

            Settings.EnableExtensions = EnableExtensionsCheckBox.Checked;
            Settings.EnableDebugSymbols = EnableDbgSymbolsCheckBox.Checked;
            Settings.CompilerVersion = CompilerVersion;

            try
            {
                Settings.SaveSettings();
            }
            catch (Exception ex)
            {
                MessageBox.Show(this, String.Format("Failed to save settings (settings stored in hak files must be manually saved in a standalone module and re-added to the hak): Exception: {0}, stack {1}", ex, ex.StackTrace), "Failed to save settings");
            }

            Close();
        }

        private void FormCancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private NWN2ToolsetCompilerPlugin m_Plugin;
    }
}

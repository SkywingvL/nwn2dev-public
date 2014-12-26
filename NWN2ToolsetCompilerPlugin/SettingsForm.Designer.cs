namespace NWN2ToolsetCompilerPlugin
{
    partial class SettingsForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>

        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(SettingsForm));
            this.OKButton = new System.Windows.Forms.Button();
            this.FormCancelButton = new System.Windows.Forms.Button();
            this.EnableExtensionsCheckBox = new System.Windows.Forms.CheckBox();
            this.EnableDbgSymbolsCheckBox = new System.Windows.Forms.CheckBox();
            this.CompilerVersionTextBox = new System.Windows.Forms.TextBox();
            this.CompilerVersionLabel = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // OKButton
            // 
            this.OKButton.Location = new System.Drawing.Point(351, 227);
            this.OKButton.Name = "OKButton";
            this.OKButton.Size = new System.Drawing.Size(75, 23);
            this.OKButton.TabIndex = 3;
            this.OKButton.Text = "&OK";
            this.OKButton.Click += new System.EventHandler(this.OKButton_Click);
            // 
            // FormCancelButton
            // 
            this.FormCancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.FormCancelButton.Location = new System.Drawing.Point(432, 227);
            this.FormCancelButton.Name = "FormCancelButton";
            this.FormCancelButton.Size = new System.Drawing.Size(75, 23);
            this.FormCancelButton.TabIndex = 4;
            this.FormCancelButton.Text = "&Cancel";
            this.FormCancelButton.Click += new System.EventHandler(this.FormCancelButton_Click);
            // 
            // EnableExtensionsCheckBox
            // 
            this.EnableExtensionsCheckBox.Location = new System.Drawing.Point(12, 12);
            this.EnableExtensionsCheckBox.Name = "EnableExtensionsCheckBox";
            this.EnableExtensionsCheckBox.Size = new System.Drawing.Size(423, 46);
            this.EnableExtensionsCheckBox.TabIndex = 0;
            this.EnableExtensionsCheckBox.Text = resources.GetString("EnableExtensionsCheckBox.Text");
            // 
            // EnableDbgSymbolsCheckBox
            // 
            this.EnableDbgSymbolsCheckBox.Location = new System.Drawing.Point(12, 64);
            this.EnableDbgSymbolsCheckBox.Name = "EnableDbgSymbolsCheckBox";
            this.EnableDbgSymbolsCheckBox.Size = new System.Drawing.Size(423, 24);
            this.EnableDbgSymbolsCheckBox.TabIndex = 1;
            this.EnableDbgSymbolsCheckBox.Text = "Always generate &debug symbols.";
            // 
            // CompilerVersionTextBox
            // 
            this.CompilerVersionTextBox.Location = new System.Drawing.Point(12, 104);
            this.CompilerVersionTextBox.Name = "CompilerVersionTextBox";
            this.CompilerVersionTextBox.Size = new System.Drawing.Size(100, 20);
            this.CompilerVersionTextBox.TabIndex = 2;
            this.CompilerVersionTextBox.Text = "169";
            // 
            // CompilerVersionLabel
            // 
            this.CompilerVersionLabel.Location = new System.Drawing.Point(118, 91);
            this.CompilerVersionLabel.Name = "CompilerVersionLabel";
            this.CompilerVersionLabel.Size = new System.Drawing.Size(317, 48);
            this.CompilerVersionLabel.TabIndex = 0;
            this.CompilerVersionLabel.Text = "BioWare-compatible compiler version.  Version 169 or below will enable warnings f" +
    "or code that will only compile with the Advanced Script Compiler due to standard" +
    " compiler limitations or bugs.";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(12, 176);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(498, 48);
            this.label1.TabIndex = 1;
            this.label1.Text = resources.GetString("label1.Text");
            // 
            // SettingsForm
            // 
            this.AcceptButton = this.OKButton;
            this.CancelButton = this.FormCancelButton;
            this.ClientSize = new System.Drawing.Size(519, 262);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.OKButton);
            this.Controls.Add(this.FormCancelButton);
            this.Controls.Add(this.EnableExtensionsCheckBox);
            this.Controls.Add(this.EnableDbgSymbolsCheckBox);
            this.Controls.Add(this.CompilerVersionTextBox);
            this.Controls.Add(this.CompilerVersionLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "SettingsForm";
            this.Text = "Module Compiler Settings";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button OKButton;
        private System.Windows.Forms.Button FormCancelButton;
        private System.Windows.Forms.CheckBox EnableExtensionsCheckBox;
        private System.Windows.Forms.CheckBox EnableDbgSymbolsCheckBox;
        private System.Windows.Forms.TextBox CompilerVersionTextBox;
        private System.Windows.Forms.Label CompilerVersionLabel;
        private System.Windows.Forms.Label label1;
    }
}
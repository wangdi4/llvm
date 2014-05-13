namespace APIDebugger.CommandQueueMapClasses
{
    using System;
    using System.Collections.Generic;
    using System.CodeDom.Compiler;
    using Microsoft.VisualStudio.TestTools.UITest.Extension;
    using Microsoft.VisualStudio.TestTools.UITesting;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
    using Mouse = Microsoft.VisualStudio.TestTools.UITesting.Mouse;
    using Microsoft.VisualStudio.TestTools.UITesting.WpfControls;
    using Microsoft.VisualStudio.TestTools.UITesting.WinControls;
    using MouseButtons = System.Windows.Forms.MouseButtons;
    using System.Drawing;
    using System.Windows.Input;
    using System.Text.RegularExpressions;
    using APIDebugger.UIBasicMapClasses;
    using System.Diagnostics;
    public partial class CommandQueueMap
    {
        /// <summary>
        /// LoadSolution - Use 'LoadSolutionParams' to pass parameters into this method.
        /// </summary>
        public string[] GetDeviceTypes()
        {
            Trace.Indent();
            BasicMap.SelectTab("Command Queue");
            #region Variable Declarations
            WinButton uISeperateQueuesButton = UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UISeperateQueuesButton;
            WinComboBox uIComboBoxQueueComboBox = (WinComboBox)BasicMap.GetChild(UITestAppDebuggingMicrWindow1.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIComboBoxQueueWindow, "ComboBox");
            #endregion

            Trace.TraceInformation("Check if specific queue should be chosen");
            if (uISeperateQueuesButton.TryFind())
            {
                Trace.TraceInformation("Select separate queue.");
                // Click 'Separate Queues' label
                BasicMap.MouseClick(uISeperateQueuesButton);
            }
            else
            {
                Trace.TraceInformation("Separate queue has been already selected.");
            }

            string []devices=new string[3];
            int i=0;   
            foreach (UITestControl item in uIComboBoxQueueComboBox.Items)
            {
                Trace.TraceInformation("Read device info from " + item.FriendlyName);
                devices[i++]=Regex.Match(item.FriendlyName,@"\(([C,P,U,G,M,I]*),").Groups[1].Value;
            }
            Trace.TraceInformation("Device types are: " + string.Join(",",devices));

            Trace.Unindent();
            return devices;
        }

        public UIBasicMapClasses.UIBasicMap BasicMap
        {
            get
            {
                if ((this._BasicMap == null))
                {
                    this._BasicMap = new UIBasicMapClasses.UIBasicMap();
                }

                return this._BasicMap;
            }
        }

        private UIBasicMapClasses.UIBasicMap _BasicMap;
    }
}


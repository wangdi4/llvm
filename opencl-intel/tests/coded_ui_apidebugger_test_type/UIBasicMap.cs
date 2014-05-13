namespace APIDebugger.UIBasicMapClasses
{
    using Microsoft.VisualStudio.TestTools.UITesting.WpfControls;
    using Microsoft.VisualStudio.TestTools.UITesting.WinControls;
    using System;
    using System.Collections.Generic;
    using System.CodeDom.Compiler;
    using Microsoft.VisualStudio.TestTools.UITest.Extension;
    using Microsoft.VisualStudio.TestTools.UITesting;
    using Microsoft.VisualStudio.TestTools.UnitTesting;
    using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
    using Mouse = Microsoft.VisualStudio.TestTools.UITesting.Mouse;
    using MouseButtons = System.Windows.Forms.MouseButtons;
    using System.Drawing;
    using System.Windows.Input;
    using System.Text.RegularExpressions;
    using System.Diagnostics;
    using EnvDTE80;
    using EnvDTE;
    using System.Runtime.InteropServices;
    using System.Runtime.InteropServices.ComTypes;
    using System.Windows.Automation;
    public sealed class DebuggerEngine
    {
        private EnvDTE80.DTE2 _DTE2 = null;
        private string VSVersion;
        public DebuggerEngine(string VSVersion)
        {
            this.VSVersion = VSVersion;
            QuitDTE();

            try
            {
                CreateDTE();
            }
            catch (Exception ex)
            {
                Trace.TraceInformation(ex.Message);
                throw new Exception("CreateDTE failed");
            }

            try
            {
                 DoWithRetry(() => DTE2.MainWindow.Activate());
            }
            catch (Exception ex)
            {
                Trace.TraceInformation(ex.Message);
                throw new Exception("DTE2.MainWindow.Activate() failed");
            }

            try
            {
                DTE2.MainWindow.WindowState = EnvDTE.vsWindowState.vsWindowStateMaximize;
                AutomationElement.FromHandle(((IntPtr) DoWithRetry(() => DTE2.MainWindow.HWnd))).SetFocus();
            }
            catch (Exception ex)
            {
                Trace.TraceInformation(ex.Message);
                throw new Exception("..AutomationElement.SetFocus() failed");
            }
            CheckOCLDebugEngine();
        }
        public EnvDTE80.DTE2 DTE2
        {
            get
            {
                return _DTE2;
            }
        }
        public void Go(bool WaitForBreakOrEnd = true)
        {
            DoWithRetry(() => DTE2.Debugger.Go(WaitForBreakOrEnd));
        }

        private void QuitDTE()
        {
            try
            {
                if (_DTE2 != null)
                    _DTE2.Quit();
            }
            catch (Exception) { }
            _DTE2 = null;
        }

        public void Close()
        {
            DoWithRetry(() => DTE2.Debugger.Stop());
            DoWithRetry(() => DTE2.Solution.Close(false));
            QuitDTE();
        }

        private void CreateDTE()
        {
            String DTEObjectName = "VisualStudio.DTE." + VSVersion + ".0";
            Type type;


            type = Type.GetTypeFromProgID(DTEObjectName);
            if (type == null)
                throw new Exception("Can't find " + DTEObjectName + " type.");

            System.Threading.Thread.Sleep(1000);

            bool res = true;
            for (int i = 0; i < 10; i++)
            {
                try
                {
                    _DTE2 = (EnvDTE80.DTE2)Activator.CreateInstance(type);
                    res = true;
                    break;
                }
                catch (Exception ex)
                {
                    Trace.TraceInformation(ex.Message);
                    res = false;
                    System.Threading.Thread.Sleep(10000);
                }
            }

            if (res == false)
            {
                throw new Exception("Activator.CreateInstance(type) failed");
            }

        }
        public void CheckOCLDebugEngine()
        {
            EnvDTE80.Debugger2 dbg = (EnvDTE80.Debugger2)DoWithRetry(() =>DTE2.Debugger);
            EnvDTE80.Transport trans = DoWithRetry(() => dbg.Transports.Item("Default"));
            EnvDTE80.Engine eng = null;
            try
            {
                eng = DoWithRetry(() =>trans.Engines.Item("Intel OpenCL Debug Engine"));
            }
            catch { }
            Assert.AreNotEqual(eng, null, "Intel OpenCL Debug Engine is not present");
        }
        /// <summary>
        /// Loads provided solution in prevoiuselly created debug engine
        /// </summary>
        /// <param name="name">Full path to solution file</param>
        public void SolutionOpen(string name)
        {
            Trace.Indent();

            Trace.TraceInformation("Try to load solution: "+name);
            DoWithRetry(() => DTE2.Solution.Open(name));
            Trace.Unindent();
        }
        private static void DoWithRetry(Action action, int retryNum = 10, int retryTime = 6000 )
        {
            DoWithRetry<object>(() => { action(); return null; });
        }
        private static T DoWithRetry<T>(Func<T> action, int retryNum = 10, int retryTime = 6000 )
        {
            List<Exception> exceptions = new List<Exception>();
            while (retryNum >= 0)
            {
                try
                {
                    return action();
                }catch (Exception e)
                {
                    exceptions.Add(e);
                    Playback.Wait(retryTime);
                    retryNum--;
                }
            }
            throw new AggregateException(exceptions);
        }
        /// <summary>
        /// Set up configuration for current solution
        /// </summary>
        /// <param name="ConfigName">Name of config (Debug|Release)</param>
        /// <param name="PlatformName">Platform (Win32|x64)</param>
        /// <returns></returns>
        public void SetActiveConfiguration(string ConfigName, string PlatformName)
        {
            Trace.Indent();
            Trace.TraceInformation("Try to set configuration for the solution: ({0}, {1})", ConfigName, PlatformName);

            SolutionConfigurations cfgs =  DoWithRetry(() => DTE2.Solution.SolutionBuild.SolutionConfigurations);
            for (int i = 1; i <= DoWithRetry(() =>cfgs.Count); i++)
            {
                SolutionConfiguration cfg = DoWithRetry(() =>cfgs.Item(i));
                SolutionContexts ssc = DoWithRetry(() =>cfg.SolutionContexts);
                for (int j = 1; j <= DoWithRetry(() =>ssc.Count); j++)
                {
                    SolutionContext sscc = DoWithRetry(() =>ssc.Item(j));
                    if (DoWithRetry(() =>sscc.ConfigurationName) == ConfigName && DoWithRetry(() =>sscc.PlatformName) == PlatformName)
                    {
                        DoWithRetry(cfg.Activate);
                    }
                }
            }

            Trace.Unindent();
        }

        /// <summary>
        /// Perform step over
        /// </summary>
        /// <param name="WaitForBreakOrEnd">If we should wait end of execution</param>
        public void StepOver(bool WaitForBreakOrEnd = true)
        {
            DoWithRetry(() => DTE2.Debugger.StepOver(WaitForBreakOrEnd));
        }
        /// <summary>
        /// Loads breakpoints in the Visual Studio
        /// </summary>
        /// <param name="file_name"> Name of the file containing list of breakpoints relative to the directory containing test</param>
        public void LoadBreakpoints(string[][] bps, string BaseDirectory, string TestApp)
        {
            Trace.Indent();
            Trace.TraceInformation("Clean breakpoints..");
            
            foreach (Breakpoint b in DoWithRetry(() =>  DTE2.Debugger.Breakpoints))
            {
                Trace.TraceInformation("Deleting " + b);
                DoWithRetry(()=>b.Delete());
            }
            foreach (string[] bp in bps)
            {
                string tmp = BaseDirectory + "\\" + TestApp + "\\" + bp[0];
                //DTE2.Debugger.Breakpoints.Add("",bp[0], int.Parse(bp[1]));
                Trace.TraceInformation("Adding " + tmp + ":" + bp[1]);
                DoWithRetry(() => DTE2.Debugger.Breakpoints.Add("", tmp, int.Parse(bp[1])));
            }
            Trace.Unindent();
        }

        public dbgDebugMode getMode()
        {
           return  DoWithRetry(() => DTE2.Debugger.CurrentMode);
        }
    }
    /*
    public sealed class VSSolution
    {
        public class SolutionConfigurationCollection : List<SolutionConfiguration>
        {
            internal SolutionConfigurationCollection(Solution solution)
            {
                SolutionConfigurations cfgs = solution.SolutionBuild.SolutionConfigurations;
                for (int i = 1; i <= cfgs.Count; i++)
                    Add(cfgs.Item(i));
            }
        }

        internal Solution solution = null;
        public SolutionConfigurationCollection SolutionConfigurations
        {
            get
            {
                return new SolutionConfigurationCollection(solution);
            }
        }

        public VSSolution(DTE dte)
        {
            solution = dte.Solution;
        }

        public VSSolution(DTE2 dte)
        {
            solution = dte.Solution;
        }

        public void Open(String FileName)
        {
            solution.Open(FileName);
        }

        public void Save(string Name)
        {
            solution.SaveAs(Name);
        }

        public void Save()
        {
            solution.SaveAs(solution.FullName);
        }

        public void Close()
        {
            solution.Close(false);
        }

        public bool SetActiveConfiguration(string ConfigName, string PlatformName)
        {
            SolutionConfigurations cfgs = solution.SolutionBuild.SolutionConfigurations;
            for (int i = 1; i <= cfgs.Count; i++)
            {
                SolutionConfiguration cfg = cfgs.Item(i);
                SolutionContexts ssc = cfg.SolutionContexts;
                for (int j = 1; j <= ssc.Count; j++)
                {
                    SolutionContext sscc = ssc.Item(j);
                    if (sscc.ConfigurationName == ConfigName && sscc.PlatformName == PlatformName)
                    {
                        cfg.Activate();
                        return true;
                    }
                }
            }
            return false;
        }

        public void Build()
        {
            if (solution.IsDirty)
                Save();
            solution.SolutionBuild.Build(true);
        }

        public void Clean()
        {
            solution.SolutionBuild.Clean(true);
        }

    }
     */
    /// <summary>
    /// Call conntaining basic functionality to work with Visual Studio
    /// </summary>
    public partial class UIBasicMap
    {
        /// <summary>
        /// Version of VS to be used (10-2010, 11-2012, 12-2013)
        /// </summary>
        public static string VSVersion = "Uninitialized";
        /// <summary>
        /// Folder name containing test application
        /// </summary>
        public static string TestApp = "Uninitialized";
        /// <summary>
        /// Autocalculated current directory
        /// </summary>
        public static string BaseDirectory = "Uninitialized";
        /// <summary>
        /// Architecture to be used for testing (Win32, x64)
        /// </summary>
        public static string Architecture = "Uninitialized";

        private DebuggerEngine _dbg = null;
        public DebuggerEngine dbg
        {
            get
            {
                if (_dbg == null)
                    _dbg = new DebuggerEngine(VSVersion);
                return _dbg;
            }
        }

        //public static EnvDTE80.DTE2 DTE;

        /// <summary>
        /// Loads configuration from XML file
        /// </summary>
        /// <param name="ctx"> TestContext object</param>
        /// <param name="App"> Name of aplication to be used for test</param>
        public void LoadConfig(TestContext ctx, string App)
        {
            TestApp = App;
            VSVersion = ctx.DataRow["VSVersion"].ToString();
            Architecture = ctx.DataRow["Architecture"].ToString();
            BaseDirectory = System.IO.Path.GetDirectoryName(Uri.UnescapeDataString((new UriBuilder(System.Reflection.Assembly.GetExecutingAssembly().CodeBase)).Path));
            Trace.TraceInformation("\tTestApp='" + TestApp + "'\n\tVSVersion='" + VSVersion + "'\n\tArchitecture='" + Architecture + "'\n\tBaseDirectory='" + BaseDirectory + "'");
        }

        /// <summary>
        /// Gets child of the specified control matching specified type
        /// </summary>
        /// <param name="item"> Parent control</param>
        /// <param name="controlType"> Type of control to be found</param>
        /// <returns>Control which was found or null</returns>
        public UITestControl GetChild(UITestControl item, string controlType)
        {
            Trace.Indent();
            Trace.TraceInformation("Looking for child of type " + controlType + " under item with name " + item.Name);
            foreach (UITestControl child in item.GetChildren())
            {
                Trace.TraceInformation("Checking control with name " + child.Name + "and type" + child.ControlType);
                if (child.ControlType == controlType)
                {
                    Trace.Unindent();
                    return child;
                }
            }
            Trace.Unindent();
            return null;
        }

        /// <summary>
        /// Loads breakpoints in the Visual Studio
        /// </summary>
        /// <param name="file_name"> Name of the file containing list of breakpoints relative to the directory containing test</param>
        public void LoadBreakpoints(string[][] bps)
        {
            dbg.LoadBreakpoints(bps,BaseDirectory,TestApp);
        }


        /// <summary>
        /// LoadSolution - Starts visual studio (version is taken from VSVersion variable) and loads solution (directory is taken from TestApp variable).
        /// </summary>
        public void LoadSolution()
        {
            #region Variable Declarations
            WinCheckBox uIAskmeforeveryprojectCheckBox = this.UISecurityWarningforTeWindow.UIAskmeforeveryprojectWindow.UIAskmeforeveryprojectCheckBox;
            WinButton uIOKButton = this.UISecurityWarningforTeWindow.UIOKWindow.UIOKButton;
            WpfComboBox uISolutionConfiguratioComboBox = this.UITestAppMicrosoftVisuWindow.UISolutionConfiguratioButton.UISolutionConfiguratioComboBox;
            WpfComboBox uISolutionPlatformsComboBox = this.UITestAppMicrosoftVisuWindow.UISolutionPlatformsButton.UISolutionPlatformsComboBox;
            WpfControl uISolutionTestApp1projTreeItem = this.UITestAppMicrosoftVisuWindow1.UIItemWindow.UISolutionExplorerClient.UISolutionExplorerWindow.UISolutionTestApp1projTreeItem;
            WinCheckBox uIDonotshowthisdialogaCheckBox = this.UIMicrosoftVisualStudiWindow.UIDonotshowthisdialogaWindow.UIDonotshowthisdialogaCheckBox;
            WinButton uIYesButton = this.UIMicrosoftVisualStudiWindow.UIYesWindow.UIYesButton;
            #endregion

            Trace.Indent();

            string tmp = BaseDirectory + @"\" + TestApp + @"\TestApp" + VSVersion + @"\TestApp.sln";
            dbg.SolutionOpen(tmp);
            Trace.TraceInformation("Select " + Architecture + " as target architecture.");
            // Select architecture (x64/Win32)
            if (Architecture.ToLower().Equals("x86") || Architecture.ToLower().Equals("win32"))
            {
                dbg.SetActiveConfiguration("Debug", "Win32");
            }
            else
            {
                dbg.SetActiveConfiguration("Debug", Architecture);
            }
            LoadLayots();
            dbg.StepOver();
            Playback.Wait(2000);

            /*
            if (uIYesButton.TryFind())
            {
                Trace.TraceInformation(" Question dialog appered, select avoid displaying it in future and press OK.");
                // Select '&Do not show this dialog again' check box
                uIDonotshowthisdialogaCheckBox.Checked = true;

                // Click '&Yes' button
                MouseClick(uIYesButton);

            }
*/
            Trace.Unindent();
        }
        /// <summary>
        /// MouseClick - Clicks to the center of specified control.
        /// </summary>
        /// <param name="control">Control to be clicked</param>
        /// <param name="buttons">Mouse button to be clicked (default=left)</param>
        /// <param name="buttons">Modifier key to be used (defult=none)</param>
        /// <returns></returns>
        public void MouseClick(UITestControl control, MouseButtons buttons = MouseButtons.Left, ModifierKeys modifier = ModifierKeys.None,bool doubleClick = false)
        {
            if (!control.WaitForControlReady(60000))
            {
                Trace.Fail("FAILED: Failed to reach program ready during selection of tab.");
            }
            try
            {
                Trace.TraceInformation("Try to " + (doubleClick ? "doubleclick" : "click") + " on center of component '" + control.FriendlyName + "' x=" + control.BoundingRectangle.Width / 2 + ", y" + control.BoundingRectangle.Height / 2);
                if (doubleClick) Mouse.DoubleClick(control, buttons, modifier, new Point(control.BoundingRectangle.Width / 2, control.BoundingRectangle.Height / 2));
                else Mouse.Click(control, buttons, modifier, new Point(control.BoundingRectangle.Width / 2, control.BoundingRectangle.Height / 2));
            }
            catch (SystemException e1)
            {
                Trace.TraceInformation(e1.StackTrace);
                try
                {
                    Trace.TraceInformation("Try to " + (doubleClick ? "doubleclick" : "click") + " on center of component '" + control.FriendlyName + "' x=" + 4 + ", y" + 4);
                    if (doubleClick) Mouse.DoubleClick(control, buttons, modifier, new Point(4, 4));
                    else Mouse.Click(control, buttons, modifier, new Point(4, 4));
                }
                catch (SystemException e)
                {
                    Trace.TraceInformation(e.StackTrace);
                    Trace.TraceInformation("Try to " + (doubleClick ? "doubleclick" : "click") + " point " + control.FriendlyName + " x=" + (control.BoundingRectangle.X + control.BoundingRectangle.Width / 2) +
                                                                                      ", y" + (control.BoundingRectangle.Y + control.BoundingRectangle.Height / 2));
                    if (doubleClick) Mouse.DoubleClick(buttons, modifier,
                                  new Point(control.BoundingRectangle.X + control.BoundingRectangle.Width / 2,
                                           control.BoundingRectangle.Y + control.BoundingRectangle.Height / 2));
                    else Mouse.Click(buttons, modifier,
                                  new Point(control.BoundingRectangle.X + control.BoundingRectangle.Width / 2,
                                           control.BoundingRectangle.Y + control.BoundingRectangle.Height / 2));
                }
            }
        }

        /// <summary>
        /// Select tabulation
        /// </summary>
        /// <param name="caption"> Name of the tab to be selected</param>
        public void SelectTab(string caption)
        {
            bool found = false;
            Trace.Indent();

            Trace.TraceInformation("Trying to select tab with name " + caption);
            int tabsNum = this.UITestAppDebuggingMicrWindow.UIItemTabList.GetChildren().Count;

            foreach (UITestControl child in this.UITestAppDebuggingMicrWindow.UIItemTabList.GetChildren())
            {
                Trace.TraceInformation("Check child '" + child.Name + "' of type " + child.ControlType);
                if (Regex.Match(child.FriendlyName,caption,RegexOptions.IgnoreCase).Success)
                {

                    Trace.TraceInformation("Requested tab has found, ensure that it is visible");

                    Trace.TraceInformation("control state is: " + child.State.ToString().ToLower());
                    while (child.State.ToString().ToLower().Contains("offscreen"))
                    {
                        Trace.TraceInformation("Tab seems to be invisible: " + child.State.ToString().ToLower() + " shift tabs... ");
                        Keyboard.SendKeys("{Tab}", ModifierKeys.Control | ModifierKeys.Shift);
                    }
                    found = true;

                    if (!child.WaitForControlReady(60000))
                    {
                        Trace.Fail("FAILED: Failed to reach program ready during selection of tab.");
                    }

                    try
                    {
                        MouseClick(child);
                    }
                    catch
                    {
                        Mouse.Click(child.BoundingRectangle.Location);
                    }

                    System.Threading.Thread.Sleep(1000);
                }
            }

            Trace.Unindent();
            Assert.IsTrue(found, String.Format("FAILED: Fialed to find tab with name \"{0}\".", caption));
        }

        /// <summary>
        /// Close visual studio
        /// </summary>
        public void CloseVS()
        {
            dbg.Close();
        }

        /// <summary>
        /// Continue/Start program execution under Visual Studio
        /// </summary> 
        /// <param name="continues"> if it is expected that we stop on breakpoint</param>
        public void ExecuteProgram(bool continues = true)
        {
            #region Variable Declarations
            WinButton uIYesButton = this.UIMicrosoftVisualStudiWindow.UIYesWindow.UIYesButton;
            //bool res;
            #endregion
            
            Trace.Indent();

            Trace.TraceInformation("Press F5 to continue execution");
            // Type '{F5}' in 'SimpleOptimizations.cpp, line 179' check box
            dbg.Go();

            Assert.AreEqual(dbgDebugMode.dbgBreakMode, dbg.getMode(), "Unexpected status for DTE engine");
            Trace.Unindent();
        }

        /// <summary>
        /// Load layouts configuration from the file stored in test root directory (appearenceVSVerion.vssettings)
        /// </summary>
        public void LoadLayots()
        {
            Trace.Indent();
            dbg.DTE2.ExecuteCommand("Tools.ImportandExportSettings", "/import:\"" + BaseDirectory.Replace(@"\\", @"\") + "\\appearence" + VSVersion + ".vssettings\"");
            Trace.Unindent();
        }
    }
}
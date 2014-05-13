using System;
using System.Collections.Generic;
using System.Collections;
using System.Text.RegularExpressions;
using System.Windows.Input;
using System.Windows.Forms;
using System.Diagnostics;using System.Drawing;
using Microsoft.VisualStudio.TestTools.UITesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UITest.Extension;
using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
using Microsoft.VisualStudio.TestTools.UITesting.WinControls;
//using Microsoft.VisualStudio.TestTools.UITesting.WpfControls;

namespace APIDebugger
{
    /// <summary>
    /// Summary description for ProbTraceTests
    /// </summary>
    [DeploymentItem(@".\data.xml"), DeploymentItem(@".\appearence10.vssettings"), DeploymentItem("appearence11.vssettings"), DeploymentItem("appearence12.vssettings"), DeploymentItem("TestSettings.testsettings")]
    [CodedUITest]
    public class ProbTraceTests
    {
        /// <summary>
        /// Function testing filtering oprions on trace view
        /// </summary>
        public ProbTraceTests()
        {


        }

        private void _PrintControlTree(UITestControl root){
            Trace.Indent();

            foreach(UITestControl tool in root.GetChildren()){
                string tmp = tool.Name + "-" + tool.ControlType;
                Trace.TraceInformation(tmp);
                _PrintControlTree(tool);
            }
            Trace.Unindent();
        }

        /// <summary>
        /// Set filter for trace view
        /// </summary>
        /// <param name="api_call">Text filter for API calls</param>
        /// <param name="showSuccessfull">If successfull API calls should be displayed</param>
        /// <param name="showErrors">If errorneous API calls should be displayed</param>
        public void _SetFilter(string api_call, bool showSuccessfull, bool showErrors)
        {
            Trace.Indent();
            UITestControl uIErrorsButton = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIToolStrip1ToolBar.UIErrorsButton;
            UITestControl uISuccessButton = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIToolStrip1ToolBar.UISuccessButton;
            WinEdit filter = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIItemWindow.UIItemEdit;

            Trace.TraceInformation("Set filter to call=" + api_call + (showSuccessfull ? ", show successfull, " : ", do not show succesfull, ") + (showErrors ? "show errors" : "do not show errors"));
            filter.Text = api_call;
            if (uIErrorsButton.State.HasFlag(ControlStates.Checked) ^ showErrors)
            {
                Trace.TraceInformation("Click show errors button.");
                UIBasicMap.MouseClick(uIErrorsButton);
            }
            if (uISuccessButton.State.HasFlag(ControlStates.Checked) ^ showSuccessfull)
            {
                Trace.TraceInformation("Click show successfull button.");
                UIBasicMap.MouseClick(uISuccessButton);
            }
            Trace.Unindent();
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@"APIApp", "APIApp")]
        public void TestTraceFilter()
        {
            bool res = true;
            int errorNum=0, successNum=0, totalNum=0;
            Trace.Indent();
            System.IO.FileStream fs_log = new System.IO.FileStream("TraceViewTestLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.DateTime };
            Trace.Listeners.Add(listener);


            UIBasicMap.LoadConfig(TestContext, "APIApp");
            UIBasicMap.LoadSolution();
            UIBasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.c", "105" } });

            UIBasicMap.ExecuteProgram();
            UIBasicMap.SelectTab("Trace View");
            _SetDisplayArgs(false);
            
            List<string> errList = new List<string>(UIProbTraceMap.api_err_calls);
            WinTable table = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UITracedataGridViewWindow.UIDataGridViewTable;
            List<string[]> trace_content = new List<string[]>();
            foreach (WinRow row in table.Rows)
            {
                trace_content.Add(row.GetChildren().GetValuesOfControls());
            }
            totalNum = trace_content.Count;
            foreach (string call in UIProbTraceMap.api_calls)
            {
                Trace.TraceInformation("Check filter for " + call);
                _SetFilter(call, true, true);

                if (table.Rows.Count==0)
                {
                    Trace.TraceError("List for " + call + " is empty.");
                    res = false;
                }
                bool errorPresent=false;
                foreach (UITestControl row in table.Rows)
                {
                    string []cells = row.GetChildren().GetValuesOfControls();
                    Trace.TraceInformation("Check row: " + string.Join("|", cells));
                    if (!cells[2].Equals("CL_SUCCESS"))
                    {
                        errorPresent=true;
                    }

                    if (!cells[1].Contains(call))
                    {
                        Trace.TraceError("Unexpected api call in trace list: " + cells[1]);
                        res = false;
                    }
                }
                if (errorPresent ^ errList.Contains(call))
                {
                    Trace.TraceError("Unexpected state of error call " + call + " expected " + errList.Contains(call) + " actual " + errorPresent);
                    res = false;
                }
            }
            _SetFilter("", false, true);
            errorNum = table.Rows.Count;
            foreach (WinRow row in table.Rows)
            {
                string[] cells = row.GetChildren().GetValuesOfControls();
                Trace.TraceInformation("Check row: " + string.Join("|", cells));
                if (cells[2].Equals("CL_SUCCESS"))
                {
                    Trace.TraceError("Unexpected successfull call: " + cells[1] + " (" + cells[2] + ")");
                    res = false;
                }
            }

            _SetFilter("", true, false);
            successNum = table.Rows.Count;
            foreach (WinRow row in table.Rows)
            {
                string[] cells = row.GetChildren().GetValuesOfControls();
                Trace.TraceInformation("Check row: " + string.Join("|", cells));
                if (!cells[2].Equals("CL_SUCCESS"))
                {
                    Trace.TraceError("Unexpected error call: " + cells[1] + " (" + cells[2] + ")");
                    res = false;
                }
            }
            if (successNum+errorNum!=totalNum)
            {
                Trace.TraceError("Number of rows after filteing does not follow expectations: " + totalNum + "~" + successNum + "+" + errorNum);
                res = false;
            }
            Trace.Unindent();
            Assert.IsTrue(res, "FAILED: TraceViewFilter test failed.");
        
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@"WorkflowApp", "WorkflowApp")]
        public void TestTraceSaveAs()
        {
            bool res = true;
            Trace.Indent();
            System.IO.FileStream fs_log = new System.IO.FileStream("TraceViewTestLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.DateTime };
            Trace.Listeners.Add(listener);


            UIBasicMap.LoadConfig(TestContext, "WorkflowApp");
            UIBasicMap.LoadSolution();
            UIBasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "238" },
                                            new string[] { @"main.cpp", "256" },
                                            new string[] { @"main.cpp", "294" }
                                            });


            UIBasicMap.ExecuteProgram();
            UIBasicMap.ExecuteProgram();
            UIBasicMap.ExecuteProgram();
            UIBasicMap.SelectTab("Trace View");

            _CheckSaveAs("Save.csv");

            Trace.Unindent();
            Assert.IsTrue(res, "FAILED: TraceViewSortAs stest failed.");

        }
        /// <summary>
        /// Set filters for problems view tab
        /// </summary>
        /// <param name="showWarnings"> if warnings should be displayed</param>
        /// <param name="showErrors"> if errors should be displayed</param>
        private void _SetWarnErr(bool showWarnings, bool showErrors)
        {
            Trace.Indent();
            UITestControl uIErrorsButton = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIToolStrip1ToolBar.UIItemErrorsButton;
            UITestControl uIWarningsButton = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIToolStrip1ToolBar.UIItemWarningsButton;

            UIBasicMap.SelectTab("Problems View");
            Trace.TraceInformation("Set filter for problems view: " + (showWarnings ? ", show warnings, " : ", do not show warnings, ") + (showErrors ? "show errors" : "do not show errors"));
            if (uIErrorsButton.State.HasFlag(ControlStates.Checked) ^ showErrors)
            {
                Trace.TraceInformation("Click show errors button.");
                UIBasicMap.MouseClick(uIErrorsButton);
            }
            if (uIWarningsButton.State.HasFlag(ControlStates.Checked) ^ showWarnings)
            {
                Trace.TraceInformation("Click show successfull button.");
                UIBasicMap.MouseClick(uIWarningsButton);
            }
            Trace.Unindent();
        }
        
        /// <summary>
        /// Check content of table
        /// </summary>
        /// <param name="expected_lines">LIst of strings containing regular expressions descriving lines in the table</param>
        /// <returns>return true if all expected lines have been found, fale overwise</returns>
        private bool _checkTableContent(string [] expected_lines)
        {
            bool res=true;
            Trace.Indent();
            WinTable uIDataProblemsViewTable = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIProblemsDataGridViewWindow.UIDataGridViewTable;
            
            foreach (string reg_string in expected_lines)
            {
                Trace.TraceInformation("Looking for " + reg_string);
                Regex reg = new Regex(reg_string);
                bool found = false;
                foreach (WinRow row in uIDataProblemsViewTable.Rows)
                {
                    string[] cells = row.GetChildren().GetValuesOfControls();
                    Trace.TraceInformation("Check " + string.Join("|", cells));
                    if (Regex.Match(cells[2], reg_string, RegexOptions.IgnoreCase).Success)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    Trace.TraceError("Required expression has not been found: "+reg_string);
                    res = false;
                }
            }
            Trace.Unindent();
            return res;
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@"ProblemApp", "ProblemApp")]
        public void TestProblemFilter()
        {
            bool res = true,subres;
            Trace.Indent();
            System.IO.FileStream fs_log = new System.IO.FileStream("ProblemViewTestLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.DateTime };
            Trace.Listeners.Add(listener);

            WinTable uIDataProblemsViewTable = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIProblemsDataGridViewWindow.UIDataGridViewTable;

            UIBasicMap.LoadConfig(TestContext, "ProblemApp");
            UIBasicMap.LoadSolution();
            UIBasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "166" },
                                            new string[] { @"main.cpp", "185" },
                                            new string[] { @"main.cpp", "366" }
                                            });


            UIBasicMap.ExecuteProgram();
            UIBasicMap.SelectTab("Problems view");
            _SetWarnErr(true, true);
            
            subres=_checkTableContent( new string[] {
                        @"clGetDeviceIDs API call #[0-9]* failed with CL_DEVICE_NOT_FOUND. Right-click here to show the trace.",
                        @"clCreateCommandQueue API call #[0-9]* failed with CL_INVALID_VALUE. Right-click here to show the trace.",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"Failed to compile Program \[1\] for Device \[1\] \(.*\). Double-click here to see the log.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"Kernel \[1\] \(QueueKernel\)'s argument number 0 is not initialized yet",
                        @"Kernel \[1\] \(QueueKernel\)'s argument number 1 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 0 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 1 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 2 is not initialized yet",
                        @"Program \[4\] was compiled with warnings",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace.",
                        @"Kernel \[3\] \(QueueKernel\)'s argument number 0 is not initialized yet",
                        @"Kernel \[3\] \(QueueKernel\)'s argument number 1 is not initialized yet",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace." });
            if (!subres) res = false;
            _SetWarnErr(false, false);
            if (uIDataProblemsViewTable.Rows.Count != 0) 
            {
                Trace.TraceError("Some problems are displayed while errors and warnings must be hidden");
                res = false;
            }
            
            UIBasicMap.ExecuteProgram();
            UIBasicMap.SelectTab("Problems View");
            _SetWarnErr(true, false);

            subres = _checkTableContent(new string[] {
                        @"Kernel \[1\] \(QueueKernel\)'s argument number 1 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 0 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 1 is not initialized yet",
                        @"Kernel \[2\] \(OverLap\)'s argument number 2 is not initialized yet",
                        @"Program \[4\] was compiled with warnings",
                        @"Kernel \[3\] \(QueueKernel\)'s argument number 0 is not initialized yet",
                        @"Kernel \[3\] \(QueueKernel\)'s argument number 1 is not initialized yet" });
            if (!subres) res = false;

            UIBasicMap.ExecuteProgram();
            UIBasicMap.SelectTab("Problems View");
            _SetWarnErr(false,true);

            subres = _checkTableContent(new string[] {
                        @"clGetDeviceIDs API call #[0-9]* failed with CL_DEVICE_NOT_FOUND. Right-click here to show the trace.",
                        @"clCreateCommandQueue API call #[0-9]* failed with CL_INVALID_VALUE. Right-click here to show the trace.",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"Failed to compile Program \[1\] for Device \[1\] \(.*\). Double-click here to see the log.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace.",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace." });
            if (!subres) res = false;

            _SetWarnErr(true, true);

            subres = _checkTableContent(new string[] {
                        @"clGetDeviceIDs API call #[0-9]* failed with CL_DEVICE_NOT_FOUND. Right-click here to show the trace.",
                        @"clCreateCommandQueue API call #[0-9]* failed with CL_INVALID_VALUE. Right-click here to show the trace.",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"Failed to compile Program \[1\] for Device \[1\] \(.*\). Double-click here to see the log.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"Program \[4\] was compiled with warnings",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace.",
                        @"clBuildProgram API call #[0-9]* failed with CL_BUILD_PROGRAM_FAILURE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_PROGRAM_EXECUTABLE. Right-click here to show the trace.",
                        @"clCreateKernel API call #[0-9]* failed with CL_INVALID_KERNEL_NAME. Right-click here to show the trace.",
                        @"Buffer \[1\] was released before SubBuffer \[1\] with SubBuffer \[1\] having Reference Count = 1."});
            if (!subres) res = false; 
            
            Trace.Unindent();
            Assert.IsTrue(res, "FAILED: TraceViewSortAs stest failed.");

        }
        /// <summary>
        /// Check lines which expect to have compiler or link messages
        /// </summary>
        /// <param name="row_mask">Mask for row to be changed</param>
        /// <param name="msgs">Array of messages to be found</param>
        /// <returns>returns true if expected messages have been found, false overwise</returns>
        private bool _CheckBuildMessages(string row_mask, string [] msgs){
            bool res = true;
            Trace.Indent();
            UIBasicMap.SelectTab("Problems view");
            Trace.TraceInformation("Check row_mask: " + row_mask + ", msgs=" + string.Join("|", msgs));
            WinTable uIDataProblemsViewTable = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIProblemsDataGridViewWindow.UIDataGridViewTable;
            foreach (WinRow row in uIDataProblemsViewTable.Rows)
            {
                string[] cells = row.GetChildren().GetValuesOfControls();
                Trace.TraceInformation("Check row: " + string.Join("|", cells));
                if (Regex.Match(cells[2], row_mask, RegexOptions.IgnoreCase).Success)
                {
                    Trace.TraceInformation("Row matching mask has been found...");
                    UIBasicMap.MouseClick(row, MouseButtons.Left, ModifierKeys.None, true);
                    break;
                }
            }
            Keyboard.SendKeys(UIProbTraceMap.UITestAppDebuggingMicrWindow, "a", ModifierKeys.Control);
            Keyboard.SendKeys(UIProbTraceMap.UITestAppDebuggingMicrWindow, "c", ModifierKeys.Control);
            string details = Clipboard.GetText().Replace("\r", "");
            Trace.TraceInformation("Detailed info is: "+details);
            foreach (string msg in msgs)
            {
                Trace.TraceInformation("Looking for "+msg);
                if (!Regex.Match(details, msg, RegexOptions.IgnoreCase).Success)
                {
                    Trace.TraceError("Failed to find "+msg);
                    res= false;
                }
            }
            Trace.Unindent();
            return res;
        }
        /// <summary>
        /// Check that problems are correctly forwarded to trace view
        /// </summary>
        /// <param name="str_num">Expected call number</param>
        /// <param name="call">API call name</param>
        /// <param name="ret_val">Return value of API call</param>
        /// <returns></returns>
        private bool _CheckTraceLine(string str_num, string call, string ret_val)
        {
            bool res = true,found = false;
            Trace.Indent();
            WinTable table = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UITracedataGridViewWindow.UIDataGridViewTable;
            foreach (WinRow row in table.Rows)
            {
                string[] cells = row.GetChildren().GetValuesOfControls();
                Trace.TraceInformation("Check: " + string.Join("|", cells));
                if (row.State.HasFlag(ControlStates.Selected))
                {
                    _SetFilter("",true,true);
                    if (!cells[0].Equals(str_num) || !cells[1].Contains(call) || !cells[2].Equals(ret_val))
                    {
                        Trace.TraceError("Unexpected data expected data("+str_num+", "+call+", "+ret_val+")");
                        res = false;
                    }
                    found = true;
                }
            }
            if(!found)Trace.TraceError("Selected line has not been found"); 
            Trace.Unindent();
            return res&found;
        }
        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@"ProblemApp", "ProblemApp")]
        public void TestProblemCorrel()
        {
            bool res = true,subres;
            Trace.Indent();
            System.IO.FileStream fs_log = new System.IO.FileStream("ProblemViewTestLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.DateTime };
            Trace.Listeners.Add(listener);

            UIBasicMap.LoadConfig(TestContext, "ProblemApp");
            UIBasicMap.LoadSolution();
            UIBasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "166" },
                                            new string[] { @"main.cpp", "185" },
                                            new string[] { @"main.cpp", "366" }
                                            });


            UIBasicMap.ExecuteProgram();
            _SetWarnErr(true, true);

            subres=_CheckBuildMessages(@"Failed to compile Program \[1\] for Device", new string[] { "error: type specifier missing", "error: read-only variable is not assignable" });
            if (!subres) res = subres;
            subres = _CheckBuildMessages(@"Program \[4\] was compiled with warnings", new string[] { "warning: expected 'enable' or 'disable' - ignoring", "warning: unknown OpenCL extension 'bla_bla_bla' - ignoring" });
            if (!subres) res = subres;
            /* Uncomment when CSSD100018957 is fixed  
            subres = _CheckBuildMessages(@"clBuildProgram API call #41 failed with CL_BUILD_PROGRAM_FAILURE.", new string[] { "unimplemented function(s) used" });
            if (!subres) res = subres;*/

            WinTable uIDataProblemsViewTable = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIProblemsviewTabPage.UIProblemsviewPane.UIProblemsviewPane1.UIProblemsDataGridViewWindow.UIDataGridViewTable;
            foreach (WinRow row in uIDataProblemsViewTable.Rows)
            {
                UIBasicMap.SelectTab("Problems View");
                string[] cells = row.GetChildren().GetValuesOfControls();
                Trace.TraceInformation("Process row: " + string.Join("|",cells));
                Match match = Regex.Match(cells[2], @"^(cl[a-z,A-Z]*) API call #([0-9]*) failed with (CL_[A-Z,_]*)\.", RegexOptions.IgnoreCase);
                if (match.Success) 
                {
                    UIBasicMap.MouseClick(row, MouseButtons.Right, ModifierKeys.None);
                    UIBasicMap.MouseClick(UIProbTraceMap.UIItemWindow.UIDropDownMenu.GetChildren()[0]);
                    subres = _CheckTraceLine(match.Groups[2].Value, match.Groups[1].Value, match.Groups[3].Value);
                    if (!subres) 
                        res = subres;
                }
                else
                {
                    Trace.TraceInformation("No correlation for this row, skip.");
                }
    
            }
            Trace.Unindent();
            Assert.IsTrue(res, "FAILED: TraceViewCorrel stest failed.");
        }

        /// <summary>
        /// Set if OpenCl calls are displayed with arguments
        /// </summary>
        /// <param name="display">If arguments to be displayed</param>
        private void _SetDisplayArgs(bool displayArgs)
        {
            Trace.Indent();
            WinMenuItem uIFunctionOnly = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIToolStrip1ToolBar.UIAPIDisplayModeMenuItem.UIFunctionnamesonlyMenuItem;
            WinMenuItem uIFunctionAndArgs = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIToolStrip1ToolBar.UIAPIDisplayModeMenuItem.UIFunctionswithargumenMenuItem;
            if (displayArgs)
            {
                Trace.TraceInformation("Function arguments need to be displayed");
                if (uIFunctionOnly.Checked) uIFunctionAndArgs.Checked = true;
            }
            else
            {
                Trace.TraceInformation("Function arguments must not be displayed");
                if (uIFunctionAndArgs.Checked) uIFunctionOnly.Checked = true;
            }
            Trace.Unindent();
        }
        /// <summary>
        /// Check is data is correctly saved to CSV file
        /// </summary>
        /// <param name="filename">Name of SCV file relative to base directory</param>
        private void _CheckSaveAs(string filename)
        {
            #region Variable Declarations
            WinComboBox uIFilenameComboBox = UIProbTraceMap.UISaveTraceWindow.UIDetailsPanePane.UIFilenameComboBox;
            WinButton uIYesButton = UIProbTraceMap.UIConfirmSaveAsWindow.UIConfirmSaveAsPane.UIYesButton;
            WinMenuItem uISaveAsCSV = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UIToolStrip1ToolBar.UISaveMenuItem.UIAPICallsasCSVcsvMenuItem;
            WinTable uITable = UIProbTraceMap.UITestAppDebuggingMicrWindow.UIItemTabList.UITraceViewTabPage.UITraceViewPane.UITraceViewPane1.UITracedataGridViewWindow.UIDataGridViewTable;
            #endregion
            bool res = true;
            Trace.Indent();
            _SetDisplayArgs(false);
            // Click 'Save As...' button
            UIBasicMap.MouseClick(uISaveAsCSV);

            // Select 'test' in 'File name:' combo box
            uIFilenameComboBox.EditableItem = UIBasicMapClasses.UIBasicMap.BaseDirectory.Replace(@"\\", @"\");

            // Type '{Enter}' in 'File name:' text box
            Keyboard.SendKeys( uIFilenameComboBox, "{Enter}", ModifierKeys.None);

            // Select 'test' in 'File name:' combo box
            uIFilenameComboBox.EditableItem = filename;

            // Type '{Enter}' in 'File name:' text box
            Keyboard.SendKeys( uIFilenameComboBox, "{Enter}", ModifierKeys.None);

            // Click '&Yes' button if corresponding dialog appeared
            if (uIYesButton.TryFind())
            {
                UIBasicMap.MouseClick(uIYesButton);
            }
            Hashtable TraceContent = new Hashtable();
            foreach (WinRow row in uITable.Rows)
            {
                string[] cells = row.GetChildren().GetValuesOfControls();
                TraceContent.Add(cells[0], new string[] { cells[1], cells[2], cells[3] });
            }

            string[] lines = System.IO.File.ReadAllLines(UIBasicMapClasses.UIBasicMap.BaseDirectory.Replace(@"\\", @"\") + @"\" + filename);
            foreach (string line in lines)
            {
                string []tokens = line.Split(new char[]{','});
                if (!tokens[0].Equals("\"ID (#)\""))
                {
                    if (!tokens[1].Equals(((string[])TraceContent[tokens[0]])[0])||
                        !tokens[2].Equals(((string[])TraceContent[tokens[0]])[1])||
                        !tokens[3].Equals(((string[])TraceContent[tokens[0]])[2]))
                    {
                        Trace.TraceError("Values in file (" + string.Join("|", (string[])TraceContent[tokens[0]]) + ") do not correspond to values in trace view (" + string.Join("|",tokens) + ")");
                        res = false;
                    }
                }

            }

            Trace.Unindent();
            Assert.IsTrue(res, "FAILED: TraceViewSaveAs test failed.");
        }


        #region Additional test attributes

        // You can use the following additional attributes as you write your tests:

        ////Use TestInitialize to run code before running each test 
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{        
        //    // To generate code for this test, select "Generate Code for Coded UI Test" from the shortcut menu and select one of the menu items.
        //}

        /// <summary>
        /// Clean up after test execution 
        /// </summary>
        [TestCleanup()]
        public void ProbTraceTestCleanup()
        {
            UIBasicMap.CloseVS();
        }

        #endregion

        /// <summary>
        ///Gets or sets the test context which provides
        ///information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }
        private TestContext testContextInstance;

        public UIBasicMapClasses.UIBasicMap UIBasicMap
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

        public ProbTraceMapClasses.ProbTraceMap UIProbTraceMap
        {
            get
            {
                if ((this._ProbTraceMap == null))
                {
                    this._ProbTraceMap = new ProbTraceMapClasses.ProbTraceMap();
                }

                return this._ProbTraceMap;
            }
        }

        private ProbTraceMapClasses.ProbTraceMap _ProbTraceMap;
    }

}

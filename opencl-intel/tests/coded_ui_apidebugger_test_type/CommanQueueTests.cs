using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Windows.Input;
using System.Windows.Forms;
using System.Drawing;
using Microsoft.VisualStudio.TestTools.UITesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UITest.Extension;
using Microsoft.VisualStudio.TestTools.UITesting.WinControls;
using Microsoft.VisualStudio.TestTools.UITesting.WpfControls;
using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
using System.Diagnostics;
using APIDebugger.UIBasicMapClasses;

namespace APIDebugger
{
    /// <summary>
    /// Summary description for CommanQueue
    /// </summary>
    [CodedUITest]
    [DeploymentItem(@".\data.xml"), DeploymentItem(@".\appearence10.vssettings"), DeploymentItem("appearence11.vssettings"), DeploymentItem("appearence12.vssettings"), DeploymentItem("TestSettings.testsettings")]
    public class CommanQueueTests
    {
        public CommanQueueTests()
        {
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestCommandQueueSaveAs()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("QueueSaveAsTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime };
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "238" },
                                            new string[] { @"main.cpp", "261" },
                                            new string[] { @"main.cpp", "298" }
                                            });
            BasicMap.ExecuteProgram();

            _CheckSaveAs(new string[] { "FILL_IMAGE(5)" },
                            "single_cmd.txt", true, null);

            BasicMap.ExecuteProgram();
            _CheckSaveAs(new string[] {     "FILL_IMAGE(4)",
                                            "NDRANGE_KERNEL(1)", 
                                            "NDRANGE_KERNEL(0)", 
                                            "READ_BUFFER(9)" },
                            "q1_cmds_asc.txt", true, new string[] { "CommandQueue [1]", "OOO" });

            BasicMap.ExecuteProgram();
            _CheckSaveAs(  new string[] {   "READ_BUFFER(10)",
                                            "MAP_IMAGE(8)",
                                            "NDRANGE_KERNEL(7)",
                                            "FILL_IMAGE(6)",
                                            "NDRANGE_KERNEL(3)",
                                            "NDRANGE_KERNEL(2)", 
                                            "READ_BUFFER(9)", 
                                            "NDRANGE_KERNEL(0)",
                                            "NDRANGE_KERNEL(1)", 
                                            "FILL_IMAGE(4)", 
                                            "FILL_IMAGE(5)"},
                            "all_cmd_des.txt", false, null);
            _CheckSaveAs(new string[] {    "NDRANGE_KERNEL(2)",
                                           "NDRANGE_KERNEL(3)",
                                           "READ_BUFFER(10)" },
                "q3_cmd_asc.txt", true, new string[] { "CommandQueue [3]", "In Order" });
            Trace.Unindent();
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestCommandQueueSort()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("QueueSortTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime }; 
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "209" },
                                            new string[] { @"main.cpp", "238" },
                                            new string[] { @"main.cpp", "261" },
                                            new string[] { @"main.cpp", "298" }
                                            });
            BasicMap.ExecuteProgram();

            BasicMap.SelectTab("Command Queue");

            WinList uIListBoxEnqueuedCommaList = (WinList)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIListBoxEnqueuedCommaWindow, "List");

            WinList uIListBoxCompletedCommList = (WinList)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIListBoxCompletedCommWindow, "List");

            _CheckListOrder(new string[] {  "NDRANGE_KERNEL(0)", 
                                            "NDRANGE_KERNEL(1)", 
                                            "NDRANGE_KERNEL(2)", 
                                            "NDRANGE_KERNEL(3)"},
                            uIListBoxEnqueuedCommaList,
                            true,
                            null);
            BasicMap.ExecuteProgram();
            _CheckListOrder(new string[] {  "READ_BUFFER(9)",
                                            "FILL_IMAGE(4)",
                                            "NDRANGE_KERNEL(1)", 
                                            "NDRANGE_KERNEL(0)" },
                            uIListBoxEnqueuedCommaList, false,
                            new string[]{"CommandQueue [1]" , "OOO"});

            BasicMap.ExecuteProgram();
            _CheckListOrder(new string[] {  "FILL_IMAGE(4)",
                                            "NDRANGE_KERNEL(1)",
                                            "NDRANGE_KERNEL(0)",
                                            "READ_BUFFER(9)" },
                            uIListBoxCompletedCommList, true,
                            new string[]{"CommandQueue [1]", "OOO"});
            BasicMap.ExecuteProgram();

            _CheckListOrder(new string[] {  "READ_BUFFER(10)",
                                            "MAP_IMAGE(8)",
                                            "NDRANGE_KERNEL(7)",
                                            "FILL_IMAGE(6)",
                                            "NDRANGE_KERNEL(3)",
                                            "NDRANGE_KERNEL(2)", 
                                            "READ_BUFFER(9)", 
                                            "NDRANGE_KERNEL(0)",
                                            "NDRANGE_KERNEL(1)", 
                                            "FILL_IMAGE(4)",
                                            "FILL_IMAGE(5)",
                                            },
                            uIListBoxCompletedCommList, false,
                            null);
            _CheckListOrder(new string[] {  "FILL_IMAGE(5)",
                                            "FILL_IMAGE(6)",
                                            "NDRANGE_KERNEL(7)",
                                            "MAP_IMAGE(8)" },
                uIListBoxCompletedCommList, true,
                new string[] { "CommandQueue [2]", "In Order" });
            Trace.Unindent();
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestCommandQueueWorkflow()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("QueueWorkflowTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime };
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"images.cl", "9" },
                                            new string[] { @"main.cpp", "197" },
                                            new string[] { @"main.cpp", "199" },
                                            new string[] { @"main.cpp", "201" },
                                            new string[] { @"main.cpp", "203" },
                                            new string[] { @"main.cpp", "238" },
                                            new string[] { @"main.cpp", "244" },
                                            new string[] { @"main.cpp", "246" },
                                            new string[] { @"main.cpp", "250" },
                                            new string[] { @"main.cpp", "252" },
                                            new string[] { @"main.cpp", "256" },
                                            new string[] { @"main.cpp", "262" },
                                            new string[] { @"main.cpp", "264" },
                                            new string[] { @"main.cpp", "265" },
                                            new string[] { @"main.cpp", "268" },
                                            new string[] { @"main.cpp", "274" },
                                            new string[] { @"main.cpp", "280" },
                                            new string[] { @"main.cpp", "282" },
                                            new string[] { @"main.cpp", "293" },
                                            new string[] { @"main.cpp", "300" },
                                            new string[] { @"queues.cl", "3" }
                                            });
            BasicMap.ExecuteProgram();//main.cpp:194
            _CheckCommands(new int[] { 0, 0, 0 }, null, null);
            BasicMap.ExecuteProgram();//main.cpp:196
            _CheckCommands(new int[] { 1, 0, 0 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(0)"},
                                        null,
                                        null},
                                    null);
            BasicMap.ExecuteProgram();//main.cpp:198
            _CheckCommands(new int[] { 2, 0, 0 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(0)","NDRANGE_KERNEL(1)"},
                                        null,
                                        null},
                                        new string[]{"CommandQueue [1]", "OOO"});
            BasicMap.ExecuteProgram();//main.cpp:200
            _CheckCommands(new int[] { 1, 0, 0 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(2)"},
                                        null,
                                        null},
                                    new string[] { "CommandQueue [3]", "In Order" });
            BasicMap.ExecuteProgram();//main.cpp:233
            _CheckCommands(new int[] { 3, 0, 1 },
                                    new string[][] { 
                                        new string[] { "FILL_IMAGE(6)", "NDRANGE_KERNEL(7)", "MAP_IMAGE(8)"},
                                        null,
                                        new string[] { "FILL_IMAGE(5)"},},
                                   new string[] { "CommandQueue [2]", "In Order" });
            BasicMap.ExecuteProgram();//main.cpp:239
            _CheckCommands(new int[] { 3, 0, 1 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(0)","NDRANGE_KERNEL(1)", "READ_BUFFER(9)"},
                                        null,
                                        new string[] {"FILL_IMAGE(4)"}},
                                   new string[] { "CommandQueue [1]", "OOO" });
            string[] device_types = CommandQueueMap.GetDeviceTypes();
            // kernelDebugger does not work on GPU or MIC
            if (device_types[0].Equals("CPU"))
            {
                BasicMap.ExecuteProgram();//queues.cl:3
                _CheckCommands(new int[] { 2, 1, 1 },
                                        new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(0)", "READ_BUFFER(9)" },
                                        new string[] { "NDRANGE_KERNEL(1)" },
                                        new string[] {"FILL_IMAGE(4)"} },
                                    new string[] { "CommandQueue [1]", "OOO" });
                BasicMap.ExecuteProgram();//main.cpp:241
            }
            BasicMap.ExecuteProgram();//main.cpp:245
            _CheckCommands(new int[] { 2, 0, 2 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(0)", "READ_BUFFER(9)" },
                                        null,
                                        new string[] { "FILL_IMAGE(4)","NDRANGE_KERNEL(1)" }},
                                   new string[] { "CommandQueue [1]", "OOO" });
            // kernelDebugger does not work on GPU
            if (device_types[0].Equals("CPU"))
            {
                BasicMap.ExecuteProgram();//queues.cl:3
                _CheckCommands(new int[] { 1, 1, 2 },
                                    new string[][] { 
                                        new string[] { "READ_BUFFER(9)" },
                                        new string[] { "NDRANGE_KERNEL(0)" },
                                        new string[] { "FILL_IMAGE(4)","NDRANGE_KERNEL(1)" }},
                                   new string[] { "CommandQueue [1]", "OOO" });
                BasicMap.ExecuteProgram();
            }
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 5, 0, 5 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(2)", "NDRANGE_KERNEL(3)","FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        null,   
                                        new string[] { "FILL_IMAGE(5)","FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                   null);
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 5, 0, 5 },
                                    new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(2)", "NDRANGE_KERNEL(3)","FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        null,
                                        new string[] { "FILL_IMAGE(5)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                   null);

            // kernelDebugger does not work on GPU and MIC
            if (device_types[2].Equals("CPU"))
            {
                BasicMap.ExecuteProgram();
                _CheckCommands(new int[] { 4, 1, 5 },
                                        new string[][] { 
                                        new string[] { "NDRANGE_KERNEL(3)","FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        new string[] { "NDRANGE_KERNEL(2)" },
                                        new string[] { "FILL_IMAGE(5)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                       null);
                BasicMap.ExecuteProgram();
                BasicMap.ExecuteProgram();
                _CheckCommands(new int[] { 0, 1, 1 },
                                        new string[][] { 
                                        null,
                                        new string[] { "NDRANGE_KERNEL(3)" },
                                        new string[] { "NDRANGE_KERNEL(2)" }},
                                       new string[] { "CommandQueue [3]", "In Order" });
            }
            BasicMap.ExecuteProgram();
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 3, 0, 7 },
                                    new string[][] { 
                                        new string[] { "FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        null,
                                        new string[] { "FILL_IMAGE(5)","NDRANGE_KERNEL(3)", "NDRANGE_KERNEL(2)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                   null);
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 3, 0, 7 },
                                    new string[][] { 
                                        new string[] { "FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        null,
                                        new string[] { "FILL_IMAGE(5)", "NDRANGE_KERNEL(3)", "NDRANGE_KERNEL(2)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                   null);
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 3, 0, 1 },
                                    new string[][] { 
                                        new string[] { "FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)" },
                                        null,
                                        new string[] {  "FILL_IMAGE(5)" }},
                                   new string[] { "CommandQueue [2]", "In Order" });
            BasicMap.ExecuteProgram();
            // kernelDebugger does not work on GPU and MIC
            if (device_types[1].Equals("CPU"))
            {
                BasicMap.ExecuteProgram();
                _CheckCommands(new int[] { 1, 1, 2 },
                                    new string[][] { 
                                        new string[] {"MAP_IMAGE(8)" },
                                        new string[] { "NDRANGE_KERNEL(7)" },
                                        new string[] { "FILL_IMAGE(5)", "FILL_IMAGE(6)" }},
                                   new string[] { "CommandQueue [2]", "In Order" });
                BasicMap.ExecuteProgram();
            }
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 0, 0, 10 },
                                   new string[][] { 
                                        null,
                                        null,
                                        new string[] { "FILL_IMAGE(5)","FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)","NDRANGE_KERNEL(3)", "NDRANGE_KERNEL(2)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)" }},
                                  null);
            BasicMap.ExecuteProgram();
            _CheckCommands(new int[] { 0, 0, 11 },
                                   new string[][] { 
                                        null,
                                        null,
                                        new string[] { "FILL_IMAGE(5)","FILL_IMAGE(6)","NDRANGE_KERNEL(7)","MAP_IMAGE(8)","NDRANGE_KERNEL(3)", "NDRANGE_KERNEL(2)", "FILL_IMAGE(4)","NDRANGE_KERNEL(0)", "NDRANGE_KERNEL(1)", "READ_BUFFER(9)", "READ_BUFFER(10)" }},
                                  null);
            Trace.Unindent();
        }

        /// <summary>
        /// Check command lists
        /// </summary>
        /// <param name="num">array containing expected number of commands in queues </param>
        /// <param name="values">array containing commands in queues</param>
        /// <param name="queue">queue filter</param>
        private void _CheckCommands(int[] num, string[][] values, string []queue)
        {
            Trace.Indent();
            BasicMap.SelectTab("Command Queue");

            WinList uIListBoxEnqueuedCommaList = (WinList)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIListBoxEnqueuedCommaWindow,"List");
            WinList uIListBoxRunningCommanList = (WinList)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIListBoxRunningCommanWindow,"List");
            WinList uIListBoxCompletedCommList = (WinList)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIListBoxCompletedCommWindow,"List");

            _SelectQueue(queue);

            Trace.TraceInformation("If number of command in specific queue must be checked...");
            if (num != null)
            {
                // Check Enqueued list
                Assert.AreEqual(num[0], uIListBoxEnqueuedCommaList.GetChildren().Count, "Submitted list is in incorrect state");

                // Check Running list
                Assert.AreEqual(num[1], uIListBoxRunningCommanList.GetChildren().Count, "Running list is in incorrect state");

                // Check Completed list
                Assert.AreEqual(num[2], uIListBoxCompletedCommList.GetChildren().Count, "Completed list is in incorrect state");
            }

            Trace.TraceInformation("If commands in specific queue must be checked...");
            if (values != null)
            {
                if (values[0] != null)
                {
                    Trace.TraceInformation("Check content of submitted list.");
                    // Check Enqueued list
                    _CheckValuesInList(values[0], uIListBoxEnqueuedCommaList.Items, "submitted");
                }
                if (values[1] != null)
                {
                    Trace.TraceInformation("Check content of running list.");
                    _CheckValuesInList(values[1], uIListBoxRunningCommanList.Items, "running");
                }
                if (values[2] != null)
                {
                    Trace.TraceInformation("Check content of completed list.");
                    _CheckValuesInList(values[2], uIListBoxCompletedCommList.Items, "completed");
                }
            }

        }

        /// <summary>
        /// Check values in lists reprsending different command queues
        /// </summary>
        /// <param name="values">List fo values to be present</param>
        /// <param name="collection">Collection of items in the list</param>
        /// <param name="name">Nam of the queue which is checked</param>
        private void _CheckValuesInList(string[] values, UITestControlCollection collection, string name) 
        {
            Trace.TraceInformation("Looking for " + string.Join(",", values) + " values in the " + string.Join(",", name) + "list.");
            foreach (string val in values)
            {
                Trace.TraceInformation("Look for " + val + " in the "+name+" list");
                bool found_flag = false;
                foreach (UITestControl item in collection)
                {

                    if (item.ControlType == "ListItem")
                    {
                        WinListItem tmp = ((WinListItem)item);
                        Trace.TraceInformation("Check " + tmp.DisplayText);
                        if (tmp.DisplayText == val)
                        {
                            Trace.TraceInformation("BINGO!!!");
                            found_flag = true;
                            break;
                        }
                    }
                }
                if (!found_flag)
                    Assert.Fail("Expected string '" + val + "' has not been found in the "+name+" list");
            }

            Trace.Unindent();
        }
        /// <summary>
        /// Check that items in lists are in expected order
        /// </summary>
        /// <param name="items">List of command in expected order</param>
        /// <param name="component">List component which must be checked</param>
        /// <param name="asc">Sort order</param>
        /// <param name="queue">Filter queue</param>
        private void _CheckListOrder(string[] items, WinList component, bool asc, string[] queue)
        {
            Trace.Indent();
            BasicMap.SelectTab("Command Queue");

            _SelectOrder(asc);
            _SelectQueue(queue);

            List<string> list = new List<string>();
            foreach (UITestControl item in component.Items)
            {
                Trace.TraceInformation("Reading comamnd from queue list: " + ((WinListItem)item).DisplayText + " of type " + item.ControlType);
                if (item.ControlType == "ListItem")
                {
                    list.Add(((WinListItem)item).DisplayText);
                }
            }

            Assert.AreEqual(string.Join(",", items), string.Join(",", list.ToArray()), "The list does not follow expectations ");
            Trace.Unindent();
        }

        /// <summary>
        /// Check save as options
        /// </summary>
        /// <param name="items">List of items in queue</param>
        /// <param name="filename">Name of file to store data</param>
        /// <param name="asc">Sorting order</param>
        /// <param name="queue">Queue filter</param>
        private void _CheckSaveAs(string[] items, string filename, bool asc, string[] queue)
        {
            #region Variable Declarations
            WinButton uISaveAsButton = CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UISaveAsButton;
            WinComboBox uIFilenameComboBox = CommandQueueMap.UISaveCompletedCommandWindow.UIDetailsPanePane.UIFilenameComboBox;
            WinEdit uIFilenameEdit = CommandQueueMap.UISaveCompletedCommandWindow.UIItemWindow.UIFilenameEdit;
            WinButton uINOButton = CommandQueueMap.UIConfirmSaveAsWindow.UIConfirmSaveAsPane.UINOButton;
            WinButton uIYesButton = CommandQueueMap.UIConfirmSaveAsWindow.UIConfirmSaveAsPane.UIYesButton;
            #endregion

            Trace.Indent();

            BasicMap.SelectTab("Command Queue");

            _SelectOrder(asc);
            _SelectQueue(queue);

            // Click 'Save As...' button
            BasicMap.MouseClick(uISaveAsButton);

            // Select 'test' in 'File name:' combo box
            uIFilenameComboBox.EditableItem = UIBasicMapClasses.UIBasicMap.BaseDirectory.Replace(@"\\", @"\");

            // Type '{Enter}' in 'File name:' text box
            Keyboard.SendKeys(uIFilenameEdit, "{Enter}", ModifierKeys.None);

            // Select 'test' in 'File name:' combo box
            uIFilenameComboBox.EditableItem = filename;

            // Type '{Enter}' in 'File name:' text box
            Keyboard.SendKeys(uIFilenameEdit, "{Enter}", ModifierKeys.None);

            // Click '&Yes' button if corresponding dialog appeared
            if (uIYesButton.TryFind())
            {
                BasicMap.MouseClick(uIYesButton);
            }


            string[] lines = System.IO.File.ReadAllLines(UIBasicMapClasses.UIBasicMap.BaseDirectory.Replace(@"\\", @"\") + @"\" + filename);

            Assert.AreEqual(string.Join(",", items), string.Join(",", lines), "Saved file does not follow expectations.");
            Trace.Unindent();
        }

        /// <summary>
        /// Select queue to be filtered
        /// </summary>
        /// <param name="queue">Queue name</param>
        private void _SelectQueue(string []queue)
        {
            #region Variable Declarations
            WinButton uIUnifyQueuesButton = CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UIUnifyQueuesButton;
            WinButton uISeperateQueuesButton = CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UISeperateQueuesButton;
            WinComboBox uIComboBoxQueueComboBox = (WinComboBox)BasicMap.GetChild(CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIComboBoxQueueWindow, "ComboBox");
            #endregion

            Trace.Indent();

            Trace.TraceInformation("Check if specific queue should be chosen");
            if (queue == null)
            {
                Trace.TraceInformation("Queue is not specified, check for all queues.");
                if (uIUnifyQueuesButton.TryFind())
                {
                    Trace.TraceInformation("Select unified queue.");
                    // Click 'Unify Queues' label
                    BasicMap.MouseClick(uIUnifyQueuesButton);
                }
                else
                {
                    Trace.TraceInformation("Unified queue has been already selected.");
                }
            }
            else
            {
                Trace.TraceInformation("Queue " + string.Join(",", queue) + " is specified, check only it.");
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

                bool found=false;

                Trace.TraceInformation("Select queue if necessary");
                string items="";
                foreach (UITestControl item in uIComboBoxQueueComboBox.Items)
                {
                    bool matches = true;
                    items += "," + item.FriendlyName;
                    Trace.TraceInformation("Check item " + item.FriendlyName);
                    foreach (string substring in queue)
                    {
                        if (!item.FriendlyName.Contains(substring))matches =false;
                    }
                    if (matches)
                    {
                        Trace.TraceInformation("Select " + item.FriendlyName);
                        uIComboBoxQueueComboBox.SelectedItem = item.FriendlyName;
                        found = true;
                    }
                }
                Assert.IsTrue(found,"Requested queue has not been found: "+string.Join(",",queue));
            }
            Trace.Unindent();
        }
        /// <summary>
        /// Select order for commands in queues
        /// </summary>
        /// <param name="asc"> If ascending</param>
        private void _SelectOrder(bool asc)
        {
            #region Variable Declarations
            WinButton uISortByTimeAscendingButton = CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UISortByTimeAscendingButton;
            WinButton uISortByTimeDescendingButton = CommandQueueMap.UITestAppDebuggingMicrWindow.UIItemTabList.UICommandQueueTabPage.UICommandQueuePane.UICommandQueuePane1.UIToolStrip1ToolBar.UISortByTimeDescendingButton;
            #endregion
            Trace.Indent();
            if (asc)
            {
                Trace.TraceInformation("Current configuration must be ascending.");
                while (uISortByTimeDescendingButton.TryFind())//ensure that label has checged
                {
                    Trace.TraceInformation("trigger button as it is descending.");
                    // Click 'Sort By Time: Descending' button
                    //UIBasicMapClasses.UIBasicMap.MouseClick(uISortByTimeDescendingButton);
                    BasicMap.MouseClick(uISortByTimeDescendingButton);

                }
            }
            else
            {
                Trace.TraceInformation("Current configuration must be descending.");
                while (uISortByTimeAscendingButton.TryFind())//ensure that label has changed
                {
                    Trace.TraceInformation("trigger button as it is ascending.");
                    // Click 'Sort By Time: Descending' button
                    //UIBasicMapClasses.UIBasicMap.MouseClick(uISortByTimeAscendingButton);
                    BasicMap.MouseClick(uISortByTimeAscendingButton);
                }
            }
            Trace.Unindent();
        }

        #region Additional test attributes

        // You can use the following additional attributes as you write your tests:

        ////Use TestInitialize to run code before running each test 
        //[TestInitialize()]
        //public void MyTestInitialize()
        //{        
        //    // To generate code for this test, select "Generate Code for Coded UI Test" from the shortcut menu and select one of the menu items.
        //}

        [TestCleanup]
        public void teardown()
        {
            BasicMap.CloseVS();
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
        public CommandQueueMapClasses.CommandQueueMap CommandQueueMap
        {
            get
            {
                if ((this._CommandQueueMap == null))
                {
                    this._CommandQueueMap = new CommandQueueMapClasses.CommandQueueMap();
                }

                return this._CommandQueueMap;
            }
        }

        private CommandQueueMapClasses.CommandQueueMap _CommandQueueMap;
    }
}

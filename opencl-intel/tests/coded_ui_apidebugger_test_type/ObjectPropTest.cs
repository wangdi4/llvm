using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Windows.Input;
using System.Windows.Forms;
using System.Drawing;
using Microsoft.VisualStudio.TestTools.UITesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UITest.Extension;
using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
using APIDebugger.UIBasicMapClasses;
using Microsoft.VisualStudio.TestTools.UITesting.WinControls;
using Microsoft.VisualStudio.TestTools.UITesting.WpfControls;
using System.Linq;

namespace APIDebugger
{
    /// <summary>
    /// Summary description for CodedUITest1
    /// </summary>
    [CodedUITest]
    [DeploymentItem(@".\data.xml"), DeploymentItem(@".\appearence10.vssettings"), DeploymentItem("appearence11.vssettings"), DeploymentItem("appearence12.vssettings"), DeploymentItem("TestSettings.testsettings")]
    public class ObjectPropTest
    {
        public ObjectPropTest()
        {
        }
        /// <summary>
        /// Test context menu for program object
        /// </summary>
        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestProgObject()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("ProgObjectTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime }; 
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"common.cpp", "92" },
                                            new string[] { @"main.cpp", "342" }
                                            });
            BasicMap.ExecuteProgram();

            _SetSort(true);
            _setFilter(new string[] { "Show Programs", "Show Released Objects" });

            _CheckProgram("Program [1]", @"\WorkflowApp\queues.cl", false,false);
            
            BasicMap.ExecuteProgram();
            _CheckProgram("Program [1]", @"\WorkflowApp\queues.cl", true, false);
            _CheckProgram("Program [2]", @"\WorkflowApp\images.cl", false, false);
            BasicMap.ExecuteProgram();
            _CheckProgram("Program [2]", @"\WorkflowApp\images.cl", true, false);
            _CheckProgram("Program [3]", @"\WorkflowApp\queues.cl", false, false);
            BasicMap.ExecuteProgram();
            _CheckProgram("Program [1]", @"\WorkflowApp\queues.cl", true, true);
            _CheckProgram("Program [2]", @"\WorkflowApp\images.cl", true, true);
            _CheckProgram("Program [3]", @"\WorkflowApp\queues.cl", true, true);
            Trace.Unindent();
        }

        /// <summary>
        /// Check program submenu and context
        /// </summary>
        /// <param name="name"> name of program object to be checked</param>
        /// <param name="filename"> name of file which was used to load kernel</param>
        /// <param name="isBuilt"> if kernel has been built</param>
        /// <param name="checkReleased"> if object released</param>
        private void _CheckProgram(string name, string filename, bool isBuilt, bool checkReleased)
        {
            Trace.Indent();
            // Click 'Objects Tree' label
            BasicMap.SelectTab("Objects Tree");
            Trace.TraceInformation("Find " + name + " object isBuilt = " + isBuilt);
            bool foundBuilt=false;

            WinTree uITreeViewTree = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UITreeViewWindow.UITreeViewTree;
            foreach (UITestControl node in uITreeViewTree.Nodes)
            {
                Trace.TraceInformation("Checking "+ node.Name+" node");
                if( node.Name.Contains(name) )
                {
                    if(_CheckProgContent(node,filename)) foundBuilt=true;
                    if (foundBuilt) break;
                }
            }
            Assert.AreEqual(isBuilt, foundBuilt, "Status of program object does not follow expectation");
            Trace.Unindent();
        
        }


        /// <summary>
        /// Check context of the program object
        /// </summary>
        /// <param name="node">Node which corresponds to program</param>
        /// <param name="filename">Name of the file from which program has been created</param>
        /// <return> if there is built program object</return>
        private bool _CheckProgContent(UITestControl node, string filename)
        {
            Trace.Indent();
            bool isBuilt = node.Name.Contains("Built");
            Trace.TraceInformation("Program is expected to be built - "+isBuilt);
            BasicMap.MouseClick(node, MouseButtons.Right, ModifierKeys.None);  

            WinMenu uIDropDownMenu = UIObjectPropMap.UIItemWindow.UIDropDownMenu;
            _CheckProgMenu(uIDropDownMenu, isBuilt);
            foreach (UITestControl item in uIDropDownMenu.GetChildren())
            {
                Trace.TraceInformation("Check menu item "+ item.Name);
                if (item.Name.Equals("Open Source Code in a new tab"))
                {
                    Trace.TraceInformation("Click open source code item" + item.Name);
                    BasicMap.MouseClick(item); 
                }
            }
            //BasicMap.MouseClick(UIObjectPropMap.UITestAppDebuggingMicrWindow);
            Trace.TraceInformation("Select content of window and copy it to clipboard");
            /* Kernel content should be opened */
            Keyboard.SendKeys(UIObjectPropMap.UITestAppDebuggingMicrWindow, "a", ModifierKeys.Control);
            Keyboard.SendKeys(UIObjectPropMap.UITestAppDebuggingMicrWindow, "c", ModifierKeys.Control);

            string from_file, from_object;
            from_file = System.IO.File.ReadAllText(UIBasicMap.BaseDirectory.Replace(@"\\", @"\") + filename).Replace("\r", "");
            from_object=Clipboard.GetText().Replace("\r", "");
            Trace.TraceInformation("Kernel source from file \n" + from_file);
            Trace.TraceInformation("Kernel source from object \n" + from_object);
            Assert.AreEqual(from_file, from_object, "KernelFrom the program object is not equal to initial one");
            Keyboard.SendKeys(UIObjectPropMap.UITestAppDebuggingMicrWindow, "{F4}", ModifierKeys.Control);

            Trace.Unindent();
            return isBuilt;
        }
        /// <summary>
        /// Check that context menu is displayed correctly
        /// </summary>
        /// <param name="node"> Program object control</param>
        /// <param name="isBuilt"> If program expected to be built</param>
        /// 
        private void _CheckProgMenu(UITestControl menu, bool isBuilt)
        {
            Trace.Indent();
            string[] menu_items = new string[] { "Open Source Code in a new tab", "Save Binaries" };

            foreach (UITestControl item in menu.GetChildren())
            {
                Trace.TraceInformation("Check menu item: "+item.Name);
                Assert.IsTrue(menu_items.Contains(item.Name), "Unexpected menu item: "+ item.Name+" instead of "+string.Join(",",menu_items));
                if (item.Name.Equals(menu_items[1]))
                {
                    Trace.TraceInformation("Check that " + menu_items[1] + " correspond to object style (to be enabled=" + isBuilt + ").");
                    if (isBuilt)
                        Assert.IsTrue(item.Enabled, "Menu item is disabled while it is expected to be enabled");
                    else
                        Assert.IsFalse(item.Enabled, "Menu item is enabled while it is expected to be disabled");
                }
            }
            Trace.Unindent();
        }
        /// <summary>
        /// Check sorting of objects in 'Object Tree' view
        /// </summary>
        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestObjectSort()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("ObjectSortTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime }; 
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "135" }
                                            }); 
            BasicMap.ExecuteProgram();
            _setFilter(new string[] { "Show Devices", "Show Contexts" } );

            _CheckSort(true);
            _CheckSort(false);
            Trace.Unindent();
        }

        private void _SetSort(bool byDevice)
        {
            Trace.Indent();

            // Click 'Objects Tree' label
            BasicMap.SelectTab("Objects Tree");

            Trace.TraceInformation("Selecting correct sorting order.");
            WinMenuItem uISortbyDeviceMenuItem = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UIToolStrip1ToolBar.UISortByMenuItem.UISortbyDeviceMenuItem;
            WinMenuItem uISortbyContextMenuItem = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UIToolStrip1ToolBar.UISortByMenuItem.UISortbyContextMenuItem;
            if (byDevice && uISortbyContextMenuItem.Checked)
            {
                Trace.TraceInformation("Sorting by device should be selected");
                if (uISortbyContextMenuItem.Checked)
                {
                    Trace.TraceInformation("Selecting sort by device...");
                    // Select 'Sort By' -> 'Sort by Device' menu item
                    uISortbyDeviceMenuItem.Checked = true;
                }
                else
                {
                    Trace.TraceInformation("Sorting by device has been already selected...");
                }
            }
            else if (uISortbyDeviceMenuItem.Checked)
            {
                Trace.TraceInformation("Sorting by context should be selected");
                if (uISortbyDeviceMenuItem.Checked)
                {
                    Trace.TraceInformation("Selecting sort by device...");
                    // Select 'Sort By' -> 'Sort by Context' menu item
                    uISortbyContextMenuItem.Checked = true;
                }
                else
                {
                    Trace.TraceInformation("Sorting by context has been already selected...");
                }
            }
            Trace.Unindent();

        }

        /// <summary>
        /// Check if objects are sorted correctly
        /// </summary>
        /// <param name="byDevice">If sorted by device</param>
        private void _CheckSort(bool byDevice)
        {
            Trace.Indent();

            // Click 'Objects Tree' label
            BasicMap.SelectTab("Objects Tree");

            Trace.TraceInformation("Selecting correct sorting order.");
            _SetSort(byDevice);

            Trace.TraceInformation("Check that object tree follows expectations");
            WinTree uITreeViewTree = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UITreeViewWindow.UITreeViewTree;
            ///  check that firt level of tree is only expected
            foreach (UITestControl node in uITreeViewTree.Nodes )
            {
                if (byDevice)
                {
                    Trace.TraceInformation("Check that "+node.Name+" starts from Device");
                    Assert.IsTrue(node.Name.StartsWith("Device"),"Upper object in tree is not device object");
                    foreach(UITestControl subnode in node.GetChildren() )
                    {
                        Trace.TraceInformation("Check that " + node.Name + " starts from Context");
                        Assert.IsFalse(node.Name.StartsWith("Context"), "Lower object in tree is not context object");
                    }
                }
                else
                {
                    Trace.TraceInformation("Check that " + node.Name + " starts from Context");
                    Assert.IsTrue(node.Name.StartsWith("Context"), "Upper object in tree is not context object");
                    foreach (UITestControl subnode in node.GetChildren())
                    {
                        Trace.TraceInformation("Check that " + node.Name + " starts from Device");
                        Assert.IsFalse(node.Name.StartsWith("Device"), "Lower object in tree is not device object");
                    }
                }
            }
            Trace.Unindent();
        }

        [DataSource("Microsoft.VisualStudio.TestTools.DataSource.XML", "|DataDirectory|\\data.xml", "data", DataAccessMethod.Sequential), TestMethod]
        [DeploymentItem(@".\WorkflowApp", "WorkflowApp")]
        public void TestObjectFilter()
        {
            System.IO.FileStream fs_log = new System.IO.FileStream("ObjectFilterTraceLog.txt", System.IO.FileMode.OpenOrCreate);
            TextWriterTraceListener listener = new TextWriterTraceListener(fs_log) { TraceOutputOptions = TraceOptions.Timestamp | TraceOptions.DateTime }; 
            Trace.Listeners.Add(listener); 
            Trace.Indent();
            BasicMap.LoadConfig(TestContext, "WorkflowApp");
            BasicMap.LoadSolution();
            BasicMap.LoadBreakpoints(new string[][]{ 
                                            new string[] { @"main.cpp", "136" },
                                            new string[] { @"main.cpp", "205" },
                                            new string[] { @"main.cpp", "324" }
                                            });
            BasicMap.ExecuteProgram();
            string[] device_types = CommandQueueMap.GetDeviceTypes();

            /*Set sorting by device*/
            _SetSort(true);

            int device_num = 1;
            if (device_types[1] != "CPU") device_num++;
            if (device_types[2] != "CPU") device_num++;
            _checkFilter(new string[] { 
                                    "Show Released Objects",
                                    "Show All" },
                        new object[][]{
                                    new object []{ "Platform [1]", 1},
                                    new object []{ "Device", device_num},
                                    new object []{ "Context", 3},
                                    new object []{ "CommandQueue [1] (OOO)", 1},
                                    new object []{ "CommandQueue [2] (In Order)", 1},
                                    new object []{ "CommandQueue [3] (In Order)", 1},
                                    new object []{ "SubDevice [1] (CPU)", 1},
                                    new object []{ "SubDevice [2] (CPU)", 1}});
             

             BasicMap.ExecuteProgram();
            _checkFilter(new string[] { 
                                    "Show Released Objects",
                                    "Show All" },
                         new object[][]{
                                    new object []{ "Platform [1]", 1}, 
                                    new object []{ "Device", device_num},
                                    new object []{ "Context", 3},
                                    new object []{ "CommandQueue [1] (OOO)", 1},
                                    new object []{ "CommandQueue [2] (In Order)", 1},
                                    new object []{ "CommandQueue [3] (In Order)", 1},
                                    new object []{ "Program [1]", 1}, 
                                    new object []{ "Program [2]", 1},
                                    new object []{ "Program [3]", 1},
                                    new object []{ "Kernel [1] (QueueKernel)", 1},
                                    new object []{ "Kernel [2] (OverLap)", 1},
                                    new object []{ "Kernel [3] (QueueKernel)", 1},
                                    new object []{ "input", 2},
                                    new object []{ "output", 2},
                                    new object []{ "pconfig_src", 2},
                                    new object []{ "pconfig_dst", 1},
                                    new object []{ "Buffer", 4},
                                    new object []{ "Image", 4},
                                    new object []{ "Sampler [1]", 1},
                                    new object []{ "Event", 13},
                                    new object []{ "SubDevice ", 2},
                                    new object []{ "SubBuffer [1]", 1}
                        });
            
            _checkFilter(new string[] { 
                                    "Show Platforms",
                                    "Show Samplers",
                                    "Show Programs",
                                    "Show Kernel Arguments"},
                        new object[][]{
                                    new object []{ "Platform [1]", 1},
                                    new object []{ "Program", 3},
                                    new object []{ "input", 2},
                                    new object []{ "output", 2},
                                    new object []{ "pconfig_src",2},
                                    new object []{ "pconfig_dst", 1},
                        });
            _checkFilter(new string[] { 
                                    "Show Released Objects",
                                    "Show Samplers",
                                    "Show Events",
                                    "Show Images",
                                    "Show CommandQueues",
                                    "Show Contexts"},
                        new object[][]{
                                    new object []{ "Context", 3},
                                    new object []{ "CommandQueue [1] (OOO)", 1},
                                    new object []{ "CommandQueue [2] (In Order)", 1},
                                    new object []{ "CommandQueue [3] (In Order)", 1},
                                    new object []{ "Image", 4},
                                    new object []{ "Event", 13},
                                    new object []{ "Sampler [1]", 1}
                        });
            BasicMap.ExecuteProgram();
          
            _checkFilter(new string[] { 
                                    "Show Buffers",
                                    "Show SubBuffers",
                                    "Show Devices",
                                    "Show SubDevices",
                                    "Show Programs" },
                        new object[][]{
                                    new object []{ "Device", device_num},
                                    new object []{ "Buffer",4},
                                    new object []{ "SubBuffer [1]", 1},
                                    new object []{ "SubDevice [1] (CPU)", 1},
                                    new object []{ "SubDevice [2] (CPU)", 1}
                        });
            _checkFilter(new string[] { 
                                    "Show Released Objects",
                                    "Show Buffers",
                                    "Show SubBuffers",
                                    "Show Devices",
                                    "Show SubDevices",
                                    "Show Programs",
                                    "Show Events"},
                        new object[][]{
                                    new object []{ "Device", device_num},
                                    new object []{ "SubDevice [1] (CPU)", 1},
                                    new object []{ "SubDevice [2] (CPU)", 1},
                                    new object []{ "Buffer", 4},
                                    new object []{ "Program", 3},
                                    new object []{ "SubBuffer [1]", 1},
                                    new object []{ "Event", 17}
                        });

            _checkFilter(new string[] { 
                                    "Show Released Objects",
                                    "Show SubBuffers",
                                    "Show SubDevices"},
                         new object[][]{
                                    new object []{ "SubDevice [1] (CPU)", 1},
                                    new object []{ "SubDevice [2] (CPU)", 1},
                                    new object []{ "SubBuffer [1]", 1}
                        });
            Trace.Unindent();
        }
        /// <summary>
        /// Check filter for object tree
        /// </summary>
        /// <param name="menu_flags"> Select filter options </param>
        /// <param name="expected_objects"> List of objects which should present in tree</param>
        private void _checkFilter(string[] menu_flags, object[][] expected_objects)
        {
            Trace.Indent();
            _setFilter(menu_flags);
            int exp_obj_num = 0;

            // Check that tree contains expected elements
            WinTree uITreeViewTree = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UITreeViewWindow.UITreeViewTree;
            Trace.TraceInformation("Walk through object tree...");
            // Ensure that there is no extra objects in the tree
            foreach (object[] obj in expected_objects)
            {
                Trace.TraceInformation("Check " + (string)obj[0] );
                int obj_num=uITreeViewTree.GetChildren().Count(i => i.Name.StartsWith((string)obj[0]));
                exp_obj_num += (int)obj[1];
                Assert.AreEqual((int)obj[1], obj_num, "Number of objects for " + (string)obj[0] + " is not expected");        
            }
            Assert.AreEqual(exp_obj_num, uITreeViewTree.GetChildren().Count, "Unexpected number of object in tree.");
            Trace.Unindent();
        }
        /// <summary>
        /// 
        /// </summary>
        /// <param name="menu_flags"></param>
        private void _setFilter(string[] menu_flags)
        {
            Trace.Indent();
            Trace.TraceInformation("Select filter options for: " + string.Join(",", menu_flags));
            // Click 'Objects Tree' label
            BasicMap.SelectTab("Objects Tree");

            // Walk though 'Show objects' menu and configure it as expected
            WinMenuItem uIShowObjectsMenuItem = UIObjectPropMap.UITestAppDebuggingMicrWindow.UIItemTabList.UIObjectsTreeTabPage.UIObjectsTreePane.UIObjectsTreePane1.UIToolStrip1ToolBar.UIShowObjectsMenuItem;
            foreach (UITestControl item in uIShowObjectsMenuItem.GetChildren())
            {
                Trace.TraceInformation("Check " + item.FriendlyName + " of type " + item.ControlType);
                if (item.ControlType == "MenuItem")
                {
                    WinMenuItem menu_item = (WinMenuItem)item;
                    if (menu_flags.Contains(menu_item.DisplayText) || (menu_flags.Contains("Show All") && !menu_item.DisplayText.Equals("Show Released Objects")))
                    {
                        Trace.TraceInformation(item.FriendlyName + " should be on.");
                        if (!menu_item.Checked)
                        {
                            Trace.TraceInformation("Enabling" + item.FriendlyName);
                            BasicMap.MouseClick(uIShowObjectsMenuItem);
                            menu_item.Checked = true;
                        }

                    }
                    else
                    {
                        Trace.TraceInformation(item.FriendlyName + " should be off.");
                        if (menu_item.Checked)
                        {
                            Trace.TraceInformation("Disabling" + item.FriendlyName);
                            BasicMap.MouseClick(uIShowObjectsMenuItem);
                            menu_item.Checked = false;
                        }
                    }
                }
            }
            Trace.Indent();
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
            //BasicMap._yourApp.Close();
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


        public ObjectPropMapClasses.ObjectPropMap UIObjectPropMap
        {
            get
            {
                if ((this.ObjectPropMap == null))
                {
                    this.ObjectPropMap = new ObjectPropMapClasses.ObjectPropMap();
                }

                return this.ObjectPropMap;
            }
        }

        private ObjectPropMapClasses.ObjectPropMap ObjectPropMap;
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

using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;
using System.Windows.Input;
using System.Windows.Forms;
using System.Drawing;
using Microsoft.VisualStudio.TestTools.UITesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.VisualStudio.TestTools.UITest.Extension;
using Keyboard = Microsoft.VisualStudio.TestTools.UITesting.Keyboard;
using System.IO;
using System.Diagnostics;

namespace AnalysingSystemTester
{
    /// <summary>
    /// Summary description for AnalyzePackageTester
    /// </summary>
    [CodedUITest]
    public class AnalyzePackageTester
    {
        private string workDir;

        public AnalyzePackageTester()
        {
            string outDir = Directory.GetCurrentDirectory();
            // get the path until TestResults (drop the rest)
            workDir = outDir.Split(new string[] { "TestResults" }, StringSplitOptions.None)[0];
            
        }
        [TestMethod]
        public void CallAllApiTest()
        {
            this.UIMap.StartCallAllApiAnalyze();

            this.UIMap.WaitUntilCallAllApiFinish();

            apiCallsValidation();
            kernelLaunchValidation();
            memoryCommandsValidation();

        }

        private void apiCallsValidation()
        {
            validateSameFile(workDir + "ApiCalls_output.txt", workDir + "debugMode_apiCalls.txt");
        }

        private void kernelLaunchValidation()
        {
            validateSameFile(workDir + "kernelLaunch_output.txt", workDir + "debugMode_kernelsLaunch.txt");
        }

        private void memoryCommandsValidation()
        {
            validateSameFile(workDir + "memoryCommands_output.txt", workDir + "debugMode_memoryCommands.txt");
        }

        private void validateSameFile(string filePath1, string filePath2)
        {
            string line1, line2;
            using (StreamReader sr1 = new StreamReader(filePath1))
            {
                using (StreamReader sr2 = new StreamReader(filePath2))
                {
                    while ((line1 = sr1.ReadLine()) != null)
                    {
                        line2 = sr2.ReadLine();
                        if (line1.CompareTo(line2) != 0)
                        {
                            // validation failed
                            Console.Error.WriteLine("Validation Failed!");
                            Console.Error.WriteLine("Expected: " + line1);
                            Console.Error.WriteLine("Actual: " + line2);
                            throw new AssertFailedException();
                        }

                    }
                    // we check that the second file ended when the first file end.
                    line2 = sr2.ReadLine();
                    if (line2 != null)
                    {
                        // validation failed
                        Console.Error.WriteLine("Validation Failed!");
                        Console.Error.WriteLine("Expected: <EOF>");
                        Console.Error.WriteLine("Actual: " + line2);
                        throw new AssertFailedException();

                    }
                }
            }
        }

        #region Additional test attributes

        // You can use the following additional attributes as you write your tests:

        //Use TestInitialize to run code before running each test 
        [TestInitialize()]
        public void MyTestInitialize()
        {
            System.Diagnostics.Process startProcess = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo codedUIStartInfo = new System.Diagnostics.ProcessStartInfo();

            // run the devenv.exe according to the tVisualStudioVersion
            System.Collections.IDictionary dict = Environment.GetEnvironmentVariables();
       
            string VS_version = Environment.GetEnvironmentVariable("VisualStudioVersion");

            // using VS_version instead of "11.0" causing problem when running it on nightly from the cmd
            codedUIStartInfo.FileName = @"C:\Program Files (x86)\Microsoft Visual Studio " + "11.0" + @"\Common7\IDE\devenv.exe";

            codedUIStartInfo.Arguments = @"/ranu /rootsuffix Exp";

            startProcess.StartInfo = codedUIStartInfo;
            startProcess.Start();

            this.UIMap.OpenAnalyzeSession();

        }

        //Use TestCleanup to run code after each test has run
        [TestCleanup()]
        public void MyTestCleanup()
        {
            //closeExpVS();
        }

        private void closeExpVS()
        {
            Keyboard.SendKeys("{F4}", ModifierKeys.Alt);
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

        public UIMap UIMap
        {
            get
            {
                if ((this.map == null))
                {
                    this.map = new UIMap();
                }

                return this.map;
            }
        }

        private UIMap map;
    }
}

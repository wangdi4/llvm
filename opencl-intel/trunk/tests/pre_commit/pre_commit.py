import unittest,os

"""
This script is intended to be called by cmake.
pre_commit.py <cmake_bin_dir> <configuration>
cmake_bin_dir - path to where cmake generated the build files
configuration - used only in Windows. Specifies build configuration, e.g. Release, Debug 
"""

#TODO: Get rid of these globals
"""
CMAKE_BIN_DIR points to where cmake generated the build files
"""
CMAKE_BIN_DIR = ''

"""
VS_CONFIG is used on Windows where a single tree may build several configurations
e.g. Debug, Release, ReleaseMinSize, ... 
This value is propogated thru a VS macro according to the chosen configuration.
Ignored on Linux
"""
VS_CONFIG = ''


class CommandLineTool:
    def __init__(self):
        pass

    def runCommand(self, command, prepend_command=True):
        """Returns errorcode, output"""
        import subprocess
        
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
        out, dummy = proc.communicate()

        out.replace('\r\n', '\n')
        if prepend_command:
            out = command + out
        return proc.returncode, out

class PreCheckinTestSuite(unittest.TestCase):
    def __init__(self, methodName='runTest'):        
        import platform, os
        unittest.TestCase.__init__(self, methodName)
        self.launch_dir=os.getcwd()
        self.CLT = CommandLineTool()
        if platform.system() == 'Windows':
            program_file_x86 = os.environ['ProgramFiles(x86)']
            self.build_tool = '"' + program_file_x86 + r'\Microsoft Visual Studio 9.0\Common7\IDE\devenv.com"'
            self.build_bin_params = ' ' + CMAKE_BIN_DIR + '/LLVM.sln'            
            self.build_bin_params += ' /build ' + VS_CONFIG 
            self.build_accelarate = ''
            self.build_target_prefix = ' /project'
        else:
            self.build_tool = 'make'
            self.build_bin_params = ' -C ' + CMAKE_BIN_DIR
            self.build_accelarate = ' -j 10'
            self.build_target_prefix = ''

    def helper_run_suite(self, suite):
        """Runs a test suite"""
        command = self.build_tool + self.build_bin_params + self.build_target_prefix + ' ' + suite
        
        returncode, out = self.CLT.runCommand(command)
        self.assertEqual(returncode, 0, out)

    def helper_run_gtest(self, command):
        """Runs an executable from the output dir"""
        os.chdir(CMAKE_BIN_DIR+'/bin/'+VS_CONFIG)
        returncode, out = self.CLT.runCommand(command)
        self.assertRegexpMatches(out, 'TEST SUCCEDDED', out)

    def helper_run_outdir(self, command):
        """Runs an executable from the output dir"""
        os.chdir(CMAKE_BIN_DIR+'/bin/'+VS_CONFIG)
        returncode, out = self.CLT.runCommand(command)
        self.assertEqual(returncode, 0, out)

    '''
    def test_build(self):
        # This test must be always first
        """Build from sources"""        
        command = self.build_tool + ' ' + self.build_bin_params + self.build_accelarate
        
        returncode, out = self.CLT.runCommand(command)
        
        #If build fails, no point in running other tests.
        if returncode != 0:
            print 'FAIL. Aborting...'
            #TDOO: This a pretty hacky way to stop the tests from running...
            raise KeyboardInterrupt
        
        self.assertEqual(returncode, 0, out)            
    '''
        
    def test_check(self):
        """LLVM Regression Suite"""
        self.helper_run_suite('check')
        
    def test_check_vectorizer(self):
        """LLVM Vectorizer Suite"""
        self.helper_run_suite('check_vectorizer')
        
    def test_check_regression(self):
        """LLVM Regression Suite"""
        self.helper_run_suite('check_regression')

    def test_framework_test_type(self):
        self.helper_run_gtest('framework_test_type')

    def test_cpu_device_test_type(self):
        self.helper_run_gtest('cpu_device_test_type')

    def test_basic(self):
        self.helper_run_outdir('basic\\test_basic.exe');

    def test_wolf(self):
        """WOLF Super Fast"""
        os.chdir(self.launch_dir)
        returncode, out = self.CLT.runCommand('python gen_WOLF.py', False)
        self.assertEqual(returncode, 0, out)

        os.chdir(CMAKE_BIN_DIR+'/bin/'+VS_CONFIG)
        with open('tmp_WOLF.py', 'w') as f:
            f.write(out)
        self.helper_run_outdir('python tmp_WOLF.py -v')


            
if __name__ == '__main__':
    import sys
    CMAKE_BIN_DIR = sys.argv[1]
    # Eat the argument so unittest does not see it.
    del sys.argv[1]     
    
    VS_CONFIG = sys.argv[1]        
    # Eat the argument so unittest does not see it.
    del sys.argv[1]
        
    unittest.main()

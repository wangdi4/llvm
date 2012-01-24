import os.path, platform, re, shutil
from framework.core import TIMEOUT_MINUTE, TIMEOUT_HOUR 
from framework.tasks import VolcanoCmdTask
from framework.utils import EnvironmentValue
from Volcano_Common import DEFAULT_OCL_SOLUTION

vectorizerEnvName = "CL_CONFIG_USE_VECTORIZER"
BINARIES_ARCH_NAME = 'Binaries.7z'
z7_cmd = "/usr/intel/bin/7z -y"
if platform.system() == 'Windows':
    z7_cmd = "\"C:/Program Files/7-Zip/7z.exe\" -y"


class LitTest(VolcanoCmdTask):
    """ Runs the LIT test with the given name """
    def __init__(self, name, lit_project, config, solution_name = DEFAULT_OCL_SOLUTION):
        """name - must begin with check_"""
        VolcanoCmdTask.__init__(self, name)
        
        self.workdir = config.solution_dir
        if config.target_os == 'Windows':
            self.command = '"c:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\IDE\devenv.com" ' + solution_name + ' /build ' + config.build_type + ' /project ' + lit_project
        else:
            self.command = 'make ' + lit_project

class WOLFCommonTest(VolcanoCmdTask):
    """ Runs the WOLF or WolfBench test given the workload name and config string"""
    def __init__(self, name, workload, iterations, config_str, config, capture_data, exe_name):
        VolcanoCmdTask.__init__(self, name)
        self.command = ' '.join([exe_name, 'out/'+workload+'.out', workload, str(iterations), config_str])
        self.workdir = config.bin_dir
        self.capture_data = capture_data
        self.workload = workload
    def startUp(self):
        if( self.capture_data ):
            os.environ["OCLBACKEND_PLUGINS"] = "OCLRecorder.dll"
            os.environ["OCLRECORDER_DUMPPREFIX"] = self.name 
 
    def tearDown(self):
        if( self.capture_data ):
            os.environ.pop("OCLBACKEND_PLUGINS")

class WOLFTest(WOLFCommonTest):
    """ Runs the WOLF test given the workload name and config string"""
    def __init__(self, name, workload, iterations, config_str, config, capture_data):
        WOLFCommonTest.__init__(self, name, workload, iterations, config_str, config, capture_data, "WOLF")

class WOLFBenchTest(WOLFCommonTest):
    """ Runs the WOLFbench test given the workload name and config string"""
    def __init__(self, name, workload, iterations, config_str, config, capture_data):
        # WOLFbench workloads has the same name as WOLF workloads.
        # To distinguish them we add prefix "WOLFbench". 
        # This is mainly for historical reasons
        WOLFCommonTest.__init__(self, "WOLFbench."+name, workload, iterations, config_str, config, capture_data, "WOLFBench")

class ArchiverTask(VolcanoCmdTask):
    def __init__(self, name, arch_name, root_dir):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = root_dir
        self.command  = z7_cmd + ' a -r ' + arch_name
            
class UnarchiverTask(VolcanoCmdTask):
    def __init__(self, name, arch_name, root_dir, useCurDir = True):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = root_dir
        if useCurDir:
            self.command  = z7_cmd + ' x ' + arch_name
        else:
            self.command  = z7_cmd + ' x ' + arch_name + ' -o' + root_dir
        self.useCurDir = useCurDir
    
    def startUp(self):
        if self.useCurDir:
            if os.path.exists(self.workdir):
                shutil.rmtree(self.workdir)
            os.makedirs(self.workdir)
        
class VectorizerTest(VolcanoCmdTask):
    def __init__(self, name, config):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = config.bin_dir
        self.config  = config
        if self.config.target_type == 'Win32':
            self.vectorizer_test_cmd = 'tests.exe'
            self.vectorizer_test_bat = 'execute_ocl_tests_32.bat'
        else:
            self.vectorizer_test_cmd = 'tests_64.exe'
            self.vectorizer_test_bat = 'execute_ocl_tests_64.bat'
        self.command = self.vectorizer_test_bat
        self.tests_path = os.path.join(self.config.root_dir, 'tests', 'OCL_Vectorizer_Tests', 'tests')
    
    def startUp(self):
        shutil.copy(os.path.join(self.tests_path, self.vectorizer_test_cmd), self.config.bin_dir)
        shutil.copy(os.path.join(self.tests_path, self.vectorizer_test_bat), self.config.bin_dir)
    
        
    
        
    

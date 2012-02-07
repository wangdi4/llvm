"""
Volcano specific common definitions and configurations
"""
import sys, os.path, platform, re, time, traceback
from framework.core import RunConfig
from framework.utils import NetworkEnvironment

#Volcano specific global default paths
REPOSITORY_ROOT             = 'https://subversion.iil.intel.com/ssg-repos/MMS'
IT_SAMBA_SERVER             = '//ismb014.iil.intel.com'
VOLCANO_IDC_SAMBA_SERVER    = '//cvcc-ubu-01.iil.intel.com'
VOLCANO_NN_SAMBA_SERVER     = '//nntavc216ew.ccr.corp.intel.com/CVCC_share'

DX_PERFORMANCE_SHADERS_ROOT = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root/Performance/NoDcls/PerformanceCriticalShaders'
DX_10_SHADERS_ROOT          = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root'
OCL_CONFORMANCE_TESTS_ROOT  = '/cvcc/CapturedWLs/Conformance'
PERFORMANCE_LOG_ROOT        = '/cvcc/Logs/Volcano/Performance'
PERFORMANCE_TESTS_ROOT      = os.path.realpath('/Volcano/Performance/Tests')
DEFAULT_VS_VERSION          = 9
DEFAULT_VOLCANO_SOLUTION    = 'Backend.sln'
DEFAULT_OCL_SOLUTION        = 'OCL.sln'

SUPPORTED_CPUS         = ['auto', 'corei7', 'sandybridge']
SUPPORTED_TARGETS      = ['Win32', 'Win64', 'SLES64', 'RH64']
SUPPORTED_BUILDS       = ['Release', 'Debug']
SUPPORTED_VECTOR_SIZES = ['0', '1', '4', '8', '16']

# maps the target short name to the OS and Bitness
TARGETS_MAP = { 
                'Win32'   : ['Windows', 32],
                'Win64'   : ['Windows', 64],
                'SLES64'  : ['Linux', 64],
                'RH64'    : ['Linux', 64],
                'Linux64' : ['Linux', 64]
              }
# maps the volcano environment supported cpu names to the backend internal cpu names
CPU_MAP = { 'auto': 'auto',
            'corei7': 'corei7',
            'sandybridge': 'corei7-avx'}

DOMAIN_MAP = { 'ccr.corp.intel.com': VOLCANO_NN_SAMBA_SERVER, 
               'ger.corp.intel.com': VOLCANO_IDC_SAMBA_SERVER,
               'ill.intel.com': VOLCANO_NN_SAMBA_SERVER }

if platform.system() == 'Windows':
    DX_PERFORMANCE_SHADERS_ROOT = IT_SAMBA_SERVER + DX_PERFORMANCE_SHADERS_ROOT
    DX_10_SHADERS_ROOT          = IT_SAMBA_SERVER + DX_10_SHADERS_ROOT
    OCL_CONFORMANCE_TESTS_ROOT  = VOLCANO_IDC_SAMBA_SERVER + OCL_CONFORMANCE_TESTS_ROOT
    PERFORMANCE_TESTS_ROOT      = os.path.realpath('c:/Volcano/Performance/Tests')
    domain = NetworkEnvironment.getDomain()
    if domain in DOMAIN_MAP: 
        PERFORMANCE_LOG_ROOT = DOMAIN_MAP[domain] + PERFORMANCE_LOG_ROOT
    else:
        PERFORMANCE_LOG_ROOT = VOLCANO_IDC_SAMBA_SERVER + PERFORMANCE_LOG_ROOT

class VolcanoRunConfig( RunConfig):
    """
    Volcano specific runtime configuration
    """
    def __init__(self, root_dir, target_type, build_type, cpu, cpu_features, vec):
        RunConfig.__init__(self)
        self.cpu            = cpu
        self.cpu_features   = cpu_features
        self.target_type    = target_type
        self.build_type     = build_type
        self.transpose_size = vec
        self.root_dir       = root_dir
        self.sub_configs    = {}
        
        #Configuration validation
        if not self.cpu in SUPPORTED_CPUS:
            raise Exception("Configuration Error: Unsupported CPU specified:" + self.cpu + ". Supported CPUs are:" + str(SUPPORTED_CPUS))

        if not self.target_type in SUPPORTED_TARGETS:
            raise Exception("Configuration Error: Unsupported target specified:" + self.target_type + ". Supported targets are:" + str(SUPPORTED_TARGETS))
        
        if not self.build_type in SUPPORTED_BUILDS:
            raise Exception("Configuration Error: Unsupported build type specified:" + self.build_type + ". Supported build types are:" + str(SUPPORTED_BUILDS))
        
        if not self.transpose_size in SUPPORTED_VECTOR_SIZES:
            raise Exception("Configuration Error: Unsupported transpose size specified:" + self.transpose_size + ". Supported transpose sizes are:" + str(SUPPORTED_VECTOR_SIZES))

        #
        # Calculating derived configuration
        #
        self.target_os     = TARGETS_MAP[self.target_type][0]
        self.target_os_bit = TARGETS_MAP[self.target_type][1]
        
        # Paths
        if self.root_dir == '' or self.root_dir == None:
            self.root_dir = os.path.join(os.path.curdir, os.path.pardir, os.path.pardir, os.path.pardir, os.path.pardir)
        
        self.root_dir     = os.path.abspath(self.root_dir)
        self.src_dir      = os.path.join(self.root_dir, 'src')
        self.besrc_dir    = os.path.join(self.src_dir,  'backend' )
        self.scripts_dir  = os.path.join(self.besrc_dir, 'build')
        self.install_dir  = os.path.join(self.root_dir, 'install',  self.target_type, self.build_type)
        self.bin_dir      = os.path.join(self.install_dir, 'bin')
    
        if self.target_os == 'Windows':
            self.solution_dir = os.path.join(self.root_dir, 'build', self.target_type)
        else:
            self.solution_dir = os.path.join(self.root_dir, 'build', self.target_type, self.build_type)
    
    def getConfigMask(self):
        return [self.cpu, self.target_type, self.build_type, self.target_type, self.transpose_size]
    


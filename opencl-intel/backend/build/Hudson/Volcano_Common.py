"""
Volcano specific common definitions and configurations
"""
import sys, os.path, platform, re, time, traceback
from framework.core import RunConfig

#Volcano specific global default paths
SAMBA_SERVER = '//ismb014.iil.intel.com'
DX_PERFORMANCE_SHADERS_ROOT = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root/Performance/NoDcls/PerformanceCriticalShaders'
DX_10_SHADERS_ROOT          = '/nfs/iil/disks/cvcc/testbase/Shaders/DX/root'
PERFORMANCE_LOG_ROOT        = '/mnt/cvcc_ftp/Logs/Volcano/Performance'
PERFORMANCE_TESTS_ROOT      = '/Volcano/Performance/Tests'
OCL_CONFORMANCE_TESTS_ROOT  = '/nfs/iil/disks/cvcc/vdovleka/tests/Volcano/Conformance'
DEFAULT_WORLOADS_ROOT       = '/nfs/iil/disks/cvcc/OclConformance_14756/trunk/ReleaseCriteria/'
DEFAULT_VS_VERSION          = 9
DEFAULT_VOLCANO_SOLUTION    = 'Backend.sln'
DEFAULT_OCL_SOLUTION        = 'OCL.sln'
REPOSITORY_ROOT             = 'https://subversion.iil.intel.com/ssg-repos/MMS'


if platform.system() == 'Windows':
    DX_PERFORMANCE_SHADERS_ROOT = SAMBA_SERVER + DX_PERFORMANCE_SHADERS_ROOT
    DX_10_SHADERS_ROOT          = SAMBA_SERVER + DX_10_SHADERS_ROOT
    PERFORMANCE_LOG_ROOT        = '\\cvcc-w7-mrm-03.iil.intel.com\CVCC_FTP\Logs\Volcano\Performance'
    OCL_CONFORMANCE_TESTS_ROOT  = SAMBA_SERVER + OCL_CONFORMANCE_TESTS_ROOT
    DEFAULT_WORLOADS_ROOT       = SAMBA_SERVER + DEFAULT_WORLOADS_ROOT

SUPPORTED_CPUS = ['auto', 'corei7', 'sandybridge']
SUPPORTED_TARGETS = ['Win32', 'Win64', 'Linux64']
SUPPORTED_BUILDS = ['Release', 'Debug']
SUPPORTED_VECTOR_SIZES = ['0', '1', '4', '8', '16']

# maps the target short name to the OS and Bitness
TARGETS_MAP = { 
                'Win32'  : ['Windows', 32],
                'Win64'  : ['Windows', 64],
                'Linux64': ['Linux', 64] 
              }

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
    


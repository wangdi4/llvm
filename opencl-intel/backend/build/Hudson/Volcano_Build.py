import os,sys,platform,errno,shutil,re
import framework.cmdtool
import framework.resultPrinter
import framework.logger

from optparse import OptionParser
from framework.tasks import SimpleTest, DirCleanup
from framework.utils import VSEnvironment
from framework.core import VolcanoTestRunner, VolcanoTestSuite, VolcanoCmdTask, VolcanoTestTask, TestTaskResult
from Volcano_Tasks import BINARIES_ARCH_NAME, UnarchiverTask
from Volcano_Common import VolcanoRunConfig, SUPPORTED_TARGETS, SUPPORTED_BUILDS, DEFAULT_VS_VERSION, DEFAULT_VOLCANO_SOLUTION, DEFAULT_OCL_SOLUTION

    
class CMakeConfig:
    """CMake specific configuration"""
    CFG_NAME = "CMAKECFG"
    
    def __init__(self, vc_version, include_mic = False, enable_sde = False, include_cnf = True, include_crt = False,
            include_java = False, include_dbg = False, include_svn = True, enable_warnings = False):
        self.vc_version   = vc_version
        self.include_mic  = include_mic
        self.enable_sde   = enable_sde
        self.include_cnf  = include_cnf
        self.include_crt  = include_crt
        self.include_java = include_java
        self.include_dbg  = include_dbg
        self.include_svn  = include_svn
        self.enable_warnings = enable_warnings

class CMakeBuilder(VolcanoCmdTask):
    """ Runs the cmake for generating the Volcano specific project files """
    def __init__(self, name, config, src_dir, defs ={}):
        VolcanoCmdTask.__init__(self, name)
        self.config  = config
        self.workdir = config.solution_dir

        # Prepare cmake command and arguments
        self.command = "cmake" \
                + " -D LLVM_ENABLE_WERROR:BOOL=ON " \
                + " -D PYTHON_EXECUTABLE:PATH=\"" + sys.executable + "\""

        cmake_config = config.sub_configs[CMakeConfig.CFG_NAME]
        vsenv = VSEnvironment(cmake_config.vc_version)

        for k, v in defs.iteritems():
            self.command += " -D " + k + "=" + v 

        # Install directory

        if cmake_config.include_mic:
          self.command += " -D INCLUDE_MIC_DEVICE:BOOL=ON "

        if cmake_config.enable_sde:
          self.command += " -D ENABLE_SDE:BOOL=ON "

        if config.target_os == 'Windows':
            self.command += " -D CMAKE_INSTALL_PREFIX:PATH=\"" + os.path.join(config.root_dir, 'install',  config.target_type) + "/\\${BUILD_TYPE}\""
            #self.command += " -D LLVM_TARGETS_TO_BUILD:STRING=\"X86\"" 

            if config.target_os_bit == 32:
                self.command += ' -G "' + vsenv.CMakeGenerator() + '"'
            elif config.target_os_bit == 64:
                self.command += ' -G "' + vsenv.CMakeGenerator() + ' Win64"'
                if cmake_config.vc_version == 9:
                    self.command += ' -D CMAKE_ASM_MASM_COMPILER="' + vsenv.Asm64Path() + '"'
            else:
                raise Exception("Unknown build target")
        elif config.target_os == 'Linux':
            self.command += " -G \"Unix Makefiles\""
            self.command += " -D CMAKE_BUILD_TYPE:STRING=" + config.build_type                
            self.command += " -D CMAKE_INSTALL_PREFIX:PATH=\"" + os.path.join(config.root_dir, 'install',  config.target_type, config.build_type) + "\""
        else:
            raise Exception("Unknown build target")
            
        self.command +=  " " + src_dir
            
    def startUp(self):
        # Create build tree directory
        try:
            os.makedirs(self.config.solution_dir)
        except OSError as exc:
            if exc.errno == errno.EEXIST:
                # Directory already exists?
                pass
            else: 
                raise
                
class VolcanoCMakeBuilder( CMakeBuilder ):
    """
    Volcano specific cmake builder. Responsible for building just a Volcano project
    """
    def __init__(self, name, config):
        defs = {}
        cmake_config = config.sub_configs[CMakeConfig.CFG_NAME]
        
        defs['LLVM_ENABLE_WARNINGS:BOOL'] ='ON' if cmake_config.enable_warnings else 'OFF'

        CMakeBuilder.__init__(self, name, config, config.besrc_dir, defs)

class OCLCMakeBuilder( CMakeBuilder ):
    """
    OCL specific cmake builder. Responsible for building the whole OCL project
    """
    CONFORMANCE_TESTS='"test_allocations test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_d3d9 test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep"'

    def __init__(self, name, config):
        defs = {}
        cmake_config = config.sub_configs[CMakeConfig.CFG_NAME]
        
        defs['SVN_REVISIONS']                  ='ON' if cmake_config.include_svn     else 'OFF'
        defs['INCLUDE_MIC_DEVICE:BOOL']        ='ON' if cmake_config.include_mic else 'OFF'
        defs['ENABLE_SDE:BOOL']                ='ON' if cmake_config.enable_sde else 'OFF'
        defs['LLVM_ENABLE_WARNINGS:BOOL']      ='ON' if cmake_config.enable_warnings else 'OFF'
        defs['INCLUDE_CONFORMANCE_TESTS:BOOL'] ='ON' if cmake_config.include_cnf     else 'OFF'
        defs['CONFORMANCE_LIST:STRING']        = OCLCMakeBuilder.CONFORMANCE_TESTS
        defs['BUILD_JAVA:BOOL']                ='ON' if cmake_config.include_java    else 'OFF'
        defs['BUILD_X64:BOOL']                 ='ON' if 64 == config.target_os_bit   else 'OFF'
        defs['CMAKE_BUILD_TYPE:STRING']        = '"' +config.build_type + '"'
        defs['CMAKE_COLOR_MAKEFILE:BOOL']      ='ON'
        defs['LLVM_USE_INTEL_JITEVENTS:BOOL']  ='ON'
        
        if config.target_os == 'Windows':
            defs['INCLUDE_CMRT:BOOL']                  ='ON' if cmake_config.include_crt else 'OFF'
            defs['INCLUDE_DEBUGGER:BOOL']              ='ON' if cmake_config.include_dbg else 'OFF'
        else:
            defs['OCL_CMAKE_INCLUDE_DIRECTORIES:PATH'] = '"' + os.path.join( config.src_dir, 'cmake_utils') + '"'
            defs['OCL_TOOLCHAIN_FILE:PATH']            = '"' +os.path.join( config.src_dir, 'cmake_utils', 'Linux-GNU.cmake') + '"'
            defs['CMAKE_MODULE_PATH:PATH']             = '"' +os.path.join( config.src_dir, 'cmake_utils') + '"'
            defs['OCL_BUILD32:BOOL']                   = 'ON' if 32 == config.target_os_bit else 'OFF'
            defs['TARGET_CPU:STRING']                  = '"x86"'
            defs['INTEL_COMPILER:BOOL']                = 'OFF'

    
        CMakeBuilder.__init__(self, name, config, config.src_dir, defs)
                
class VSProjectBuilder(VolcanoCmdTask):
    """ Runs the general VS solution build """
    def __init__(self, name, config, sln_name, vc_version, rebuild):
        VolcanoCmdTask.__init__(self, name)

        vsenv = VSEnvironment(vc_version)
        
        vs_target = config.target_type
        if "Win64" == config.target_type:
            vs_target = "X64"
            
        operation = '/rebuild' if rebuild else '/build'    
            
        self.workdir = config.root_dir
        self.command = '"' + vsenv.DevEnvPath() + '"' \
                + " " + os.path.join(config.solution_dir, sln_name) \
                + " " + operation \
                + " \"" + config.build_type + "|" + vs_target + "\"" \
                + " /project \"INSTALL.vcproj\""
                
class MakeBuilder(VolcanoCmdTask):
    """ Runs the simple make project"""
    def __init__(self, name, config):
        VolcanoCmdTask.__init__(self, name)
        self.workdir = config.solution_dir
        self.command = 'make install -j 8 -B' 
        
class FixWolfWorkloads(VolcanoTestTask):
    """
    Run SED for fixing the WOLF tests configurations
    """
    def __init__(self, name, config):
        VolcanoTestTask.__init__(self, name)
        self.workdir = os.path.join( config.bin_dir, 'Workloads')
    
    def runTest(self, observer, config):
        if not os.path.exists(self.workdir):
            self.logAndPrint("Working directore not exists. Nothing to fix")
            return TestTaskResult.Passed
            
        os.chdir(self.workdir)
        
        cmd = CommandLineTool()
        
        for root, dirs, files in os.walk(self.workdir):
            for name in files:
                if name.endswith('.cfg'):
                    fname = os.path.realpath(os.path.join(root,name))
                    (retcode, stdoutdata) = cmd.runCommand("sed -i -e s#\.\.\/\.\.\/\.\.\/Workloads#Workloads# " + fname)
                    self.log(stdoutdata)
                    if retcode != 0:
                        return TestTaskResult.Failed
        return TestTaskResult.Passed

#class CopyWolfWorkloads(VolcanoTestSuite):
#    def __init__(self, name, config):
#        VolcanoTestSuite.__init__(self, name)
#        self.generate_children_report = False
#         
#        self.addTask( UnarchiverTask( 'UnarchWLs', os.path.join( DEFAULT_WORLOADS_ROOT, 'Workloads.7z'), config.bin_dir, useCurDir = False), stop_on_failure=True)
#        
#        if platform.system() != 'Windows':
#            self.addTask(SimpleTest('ChMod',   config.bin_dir,   'chmod -R 777 .'), stop_on_failure=True)
#            
#        self.addTask( FixWolfWorkloads( 'FixWolfWLs', config), stop_on_failure=True)


class FixCSharpProject(VolcanoTestTask):
    """ 
    Fix the bug in cmake and update the correct GUID in the given C# project inside the given solution
    """
    def __init__(self, name, config, sln_name, prj_name):
        VolcanoTestTask.__init__(self, name)
        self.config   = config
        self.sln_name = sln_name
        self.prj_name = prj_name
    
    def runTest(self, observer, config):
        tmpfile_name = self.sln_name + '.tmp'
        oldfile_name = self.sln_name + '.orig'
        CXX_UUID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
        C_SHARP_UUID = '{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}'

        with open(self.sln_name, 'r') as f:
            with open(tmpfile_name, 'w') as tmpfile:
                pattern = re.compile('^Project.+"' + self.prj_name + '".+')
                for line in f:
                    if pattern.search(line):
                        line = re.sub(CXX_UUID,C_SHARP_UUID,line)
                    tmpfile.write(line)

        if os.path.isfile( oldfile_name ):
            os.remove(oldfile_name)

        os.rename( self.sln_name, oldfile_name )
        os.rename( tmpfile_name, self.sln_name )
        return TestTaskResult.Passed

class VolcanoBuilderConfig:
    """
    Volcano builder task configuration
    """
    CFG_NAME = "BuildConfig"
    def __init__(self,  volcano_only    = False, 
                        include_mic     = False,
                        enable_sde      = False,
                        include_cnf     = True , 
                        include_crt     = False, 
                        include_java    = False, 
                        include_dbg     = False, 
                        include_svn     = True ,
                        enable_warnings = False):
        self.volcano_only = volcano_only
        self.solution_name= DEFAULT_VOLCANO_SOLUTION if volcano_only else DEFAULT_OCL_SOLUTION
        self.cmake_config = CMakeConfig(DEFAULT_VS_VERSION, include_mic, enable_sde, include_cnf,
                include_crt, include_java, include_dbg, include_svn, enable_warnings)
        
class VolcanoBuilder(VolcanoTestSuite):
    """
    Build suite for Volcano&OCL project
    """
    def __init__(self, name, config, rebuild = True, skip_build = False):
        VolcanoTestSuite.__init__(self, name)
        self.generate_children_report = False

        skiplist = []
        if True == skip_build:
            skiplist=[['.*']]


        # prepare the cmake configuration
        if VolcanoBuilderConfig.CFG_NAME in config.sub_configs:
            build_config = config.sub_configs[VolcanoBuilderConfig.CFG_NAME]
        else:
            build_config = VolcanoBuilderConfig()
        config.sub_configs[CMakeConfig.CFG_NAME] = build_config.cmake_config
            
        if True == rebuild:    
            self.addTask(DirCleanup('Cleanup(Build)',   config, config.install_dir),  stop_on_failure=True, skiplist=skiplist)
            self.addTask(DirCleanup('Cleanup(Install)', config, config.solution_dir), stop_on_failure=True, skiplist=skiplist)

        if( build_config.volcano_only ):
            self.addTask(VolcanoCMakeBuilder('CMake(Volcano)', config), stop_on_failure=True)
        else:
            self.addTask(OCLCMakeBuilder('CMake(OCL)', config), stop_on_failure=True)
            if config.target_os == 'Windows':
                if( build_config.cmake_config.include_java ):
                    self.addTask(FixCSharpProject('FixC#(Java)', config, os.path.join(config.solution_dir, DEFAULT_OCL_SOLUTION), 'iocgui'),stop_on_failure=True)

                if( build_config.cmake_config.include_dbg ):
                    self.addTask(FixCSharpProject('FixC#(DbgEng)', config, os.path.join(config.solution_dir, DEFAULT_OCL_SOLUTION), 'OCLDebugEngine'),stop_on_failure=True)
                    self.addTask(FixCSharpProject('FixC#(DbgCfg)', config, os.path.join(config.solution_dir, DEFAULT_OCL_SOLUTION), 'OCLDebugConfigPackage'),stop_on_failure=True)
            
        if config.target_os == 'Windows':
            self.addTask(VSProjectBuilder('VSBuild', config, build_config.solution_name, build_config.cmake_config.vc_version, rebuild), stop_on_failure=True, skiplist=skiplist)
        elif config.target_os == 'Linux':
            self.addTask(MakeBuilder('MKBuild', config), stop_on_failure=True, skiplist=skiplist)

class VolcanoBinaryCopy(VolcanoTestSuite):
    """
    Suite for unarchiving the Volcano&OCL binaries 
    """
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.generate_children_report = False
    
        self.addTask(DirCleanup('Cleanup(Install)', config, config.install_dir), stop_on_failure=True)
        self.addTask(UnarchiverTask('UnarchiveBinaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True)

def main():
    parser = OptionParser()
    parser.add_option("-r", "--root",      dest="root_dir",     help="Project root directory. Default:Autodetect", default=None)
    parser.add_option("-t", "--target",    action="store",      choices=SUPPORTED_TARGETS, dest="target_type",  default="Win32",   help="Target type: " + str(SUPPORTED_TARGETS) + ". [Default: %default]")
    parser.add_option("-b", "--build_type",action="store",      choices=SUPPORTED_BUILDS,  dest="build_type",   default="Release", help="Build type: " + str(SUPPORTED_BUILDS) + ". [Default: %default]")
    parser.add_option("-s", "--cmake_only",dest="cmake_only",   action="store_true",  help="Do not run the build, just generate the project files. Default: False", default=False)
    parser.add_option("-m", "--include-mic",
                                           dest="include_mic",  action="store_true",  help="Include the MIC Device in the build.", default=False)
    parser.add_option("-e", "--enable-sde",
                                           dest="enable_sde",   action="store_true",  help="Enable SDE support", default=False)
    parser.add_option("--volcano",         dest="volcano_only", action="store_true",  help="Build the Volcano solution only.", default=False)
    parser.add_option("--ocl",             dest="volcano_only", action="store_false", help="Build the OCL solution. Default")
    parser.add_option("--norebuild",       dest="rebuild",      action="store_false", help="Perform only regular build.", default=True )
    parser.add_option("--cnf",             dest="include_cnf",  action="store_true",  help="Include the conformance tests into solution. Default", default=True)
    parser.add_option("--ncnf",            dest="include_cnf",  action="store_false", help="Exclude the conformance tests from solution." )
    parser.add_option("--crt",             dest="include_crt",  action="store_true",  help="Include the Common Runtime into solution. Default", default=True)
    parser.add_option("--ncrt",            dest="include_crt",  action="store_false", help="Exclude the Common Runtime from solution.")
    parser.add_option("--java",            dest="include_java", action="store_true",  help="Include java code. ", default=False)
    parser.add_option("--njava",           dest="include_java", action="store_false", help="Exclude java code. Default")
    parser.add_option("--dbg",             dest="include_dbg",  action="store_true",  help="Include the debugger engine into solution", default=False)
    parser.add_option("--ndbg",            dest="include_dbg",  action="store_false", help="Exclude the debugger engine from solution. Default")
    parser.add_option("--no-svn",          dest="include_svn",  action="store_false",  help="Do not use svn revision to create package version. Useful if you are not building from an SVN working copy.", default=True)
    parser.add_option("-d", "--demo",      dest="demo_mode",    action="store_true",  help="Do not execute the commands, just print them. Default: False", default=False)
    parser.add_option(      "--nocolor",   dest="use_color",    action="store_false", help="Do not use console color output", default=True)

    (options, args) = parser.parse_args()

    framework.cmdtool.demo_mode = options.demo_mode 
    framework.logger.gLog.enableColor(options.use_color) 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              'corei7',
                              '',
                              '1')
    config.sub_configs[VolcanoBuilderConfig.CFG_NAME]=VolcanoBuilderConfig( options.volcano_only,
                                                                            options.include_mic,
                                                                            options.enable_sde,
                                                                            options.include_cnf,
                                                                            options.include_crt,
                                                                            options.include_java,
                                                                            options.include_dbg,
                                                                            options.include_svn,
                                                                            enable_warnings = False)
    suite  = VolcanoBuilder('Build', 
                            config, 
                            rebuild = options.rebuild, 
                            skip_build= options.cmake_only)
    runner = VolcanoTestRunner()
    passed = runner.runTask(suite, config)
    printer= framework.resultPrinter.ResultPrinter()
    printer.PrintResults(suite)
    
    if not passed:
        return 1
    return 0
    

if __name__ == "__main__":
        if platform.platform().startswith("CYGWIN"):
            print "Cygwin Python is not supported. Please use ActiveState Python."
            sys.exit(1);
        if sys.version_info < (2, 6):
            print "Python version 2.6 or later required"
            sys,exit(1)
        main_result = main()
        sys.exit(main_result)

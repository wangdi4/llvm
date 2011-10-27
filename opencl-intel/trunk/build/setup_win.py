"""
This file contains implementation of a command line tool which invokes cmake to generate native build system configuration files.
Currently only VS2008 is supported. For building under Linux, you should use the Makefile which is under the source tree.
"""

import os, os.path, errno, subprocess, shutil, glob
import logging
import platform

class VCVersion:
    def __init__(self, version, programFiles_relpath, cmake_builder_name):
        self.version              = version
        self.programFiles_relpath = programFiles_relpath
        self.cmake_builder_name   = cmake_builder_name

VCVersion_List = { 9  : VCVersion(9, "Microsoft Visual Studio 9.0", "Visual Studio 9 2008"),
                   10 : VCVersion(10, "Microsoft Visual Studio 10.0", "Visual Studio 10") }

class CommandLineTool:
    def __init__(self, config, logger, exit_on_fail = True):
        self.config = config
        self.logger = logger
        self.exit_on_fail = exit_on_fail

    def Execute(self):
        pass

    def RunCommand(self, command):
        logger = self.logger

        if logger != None:
            logger.debug(command)
        Shell = True
        if platform.system() == 'Windows':
            Shell = False
        proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=Shell)
        (stdoutdata, stderrdata) = proc.communicate()

        if proc.returncode != 0:
            if logger != None:
                logger.error(stdoutdata)
            if self.exit_on_fail:
                sys.exit(1)
        else:
            logger.info(stdoutdata)


class IcProjConvertConfiguration:
    def __init__(self, icc_version, sln_path):
        self.icc_version = icc_version
        self.sln_path = sln_path


class IcProjConvertBuilder(CommandLineTool):
    def __init__(self, config, logger):
        CommandLineTool.__init__(self, config, logger)

    def Execute(self):
        logger = self.logger
        config = self.config

        command = "\"" + os.environ["CommonProgramFiles(x86)"]+"\Intel\shared files\ia32\Bin\ICProjConvert" + str(config.icc_version) + "0.exe\" " \
                + config.sln_path + " /ic /nologo"

        self.RunCommand(command)

class VSConfiguration:
    def __init__(self, sln_path, target, build_config, vc_version, force_rebuild=False):
        self.sln_path      = sln_path
        self.build_config  = build_config
        self.target        = target
        self.force_rebuild = force_rebuild
        self.vc_version    = vc_version

class VSBuilder(CommandLineTool):
    def __init__(self, config, logger):
        CommandLineTool.__init__(self, config, logger)

    def Execute(self):
        logger = self.logger
        config = self.config
        vc_version = config.vc_version

        vs_target = config.target
        if "Win64" == vs_target:
            vs_target = "X64"

        operation = " /build "
        if config.force_rebuild:
            operation = " /rebuild "
        command = '"' + os.environ['ProgramFiles(x86)'] \
                + '/' + vc_version.programFiles_relpath \
                + '/Common7/IDE/devenv.com"' \
                + " " + config.sln_path \
                + operation \
                +'"' \
                + config.build_config + "|" \
                + vs_target \
                + "\""

        self.RunCommand(command)

class CmakeConfiguration:
    def __init__(self, target, path_sources, path_buildtree, path_bin, enable_warnings, path_python_executable, vc_version):
        self.target = target
        self.path_sources = path_sources
        self.path_buildtree = path_buildtree
        self.path_bin = path_bin
        self.enable_warnings = enable_warnings
        self.vc_version = vc_version
	self.path_python_executable = path_python_executable

class CMakeBuilder(CommandLineTool):
    def __init__(self, config, logger):
        CommandLineTool.__init__(self, config, logger)
        self.vc_version = config.vc_version

    def Execute(self):
        logger = self.logger
        config = self.config
        vc_version = config.vc_version

        # Create build tree directory
        try:
            os.makedirs(self.config.path_buildtree)
        except OSError as exc:
            if exc.errno == errno.EEXIST:
                # Directory already exists?
                pass
            else: 
                raise

        # cmake must be run from the path where the build tree is 
        original_dir = os.getcwd()
        os.chdir(self.config.path_buildtree)
        logger.debug(os.getcwd())

        # Prepare cmake command and arguments
        command = "cmake" \
                + " " + config.path_sources \
                + " -DLLVM_ENABLE_WERROR:BOOL=ON "\
                + " -DCMAKE_BINARY_DIR:PATH=\"" + config.path_buildtree + "\"" \
                + " -DCMAKE_INSTALL_PREFIX:PATH=\"" + config.path_bin + "\"" \
                + " -DLLVM_TARGETS_TO_BUILD:STRING=\"X86\"" \
		+ " -DPYTHON_EXECUTABLE:PATH=\"" + config.path_python_executable + "\""

        if config.enable_warnings:
            command += " -DLLVM_ENABLE_WARNINGS:BOOL=\"1\""
        
        if config.target=="win32":
            command += ' -G "' + vc_version.cmake_builder_name + '"'
        elif config.target=="win64":
            if self.vc_version.version == 9:
                asm_path = os.getenv("ProgramFiles(x86)").replace('\\','/')
                command += ' -DCMAKE_ASM_MASM_COMPILER="' + asm_path + '/' + vc_version.programFiles_relpath + '/VC/bin/x86_amd64/ml64.exe"'
            command += ' -G "' + vc_version.cmake_builder_name + ' Win64"'
        else:
            raise Exception("Unknown build target")

        # Run cmake
        self.RunCommand(command)

        os.chdir(original_dir)

def main():
    from optparse import OptionParser
    import sys

    LOGGING_LEVELS = {'debug': logging.DEBUG,
          'info': logging.INFO,
          'warning': logging.WARNING,
          'error': logging.ERROR,
          'critical': logging.CRITICAL}
    

    # set default values
    default_path_sources = "."
    default_enable_warning = False
    default_toolchain = "msvc"
    available_toolchains = ["icc11", "icc12", "msvc"]
    if platform.system() == "Windows": 
        available_targets = ["win32", "win64"]
        default_target = 'Win64'
    else:
        default_target = "TODO: set for linux"
    # Use the interpreter which is running this script as a param to cmake
    default_python_executable = sys.executable

    # parse options
    parser = OptionParser()
    parser.add_option("-t", "--target", 
            action="store", type="string", dest="target", default=default_target,
            help="Build target, possible values are: Win32, Win64. Default is " + default_target)
    parser.add_option("-s", "--path-sources", type="string", dest="path_sources", default=default_path_sources,
            help="Path to source. Default is " + default_path_sources)
    parser.add_option("-v", "--verbose-level", dest="verbose_level", default='info',
            help="Verbosity level, possible values: debug, info, warning, error, critical")
    parser.add_option("-l", "--log-file", type="string", dest="log_file", default="", #default is log to stdout
            help="Name of lof file to replace stdout")
    parser.add_option("-w", "--enable-warnings", dest="enable_warnings", action="store_true", default=False,
            help="Enable warnings")
    parser.add_option("--python-exe", dest="python_executable", default=default_python_executable,
            help="Set path to Python executable. Default will use the interpreter which is running this script. Detected: " + default_python_executable)
    parser.add_option("--toolchain", dest="toolchain", default=default_toolchain,
            help="Set toolchain to build LLVM: " + " ".join(available_toolchains) + ". default: " + default_toolchain)
    parser.add_option("-b", "--build", dest="build_config", type="string", default=None,
            help="Build the VS solution after it is generated. Build config is the configuration name e.g. Debug, Release")
    parser.add_option("--vc10", dest="use_vc10", action="store_true",
            help="Use Visual C++ 10 (experimental)")

    (options, args) = parser.parse_args()

    if options.target.lower() not in available_targets:
        parser.error("Invalid target: " + options.target)
    if options.verbose_level not in LOGGING_LEVELS:
        parser.error("Invalid verbosity level: "+options.verbose_level)
    if options.toolchain.lower() not in available_toolchains:
        parser.error("Imvalid toolchain: " + options.toolchain)

    vc_version = VCVersion_List[9]
    if options.use_vc10:
        vc_version = VCVersion_List[10]

    # setup logger
    level = LOGGING_LEVELS[options.verbose_level]
    logger = logging.getLogger('setup')
    logger.setLevel(level=level)
    if (options.log_file == ""):
        ch = logging.StreamHandler()
    else:
        ch = logging.FileHandler(options.log_file, 'w')
    ch.setLevel(level=level)
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    ch.setFormatter(formatter)
    logger.addHandler(ch)


    # Run cmake
    config = CmakeConfiguration(target=options.target.lower(), 
            path_sources=os.path.abspath("../llvm"), 
            path_buildtree = os.path.abspath("" + options.target), 
            path_bin = os.path.abspath("" + options.target),
            enable_warnings = options.enable_warnings,
            path_python_executable = options.python_executable,
            vc_version = vc_version)
            
    cmakebuilder = CMakeBuilder(config, logger)
    print "Creating build environment, this may take a while ..."
    cmakebuilder.Execute()

    # Run IcProjConvert
    if options.toolchain != "msvc":
        icc_version = 11
        if options.toolchain == "icc12":
            icc_version = 12

        config = IcProjConvertConfiguration(icc_version = icc_version, 
                sln_path = options.target+"/LLVM.sln")

        icprojconvertbuilder = IcProjConvertBuilder(config, logger)
        icprojconvertbuilder.Execute()

    # Run devenv
    if options.build_config != None:
        # Build Volcano
        config = VSConfiguration(options.target+"/LLVM.sln", options.target, options.build_config, vc_version)
        VSB = VSBuilder(config, logger)
        VSB.Execute()

        builtins_rel_path = "../libraries/ocl_builtins/"
        
        file_list = []
        
        # Find all SVML binaries
        svml_src_dir = os.path.abspath(os.path.join(builtins_rel_path, 'bin', 'svml', options.target))
        file_list += list(glob.iglob(os.path.join(svml_src_dir, '__ocl_svml_*.dll')))
        file_list += list(glob.iglob(os.path.join(svml_src_dir, '__ocl_svml_*.pdb')))
        
        
        # Copy the built-in binaries to the Volcano output dir
        dst_dir = os.path.abspath(os.path.join(options.target, 'bin', options.build_config))
        for path in file_list:
            dst_path = os.path.join(dst_dir, path)
            logger.info(path+'->'+dst_dir)
            shutil.copy(path, dst_dir )
        

if __name__ == "__main__":
        import sys
        import platform
        if platform.platform().startswith("CYGWIN"):
            print "Cygwin Python is not supported. Please use ActiveState Python."
            sys.exit(1);
        if sys.version_info < (2, 6):
            print "Python version 2.6 or later required"
            sys.exit(1)
        main_result = main()
        sys.exit(main_result)

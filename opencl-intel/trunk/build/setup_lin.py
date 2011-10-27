"""
This file contains implementation of a command line tool which invokes cmake to generate native build system configuration files.
Currently only VS2008 is supported. For building under Linux, you should use the Makefile which is under the source tree.
"""

import os, os.path, errno, subprocess, shutil, glob, distutils.dep_util, logging, platform

class CommandLineTool:
    def __init__(self, config, logger, exit_on_fail = True):
        self.config       = config
        self.logger       = logger
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

class MakeConfiguration:
    def __init__(self, makefile_path, force_rebuild=False):
        self.makefile_path = makefile_path
        self.force_rebuild = force_rebuild

class MakeBuilder(CommandLineTool):
    def __init__(self, config, logger):
        CommandLineTool.__init__(self, config, logger)

    def Execute(self):
        logger = self.logger
        config = self.config

        command = 'make -j 8' \
                + " -C " + config.makefile_path
        if config.force_rebuild:
            command += ' -B'
        self.RunCommand(command)

class CMakeConfiguration:
    def __init__(self, path_sources, path_buildtree, path_bin, build_type, enable_warnings):
        self.path_sources    = path_sources
        self.path_buildtree  = path_buildtree
        self.path_bin        = path_bin
        self.enable_warnings = enable_warnings
        self.build_type      = build_type

class CMakeBuilder(CommandLineTool):
    def __init__(self, config, logger):
        CommandLineTool.__init__(self, config, logger)

    def Execute(self):
        logger = self.logger
        config = self.config

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
        command = "cmake"
        command += " -DLLVM_ENABLE_WERROR:BOOL=ON "
        #command += " -DCMAKE_BINARY_DIR:PATH=\"" + config.path_buildtree + "\""
        #command += " -DCMAKE_INSTALL_PREFIX:PATH=\"" + config.path_bin + "\""
        #command += " -DLLVM_TARGETS_TO_BUILD:STRING=\"X86\""
        command += " -DCMAKE_BUILD_TYPE:STRING=" + config.build_type

        #if config.enable_warnings:
        #    command += " -DLLVM_ENABLE_WARNINGS:BOOL=\"1\""
        
        command += ' ' + config.path_sources
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
    default_enable_warning = False
    # parse options
    parser = OptionParser()
    parser.add_option("-v", "--verbose-level", dest="verbose_level", default='info',
            help="Verbosity level, possible values: debug, info, warning, error, critical")
    parser.add_option("-l", "--log-file", type="string", dest="log_file", default="", #default is log to stdout
            help="Name of lof file to replace stdout")
    parser.add_option("-w", "--enable-warnings", dest="enable_warnings", action="store_true", default=False,
            help="Enable warnings")
    parser.add_option("-c", "--config", dest="config", type="string", default=None,
            help="Build the VS solution after it is generated. Build config is the configuration name e.g. Debug, Release")
    parser.add_option("-b", "--build", action="store_true", dest="build",
            help="Build the VS solution after it is generated. Build config is the configuration name e.g. Debug, Release")

    (options, args) = parser.parse_args()

    if options.verbose_level not in LOGGING_LEVELS:
        parser.error("Invalid verbosity level: "+options.verbose_level)

    # setup logger
    level  = LOGGING_LEVELS[options.verbose_level]
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

    cur_dir       = os.getcwd()
    llvm_make_dir = os.path.abspath(cur_dir+'/Linux64/'+options.config)
    builtin_dir   = os.path.abspath(cur_dir+'/../libraries/ocl_builtins/')


       
    # Run cmake for Volcano
    config       = CMakeConfiguration(path_sources = os.path.abspath(cur_dir+'/../llvm/'),
        path_buildtree  = llvm_make_dir,
        path_bin        = llvm_make_dir+'/'+options.config,
        build_type      = options.config,
        enable_warnings = options.enable_warnings)
            
    cmakebuilder = CMakeBuilder(config, logger)
    cmakebuilder.Execute()

    # Run make
    if options.build:
        # Build Volcano
        config = MakeConfiguration(llvm_make_dir)
        MB     = MakeBuilder(config, logger)
        MB.Execute()

        # Install binaries
        dst_dir   = os.path.abspath(os.path.join(llvm_make_dir, 'bin'))
        file_list  = list(glob.iglob(os.path.join(os.path.join(llvm_make_dir, 'lib', '*.so'))))
        file_list += list(glob.iglob(os.path.join(os.path.join(llvm_make_dir, 'lib', 'clbltfn*.rtl'))))
        file_list += list(glob.iglob(os.path.join(builtin_dir, 'bin/svml/Linux64/__ocl_svml_*.so')))
        for path in file_list:
            dst_path = os.path.join(dst_dir, path)
            dst = os.path.join(dst_dir,os.path.split(path)[1])
            if True or platform.system()=='Windows':
                if distutils.dep_util.newer(path, dst):
                    print path+' -> '+dst_dir
                    shutil.copy(path, dst_dir)
            else:
                if not os.path.exists(dst):
                    print dst+' -> '+path
                    os.symlink(path, dst)

        dst_dir_lib   = os.path.abspath(os.path.join(llvm_make_dir, 'lib'))
        file_list_lib = list(glob.iglob(os.path.join(builtin_dir, 'bin/svml/Linux64/__ocl_svml_*.so')))

        for path_lib in file_list_lib:
            dst_path_lib = os.path.join(dst_dir_lib, path_lib)
            dst_lib = os.path.join(dst_dir_lib,os.path.split(path_lib)[1])
            if True or platform.system()=='Windows':
                if distutils.dep_util.newer(path_lib, dst_lib):
                    print path_lib+' -> '+dst_dir_lib
                    shutil.copy(path_lib, dst_dir_lib)
            else:
                if not os.path.exists(dst_lib):
                    print dst_lib+' -> '+path_lib
                    os.symlink(path, dst_lib)

if __name__ == "__main__":
    import sys
    import platform
    if platform.platform().startswith("CYGWIN"):
        print "Cygwin Python is not supported. Please use ActiveState Python."
        sys.exit(1);
    if sys.version_info < (2, 6):
        print "Python version 2.6 or later required"
        sys,exit(1)
    main_result = main()
    sys.exit(main_result)

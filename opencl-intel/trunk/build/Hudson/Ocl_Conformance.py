import os.path, sys, subprocess, platform, shutil
from optparse import OptionParser

def RunProc(command):
    print command
    sys.stdout.flush()
    proc = subprocess.Popen(command, stderr=subprocess.STDOUT, shell=True)
    stdoutdata = proc.communicate()[0]

    print stdoutdata
    sys.stdout.flush()

    if proc.returncode != 0:
        print "failed"
        sys.exit(1)

def main():
    
    CONFIG = ['tests_pre_commit', 'tests_nightly']
    TARGET_TYPE = ['Win32', 'Win64', 'Linux64']
    BUILD_TYPE = ['Release', 'Debug']
    
    parser = OptionParser()
    parser.add_option('-c', '--config', dest='config', help='Name of tests configuration to run. Allowed values: ' + str(CONFIG))
    parser.add_option('-t', '--target_type', dest='target_type', help='Target type. Allowed values:  ' + str(TARGET_TYPE))
    parser.add_option('-b', '--build_type', dest='build_type', help='Build type. Possible values. Allowed values: ' + str(BUILD_TYPE))
    
    parser.add_option('-d', '--do_build', dest='do_build', help='Builds Volcano, default: False', action='store_true')
    
    options = parser.parse_args()[0]
     
    config = options.config
    target_type = options.target_type
    build_type = options.build_type
    do_build = options.do_build
    
    if config not in CONFIG:
        print 'Illegal coniguration: ', config, '. Allowed values: ', str(CONFIG)
        sys.exit(-1)
        
    if target_type not in TARGET_TYPE:
        print 'Illegal target type: ', target_type, '. Allowed values: ', str(TARGET_TYPE)
        sys.exit(-1)
        
    if build_type not in BUILD_TYPE:
        print 'Illegal build type: ', build_type, '. Allowed values: ', str(BUILD_TYPE)
        sys.exit(-1) 

    builtins_target_type = target_type
    SVML_target_type =  target_type
    backend_prefix = ''
    backend_extention = 'dll'
    if target_type == 'Win64':
        builtins_target_type = 'x64'
        SVML_target_type = builtins_target_type
    elif target_type== 'Linux64':
        SVML_target_type = 'lin64'
        backend_prefix = 'lib'
        backend_extention = 'so'
        
    
    build_home_dir = os.path.dirname(os.getcwd()) #  build_home_dir = trunk/build   
    trunk_dir = os.path.dirname(build_home_dir)
    build_system_dir = os.path.join(build_home_dir, target_type)

    if platform.system() == 'Windows':
        build_bin_dir = os.path.join(build_system_dir, 'bin',  build_type)
        backend_dir = build_bin_dir
        pass
    else:
        build_system_dir = os.path.join(build_system_dir, build_type)
        build_bin_dir = os.path.join(build_system_dir, 'bin')
        backend_dir = os.path.join(build_system_dir, 'lib')

    
    # clean everything
    if False and os.path.exists(build_system_dir):
        print "Cleaning " + build_system_dir 
        shutil.rmtree(build_system_dir)
    
    os.chdir(build_home_dir)
    print os.getcwd()
    
    # Run Setup script
    if do_build:
        print 'Building Volcano:'
        
        if platform.system() == 'Windows':
            command = 'python setup_win.py'
            command += ' -t ' + target_type 
            command += ' -b ' + build_type 
            command += ' -v debug'
            RunProc(command)
        else:
            command = 'python setup_lin.py'
            command += ' -c ' + build_type 
            command += ' -v debug'
            RunProc(command)
            
            os.chdir(build_home_dir)
            print os.getcwd()
            
            command = 'make'
            command += ' -j8'
            RunProc(command)
            

    # Built-ins
    builtinsDir = os.path.join(trunk_dir, 'libraries', 'ocl_builtins')
    
    if do_build:
        print 'Building builtins:'
    
        if platform.system() == 'Windows':
            os.chdir(os.path.join(build_home_dir, 'CompileBuiltins', 'src'))
            print os.getcwd()
            command = 'python CompileBuiltins.py'
            command += ' --config=' + build_type 
            command += ' --platform=' + builtins_target_type 
            command += ' --solution=' + os.path.join(builtinsDir, 'clbltfn.sln')
            RunProc(command)
        else:
            os.chdir(builtinsDir)
            print os.getcwd()
            command = 'gen_linux.sh'
            command += ' ' + build_type  
            command += ' ' + os.path.join(builtins_target_type, build_type) # Destination dir
            RunProc(command)
            
            os.chdir(os.path.join(builtinsDir, builtins_target_type, build_type))
            print os.getcwd()
            command = 'make'
            command += ' -j8'
            RunProc(command)
            
    print 'Running conformance:'
    
    # SVML
    svmlDir = os.path.join(builtinsDir, 'bin', 'svml')
   
    # Regression suite
    os.chdir(os.path.join(build_home_dir, 'BackEndValidation'))
    print os.getcwd()
    command = 'python BackEndValidator.py'
    command += ' --config=' + config + '.cfg' 
    command += ' --backend=' + os.path.join(backend_dir, backend_prefix + 'OclCpuBackEnd.' + backend_extention)
    command += ' --func=' + os.path.join(builtinsDir, builtins_target_type, build_type)
    command += ' --svml=' + os.path.join(svmlDir, SVML_target_type)
    command += ' --target-platform=' + target_type
    command += ' --work-dir=' + build_bin_dir
    command += ' --run-only' # no need to copy files again 
    RunProc(command)
    

if __name__ == "__main__":
        if platform.platform().startswith("CYGWIN"):
            print "Cygwin Python is not supported. Please use ActiveState Python."
            sys.exit(-1);
        if sys.version_info < (2, 6):
            print "Python version 2.6 or later required"
            sys.exit(-1);

        main_result = main()
        sys.exit(main_result)


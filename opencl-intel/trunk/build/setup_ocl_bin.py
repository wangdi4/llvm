import sys
import platform
import subprocess, os, glob
from optparse import OptionParser

def RunProc(command, die_on_fail=True):
    Shell = True
    if platform.system() == 'Windows':
        Shell = False
    print command
    proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=Shell)
    (stdoutdata, stderrdata) = proc.communicate()
    print stdoutdata

    if proc.returncode != 0:
        if die_on_fail:
            sys.exit(1)

def main():
    available_targets = ["win32", "win64", "linux64"]
    default_target = "Linux64"
    z7_cmd = "/usr/intel/bin/7z -y"
    default_path_sources = '/nfs/iil/disks/cvcc/OclConformance_14756/trunk/ReleaseCriteria/'
    if platform.system() == 'Windows':
        default_target = None
        z7_cmd = "C:/Program Files/7-Zip/7z.exe -y"
        default_path_sources = '//isamba.iil.intel.com' + default_path_sources

    available_configs = ["debug", "release"]
    # parse options
    parser = OptionParser()
    parser.add_option("-t", "--target",
            action="store", type="string", dest="target", default=default_target,
            help="Build target, possible values are: Win32, Win64")
    parser.add_option("-c", "--config", dest="config", type="string",
            help="Build, possible values are: Debug, Release")
    parser.add_option("-p", "--path-cl_bin", type="string", dest="path_sources", default=default_path_sources,
            help="Path to source. Default is " + default_path_sources)

    (options, args) = parser.parse_args()
    
    if options.target == None or options.target.lower() not in available_targets:
        parser.error("Invalid target: " + str(options.target))
    if options.config.lower() not in available_configs:
        parser.error("Invalid config: " + str(options.config))


    if platform.system() == 'Windows':
        target_dir = os.path.join(options.target, 'bin', options.config)
    else:
        # Linux
        target_dir = os.path.join(options.target, options.config, 'bin')

    try:
        os.makedirs(target_dir)
    except OSError:
        pass

    os.chdir(target_dir)

    # Extract cl_prod
    RunProc(z7_cmd + ' x ' + options.path_sources + '/' + options.target + '_Release.7z')

    # Extract Workloads
    RunProc(z7_cmd + ' x ' + options.path_sources + '/Workloads.7z')

    workloadsPath = os.path.realpath(os.path.join(os.path.curdir, 'Workloads'))
    for root, dirs, files in os.walk(workloadsPath):
        for name in files:
            if name.endswith('.cfg'):
                fname = os.path.realpath(os.path.join(root,name))
                RunProc("sed -i -e s#\.\.\/\.\.\/\.\.\/Workloads#Workloads# " + fname)
     

if __name__ == "__main__":
    if platform.platform().startswith("CYGWIN"):
        print "Cygwin Python is not supported. Please use ActiveState Python."
        sys.exit(1);
    if sys.version_info < (2, 6):
        print "Python version 2.6 or later required"
        sys.exit(1)
    main_result = main()
    sys.exit(main_result)



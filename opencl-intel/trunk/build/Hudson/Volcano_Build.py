import os,sys,platform
from Volcano_Tasks import BINARIES_ARCH_NAME, UnarchiverTask
from optparse import OptionParser
import Volcano_CmdUtils
from Volcano_Common import VolcanoRunConfig, VolcanoTestRunner, VolcanoTestSuite
from Volcano_Tasks import SimpleTest



class VolcanoBuilder(VolcanoTestSuite):
    def __init__(self, config):
        VolcanoTestSuite.__init__(self, "Build")
        self.config = config
        self.generate_children_report = False

        self.addTask(SimpleTest('Cleanup', config.build_dir, 'python cleanup.py -r ' + config.root_dir + ' -t ' + config.target_type + ' -b ' + config.build_type), stop_on_failure=True)
    
        if platform.system() == 'Windows':
            # Run Setup and Build script, also builds OCL built-ins
            self.addTask(SimpleTest('BuildWin',config.build_dir, 'python setup_win.py -t ' + config.target_type + ' -b ' + config.build_type + ' -v debug'), stop_on_failure=True)
        else:
            # Linux
            self.addTask(SimpleTest('BuildLin',config.build_dir, 'python setup_lin.py -c ' + config.build_type + ' -b -v debug'), stop_on_failure=True)


class VolcanoBinaryCopy(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.config = config
        self.generate_children_report = False
    
        self.addTask(SimpleTest('Cleanup', config.build_dir, 'python cleanup.py -r ' + config.root_dir + ' -t ' + config.target_type + ' -b ' + config.build_type), stop_on_failure=True)
    
        if platform.system() == 'Windows':
            # Run Setup and Build script, also builds OCL built-ins
            self.addTask(SimpleTest('BuildWin',config.build_dir, 'python setup_win.py -t ' + config.target_type + ' -v debug'), stop_on_failure=True)
        else:
            # Linux
            self.addTask(SimpleTest('BuildLin',config.build_dir, 'python setup_lin.py -c ' + config.build_type + ' -v debug'), stop_on_failure=True)
            
        self.addTask(UnarchiverTask('Prepare_Binaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True, always_pass = False)


def main():
    parser = OptionParser()
    parser.add_option("-r", "--root",   dest="root_dir",    help="project root directory", default=None)
    parser.add_option("-t", "--target", dest="target_type", help="target type: Win32/64,Linux64", default="Win32")
    parser.add_option("-b", "--build",  dest="build_type",  help="build type: Debug, Release", default="Release")
    parser.add_option("-d", "--demo",   dest="demo_mode",   action="store_true", help="Do not execute the command, just print them", default=False)
    
    (options, args) = parser.parse_args()

    Volcano_CmdUtils.demo_mode = options.demo_mode 

    config = VolcanoRunConfig(options.root_dir, 
                              options.target_type, 
                              options.build_type,
                              'corei7',
                              '',
                              '1')
    suite  = VolcanoBuilder(config)
    runner = VolcanoTestRunner()
    passed = runner.runTask(suite, config)
    
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

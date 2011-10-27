import os.path, sys, platform, traceback
from Volcano_Common import VolcanoRunConfig
from Hudson_Common import HudsonTestRunner, HudsonBuildEnvironment
from Volcano_Build import VolcanoBuilder
from Volcano_Tasks import ArchiverTask, BINARIES_ARCH_NAME
import Volcano_CmdUtils

# Initialize Globals

def main():
    env       = HudsonBuildEnvironment()
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config    = VolcanoRunConfig(trunk_dir, 
                                 env.getTargetPlatform(), 
                                 env.getBuildType(),
                                 'corei7', #not used
                                 '',  # not used
                                 '1') # not used
    suite     = VolcanoBuilder('Build', config)
    suite.addTask(ArchiverTask('Archive_Binaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True, always_pass = False)
    runner    = HudsonTestRunner(config, 'Build')
    passed    = False
    
    try:
        passed = runner.runTask(suite, config)
    except:
        print "Unexpected error:"
        traceback.print_exc()        
        raise
    finally:
        runner.publishResults()
    
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

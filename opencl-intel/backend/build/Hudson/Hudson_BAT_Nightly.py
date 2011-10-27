import os.path, sys, platform, traceback
from Volcano_Common import VolcanoRunConfig, VolcanoTestSuite
from Hudson_Common import HudsonTestRunner, HudsonBuildEnvironment
from Volcano_Nightly import VolcanoNightlyBAT
from Volcano_Build import VolcanoBuilder
import Volcano_CmdUtils



class HudsonPreCommit(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.addTask(VolcanoBuilder('Build', config),stop_on_failure = True, always_pass = False)
        self.addTask(VolcanoNightlyBAT("Tests", config), stop_on_failure = True, always_pass = False)

def main():
    env       = HudsonBuildEnvironment()
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config    = VolcanoRunConfig(trunk_dir, 
                                 env.getTargetPlatform(), 
                                 env.getBuildType(),
                                 env.getCPUType(),
                                 env.getTransposeSize())
    suite     = HudsonPreCommit('', config)
    runner    = HudsonTestRunner(config, 'Nightly_Tests')
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

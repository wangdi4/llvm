import os.path, sys, platform, traceback
from Volcano_Common import VolcanoTestSuite
from Hudson_Common import HudsonTestRunner, HudsonRunConfig
from Volcano_BAT import VolcanoBAT
from Volcano_Build import VolcanoBuilder
import Volcano_CmdUtils



class HudsonPreCommit(VolcanoTestSuite):
    def __init__(self, name, config):
        VolcanoTestSuite.__init__(self, name)
        self.addTask(VolcanoBuilder(config),stop_on_failure = True, always_pass = False)
        self.addTask(VolcanoBAT("Tests", config), stop_on_failure = True, always_pass = False)

def main():
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config    = HudsonRunConfig(trunk_dir)
    suite     = HudsonPreCommit('', config)
    runner    = HudsonTestRunner(config, '')
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

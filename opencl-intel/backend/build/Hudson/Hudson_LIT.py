import os.path, sys, platform, traceback
from framework.hudson.core import HudsonTestRunner
from Hudson_Common import HudsonBuildEnvironment
from Volcano_Common import VolcanoRunConfig
from Volcano_LIT import VolcanoLIT

def main():
    env       = HudsonBuildEnvironment()
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config    = VolcanoRunConfig(trunk_dir, 
                                 env.getTargetPlatform(), 
                                 env.getBuildType(),
                                 'corei7', #not used
                                 '',  # not used
                                 '1') # not used
    suite     = VolcanoLIT("LITTests", config)
    runner    = HudsonTestRunner(config, 'LIT_Tests')
    passed    = False
    
    try:
        passed = runner.runTask(suite, config)
    except:
        print "Unexpected error:"
        traceback.print_exc()        
        raise
    finally:
        runner.publishResults()
    
    return 0 if passed else 1

if __name__ == "__main__":
    if platform.platform().startswith("CYGWIN"):
        print "Cygwin Python is not supported. Please use ActiveState Python."
        sys.exit(1);
    if sys.version_info < (2, 6):
        print "Python version 2.6 or later required"
        sys,exit(1)
    main_result = main()
    sys.exit(main_result)

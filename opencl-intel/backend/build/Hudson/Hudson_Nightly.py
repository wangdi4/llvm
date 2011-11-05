import os.path, sys, platform
from Hudson_Common import HudsonTestRunner, HudsonRunConfig
from Volcano_Nightly import VolcanoNightlyBAT
from Volcano_Build import VolcanoBinaryCopy

def main():
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config    = HudsonRunConfig(trunk_dir)
    suite     = VolcanoNightlyBAT('Tests',config)
    suite.insertTask(0, VolcanoBinaryCopy("CopyVolcanoBinary", config),stop_on_failure = True)
    runner    = HudsonTestRunner(config, 'Tests')
    passed    = False
    
    try:
        passed = runner.runTask(suite, config)
    except:
        print "Unexpected error:", sys.exc_info()[0]
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

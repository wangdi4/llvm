
'''
Reference conformance testing in Hudson
'''

from Hudson_Common import HudsonTestRunner, HudsonRunConfig
from Volcano_Build import VolcanoBinaryCopy
from Volcano_Reference_Conformance_PostCommit import VolcanoReferenceConformancePostCommit
from Volcano_Tasks import BINARIES_ARCH_NAME, UnarchiverTask
import os
import platform
import sys

def     main():
    print "Hudson OCL_Ref_Test started.."
    print "Build Environment done.."
    trunk_dir = os.path.join(os.getcwd(), 'trunk')
    config = HudsonRunConfig(trunk_dir)
    print "Config done.."
    suite = VolcanoReferenceConformancePostCommit("Conformance_Test_Suite", config)
    print "Suite done.."
    suite.insertTask(0, UnarchiverTask('Prepare_Binaries', os.path.join(config.root_dir,BINARIES_ARCH_NAME), config.bin_dir), stop_on_failure = True, always_pass = False)
    print "Tasks added.."
    runner = HudsonTestRunner(config, 'Ocl_Ref_Conformance_tests')
    print "Running tasks.."
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
            sys, exit(1)
        main_result = main()
        sys.exit(main_result)




from Volcano_Common import VolcanoRunConfig
from framework.hudson.core import HudsonEnvironment
    
#Volcano Hudson specific global default paths
VOLCANO_JENKINS_URL = 'http://cvcc-fc14-01.iil.intel.com:8080'
    
    
class HudsonBuildEnvironment(HudsonEnvironment):
    """
    Role: Volcano Hudson Job specific environment wrapper
    Responsibility:
        Provides information about Volcano specific settings, like Volcano Labels mapping to target platform and cpu information
    """
    def getTargetPlatform(self):
        return self.getParam('Target_Type')

    def getCPUType(self):
        return self.getParam('Cpu_Arch')

    def getTransposeSize(self):
        return self.getParam('Transpose_Size')

    def getBuildType(self):
        return self.getParam('Build_Type')

class HudsonRunConfig(VolcanoRunConfig):
    def __init__(self, root_dir ):
        env = HudsonBuildEnvironment()
        cpu = env.getCPUType()
        cpu_features = ''
        if( cpu == 'avx128'):
            #currently the AVX256 is not supported for sandybridge 
            cpu_features = '-avx256'
        VolcanoRunConfig.__init__(self, 
                                  root_dir,
                                  env.getTargetPlatform(), 
                                  env.getBuildType(),
                                  cpu,
                                  cpu_features,
                                  env.getTransposeSize())


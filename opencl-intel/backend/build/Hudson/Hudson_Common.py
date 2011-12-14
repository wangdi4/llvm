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
        targetType = self.getParam('Target_Type')
        if( targetType == '' ):
            # WORKAROUND: Maps a Hudson job label to a target string.
            #             Enables the legacy jobs to continue to work with old configurations
            label2TargetMap = {'Volcano_Linux64':'Linux64', 'Volcano_SLES11': 'Linux64', 'Volcano_Win32': 'Win32', 'Volcano_Win64': 'Win64', 'Volcano_Win32_snb': 'Win32', 'Volcano_Win64_snb': 'Win64', 'NN_Win32': 'Win32', 'NN_Win64': 'Win64' }
            targetType = label2TargetMap[ self.getParam('label')]
        return targetType     

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


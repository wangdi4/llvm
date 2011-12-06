#ifndef __VEC_OPTIONS__H__
#define __VEC_OPTIONS__H__

#include "TargetArch.h"

namespace intel {

class OptimizerConfig
{
public:
    OptimizerConfig( int cpuId, int tranposeSize, int features,
            std::vector<int> dumpIROptionAfter,
            std::vector<int> dumpIROptionBefore,
            std::string dumpIRDir,
            bool debugInfo,
            bool disableOpt,
            bool relaxedMath,
            bool libraryModule):
      m_cpuId(cpuId),
      m_transposeSize(tranposeSize),
      m_cpuFeatures(features),
      m_dumpIROptionsAfter(dumpIROptionAfter),
      m_dumpIROptionsBefore(dumpIROptionBefore),
      m_dumpIRDir(dumpIRDir),
      m_debugInfo(debugInfo),
      m_disableOpt(disableOpt),
      m_relaxedMath(relaxedMath),
      m_libraryModule(libraryModule)
    {}

    int GetCpuId() const { return m_cpuId; }
    int GetTransposeSize() const { return m_transposeSize; }
    int GetCpuFeatures() const{ return m_cpuFeatures; }

    const std::vector<int>* GetIRDumpOptionsAfter() const{ return &m_dumpIROptionsAfter; }
    const std::vector<int>* GetIRDumpOptionsBefore() const{ return &m_dumpIROptionsBefore; }
    const std::string& GetDumpIRDir() const{ return m_dumpIRDir; }
    bool GetDisableOpt()    const { return m_debugInfo; }
    bool GetDebugInfoFlag() const { return m_disableOpt; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    // Sets whether optimized code is library module or not (contains kernels)
    // If this options is set to true then some optimization passes will be skipped
    bool GetLibraryModule()   const { return m_libraryModule; }

private:
    int m_cpuId;
    int m_transposeSize;
    int m_cpuFeatures;

    std::vector<int> m_dumpIROptionsAfter;
    std::vector<int> m_dumpIROptionsBefore;
    const std::string m_dumpIRDir;
    bool m_debugInfo; 
    bool m_disableOpt;
    bool m_relaxedMath;
    // Sets whether optimized code is library module or not (contains kernels and barriers)
    // If this options is set to true then some optimization passes will be skipped
    bool m_libraryModule;
};


}

#endif

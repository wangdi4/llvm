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
            std::string dumpIRDir):
      m_cpuId(cpuId),
      m_transposeSize(tranposeSize),
      m_cpuFeatures(features),
      m_dumpIROptionsAfter(dumpIROptionAfter),
      m_dumpIROptionsBefore(dumpIROptionBefore),
      m_dumpIRDir(dumpIRDir)
    {}

    int GetCpuId() const { return m_cpuId; }
    int GetTransposeSize() const { return m_transposeSize; }
    int GetCpuFeatures() const{ return m_cpuFeatures; }
    const std::vector<int>* GetIRDumpOptionsAfter() const{ return &m_dumpIROptionsAfter; }
    const std::vector<int>* GetIRDumpOptionsBefore() const{ return &m_dumpIROptionsBefore; }
    const std::string& GetDumpIRDir() const{ return m_dumpIRDir; }

private:
    int m_cpuId;
    int m_transposeSize;
    int m_cpuFeatures;
    std::vector<int> m_dumpIROptionsAfter;
    std::vector<int> m_dumpIROptionsBefore;
    const std::string m_dumpIRDir;
};

}

#endif

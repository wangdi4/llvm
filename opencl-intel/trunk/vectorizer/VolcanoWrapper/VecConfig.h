#ifndef __VEC_OPTIONS__H__
#define __VEC_OPTIONS__H__

#include "TargetArch.h"

namespace intel {

class OptimizerConfig
{
public:
    OptimizerConfig( int cpuId, int tranposeSize, int features):
      m_cpuId(cpuId),
      m_transposeSize(tranposeSize),
      m_cpuFeatures(features){}

    int GetCpuId() const { return m_cpuId; }
    int GetTransposeSize() const { return m_transposeSize; }
    int GetCpuFeatures() const{ return m_cpuFeatures; }

private:
    int m_cpuId;
    int m_transposeSize;
    int m_cpuFeatures;
};

}

#endif

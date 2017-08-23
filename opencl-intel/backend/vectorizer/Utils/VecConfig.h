/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __VEC_OPTIONS__H__
#define __VEC_OPTIONS__H__

#include "TargetArch.h"
#include "ICLDevBackendOptions.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace intel {

class OptimizerConfig
{
public:
    OptimizerConfig( const Intel::CPUId &cpuId, ETransposeSize tranposeSize,
            std::vector<int> dumpIROptionAfter,
            std::vector<int> dumpIROptionBefore,
            std::string dumpIRDir,
            bool debugInfo,
            bool profiling,
            bool disableOpt,
            bool relaxedMath,
            bool libraryModule,
            bool fpgaEmulator,
            bool heuristicIR,
            int  APFLevel,
            int rtLoopUnrollFactor):
      m_cpuId(cpuId),
      m_transposeSize(tranposeSize),
      m_dumpIROptionsAfter(dumpIROptionAfter),
      m_dumpIROptionsBefore(dumpIROptionBefore),
      m_dumpIRDir(dumpIRDir),
      m_debugInfo(debugInfo),
      m_profiling(profiling),
      m_disableOpt(disableOpt),
      m_relaxedMath(relaxedMath),
      m_libraryModule(libraryModule),
      m_fpgaEmulator(fpgaEmulator),
      m_dumpHeuristicIR(heuristicIR),
      m_APFLevel(APFLevel),
      m_rtLoopUnrollFactor(rtLoopUnrollFactor)
    {}

    const Intel::CPUId &GetCpuId() const { return m_cpuId; }
    ETransposeSize GetTransposeSize() const { return m_transposeSize; }

    const std::vector<int>* GetIRDumpOptionsAfter() const{ return &m_dumpIROptionsAfter; }
    const std::vector<int>* GetIRDumpOptionsBefore() const{ return &m_dumpIROptionsBefore; }
    const std::string& GetDumpIRDir() const{ return m_dumpIRDir; }
    bool GetDisableOpt()    const { return m_disableOpt; }
    bool GetDebugInfoFlag() const { return m_debugInfo; }
    bool GetProfilingFlag() const { return m_profiling; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    bool isFpgaEmulator()   const { return m_fpgaEmulator; }
    int GetRTLoopUnrollFactor() const { return m_rtLoopUnrollFactor; }
    // Sets whether optimized code is library module or not (contains kernels)
    // If this options is set to true then some optimization passes will be skipped
    bool GetLibraryModule()   const { return m_libraryModule; }
    bool GetDumpHeuristicIRFlag() const {return m_dumpHeuristicIR; }
    int  GetAPFLevel() const { return m_APFLevel; }

private:
    Intel::CPUId m_cpuId;
    ETransposeSize m_transposeSize;

    std::vector<int> m_dumpIROptionsAfter;
    std::vector<int> m_dumpIROptionsBefore;
    const std::string m_dumpIRDir;
    bool m_debugInfo; 
    bool m_profiling;
    bool m_disableOpt;
    bool m_relaxedMath;
    // Sets whether optimized code is library module or not (contains kernels and barriers)
    // If this options is set to true then some optimization passes will be skipped
    bool m_libraryModule;
    // Sets whether we are working as fpga emulator
    bool m_fpgaEmulator;
    // Sets whether the vectorize should output heuristic LL IR inputs
    bool m_dumpHeuristicIR;
    // Auto prefetch disable options
    int  m_APFLevel;

    int m_rtLoopUnrollFactor;
};


}

#endif

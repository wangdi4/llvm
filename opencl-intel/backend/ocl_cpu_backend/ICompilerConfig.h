// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "ICLDevBackendOptions.h"
#include "common_dev_limits.h"

#include <string>
#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * Global compiler configuration interface.
 *
 * This configuration is used for all the instances of the compiler
 * Internally it is mapped to the global LLVM state
 */
class IGlobalCompilerConfig
{
public:
    virtual ~IGlobalCompilerConfig(){}
    /**
     * Time each pass, printing elapsed time for each on exit
     */
    virtual bool EnableTiming() const = 0;
    /**
     * File to append -stats and -timer output to
     */
    virtual std::string InfoOutputFile() const = 0;
    /**
     * Disables printing the stack dump upon crash - used
     * primarity to disable the SEH handling by llvm library
     * usually for SDE tracing support.
     */
    virtual bool DisableStackDump() const = 0;
    /**
     * Pass command line options to LLVM
     */
    virtual std::string LLVMOptions() const = 0;

    virtual DeviceMode TargetDevice() const = 0;
};

/**
 * Compiler configuration interface
 *
 * This configuration is used for specific instance of the compiler
 */
class ICompilerConfig
{
public:
    virtual ~ICompilerConfig(){}

    virtual std::string GetCpuArch() const = 0;
    virtual std::string GetCpuFeatures() const = 0;
    virtual ETransposeSize GetTransposeSize() const = 0;
    virtual int GetRTLoopUnrollFactor() const = 0;
    virtual bool GetUseVTune() const = 0;
    virtual size_t GetForcedPrivateMemorySize() const = 0;
    // sets whether we need built-in module to be loaded for current compiler
    virtual bool GetLoadBuiltins() const = 0;
    virtual std::vector<int> GetIRDumpOptionsAfter() const = 0;
    virtual std::vector<int> GetIRDumpOptionsBefore() const = 0;
    virtual std::string GetDumpIRDir() const = 0;
    virtual bool GetDumpHeuristicIRFlag() const = 0;
    virtual const std::string &GetStatFileBaseName() const = 0;

    virtual DeviceMode TargetDevice() const = 0;
};

}}}

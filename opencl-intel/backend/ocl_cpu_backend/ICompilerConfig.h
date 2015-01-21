/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ICompilerConfig.h

\*****************************************************************************/
#pragma once

#include "ICLDevBackendOptions.h"

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
    virtual ETransposeSize GetTransposeSize() const  = 0;
    virtual int GetRTLoopUnrollFactor() const = 0;
    virtual bool  GetUseVTune() const = 0;
    // sets whether we need built-in module to be loaded for current compiler
    virtual bool  GetLoadBuiltins() const = 0;
    virtual std::vector<int> GetIRDumpOptionsAfter() const = 0;
    virtual std::vector<int> GetIRDumpOptionsBefore() const = 0;
    virtual std::string GetDumpIRDir() const = 0;
    virtual bool GetDumpHeuristicIRFlag() const = 0;
    virtual const std::string &GetStatFileBaseName() const = 0;
};

}}}

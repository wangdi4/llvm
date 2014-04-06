/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUProgram.h

\*****************************************************************************/
// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE 
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS  
#pragma once

#include "Program.h"

namespace llvm {
  class ExecutionEngine;
  class Function;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {


class CPUProgram : public Program
{
public:
    CPUProgram()
      :m_pExecutionEngine(NULL), m_pBIModule(NULL) {}
    virtual ~CPUProgram();
    
    virtual void SetBuiltinModule(void* pModule) { m_pBIModule = pModule; }

    virtual void SetExecutionEngine(void* ee) { m_pExecutionEngine = (llvm::ExecutionEngine*)ee;}

    llvm::ExecutionEngine* GetExecutionEngine() { return m_pExecutionEngine; }
    
    void ReleaseExecutionEngine();

    void* GetPointerToFunction(llvm::Function* F);

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);

private:
    llvm::ExecutionEngine*  m_pExecutionEngine;
    void *                  m_pBIModule;

private:
    // Disable copy ctor and assignment operator
    CPUProgram( const CPUProgram& );
    bool operator = (const CPUProgram& );

};

}}} // namespace

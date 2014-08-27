/*****************************************************************************\

Copyright (c) Intel Corporation (2011,2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  FeMock.cpp

\*****************************************************************************/
#include <cstdlib>
#include <cstdio>
#include "plugin_interface.h"
#include "compile_data.h"
#include "source_file.h"

using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Frontend;
using namespace Intel::OpenCL::DeviceBackend;

struct FePluginMock: ICLFrontendPlugin {
  FePluginMock(){
  }

  void OnLink (const LinkData* data){
    printf ("on link was called\n");
  }

  void OnCompile (const CompileData* data){
    SourceFile file("my name is", "slim shady", "");
    const_cast<CompileData*>(data)->sourceFile(file);
  }
};

struct NullPlugin: ICLDevBackendPlugin{
  void OnCreateBinary(const ICLDevBackendKernel_* pKernel,
                      const _cl_work_description_type* pWorkDesc,
                      size_t bufSize,
                      void* pArgsBuffer){}

  void OnCreateKernel(const ICLDevBackendProgram_* pProgram,
                      const ICLDevBackendKernel_* pKernel,
                      const void* pFunction){}

  void OnCreateProgram(const void* pBinary,
                       size_t uiBinarySize,
                       const ICLDevBackendProgram_* pProgram){}

  void OnReleaseProgram(const ICLDevBackendProgram_* pProgram){}
};

struct DummyPlugin: IPlugin{
  FePluginMock* pfe;
  DeviceBackend::ICLDevBackendPlugin* pbe;

  DummyPlugin(): pfe(NULL), pbe(NULL){}

  ICLFrontendPlugin* getFrontendPlugin(){
    if (pfe)
      return pfe;
    pfe = new FePluginMock();
    return pfe;
  }

  DeviceBackend::ICLDevBackendPlugin* getBackendPlugin(){
    if(pbe)
      return pbe;
    pbe = new NullPlugin();
    return pbe;
  }

  ~DummyPlugin(){
    if(pfe) delete pfe;
    if(pbe) delete pbe;
  }
};

extern "C"{
  IPlugin* CreatePlugin(){
      printf ("creating plugin\n");
      return new DummyPlugin();
  }
  void ReleasePlugin(IPlugin* p){
      printf ("releasing plugin\n");
      if(p) delete p;
  }
}
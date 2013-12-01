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

File Name:  ImageCallbackManager.cpp

\*****************************************************************************/

#include <assert.h>
#include <string>
#include <memory>
#include "ImageCallbackManager.h"
#include "ImageCallbackLibrary.h"
#include "Compiler.h"
#include "CPUCompiler.h"
#include "MICCompiler.h"
#include "CompilationUtils.h"


//void RegisterBIFunctions(void);

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ImageCallbackManager* ImageCallbackManager::s_pInstance = NULL;

ImageCallbackManager::ImageCallbackManager()
{}

ImageCallbackManager::~ImageCallbackManager()
{
    for( ImageCallbackMap::iterator i = m_ImageCallbackLibs.begin(), e = m_ImageCallbackLibs.end(); i != e; ++i )
    {
        delete i->second;
    }
}

void ImageCallbackManager::Init()
{
    assert(!s_pInstance);
    s_pInstance = new ImageCallbackManager();
}

void ImageCallbackManager::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

ImageCallbackManager* ImageCallbackManager::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}


bool ImageCallbackManager::InitLibrary(const ICompilerConfig& config, bool isCpu, Intel::CPUId& cpuId)
{
  // MIC is not supported currently. Return.
  if(!isCpu)
      return true;
  std::auto_ptr<CPUCompiler> spCompiler;

  // Initialize compiler first to get archId and CPUFeatures
  if(isCpu){
      spCompiler = std::auto_ptr<CPUCompiler>(new CPUCompiler(config));
  } else {
      // TODO: enable with MIC is support
      //spCompiler = std::auto_ptr<Compiler>(new MICCompiler(config));
  }
  // Retrieve CPU ID
  cpuId = spCompiler->GetCpuId();
  // KNL is not supported
  if (cpuId.GetCPU() == CPU_KNL)
    return true;

  // Find library for this platform if it has been built earlier
  ImageCallbackMap::iterator it = m_ImageCallbackLibs.find(cpuId);
  if( it != m_ImageCallbackLibs.end() )
      return true;


  // ImageCallbackLibrary becomes the owner of compiler. So release compiler here
  std::auto_ptr<ImageCallbackLibrary> spLibrary(new ImageCallbackLibrary(cpuId, spCompiler.release()));
  spLibrary->Load();
  spLibrary->Build();

  if (! spLibrary->LoadExecutable()){
      return false;   // failed to load library to the device
  }
  m_ImageCallbackLibs[cpuId] = spLibrary.release();
  return true;
}

ImageCallbackFunctions* ImageCallbackManager::getCallbackFunctions(const Intel::CPUId &cpuId)
{
    ImageCallbackMap::iterator it = m_ImageCallbackLibs.find(cpuId);
    if (it ==  m_ImageCallbackLibs.end() )
    {
        throw Exceptions::CompilerException("Requested image function for library that hasn't been loaded");
    }

    return m_ImageCallbackLibs[cpuId]->getImageCallbackFunctions();
}


}}}

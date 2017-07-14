/*****************************************************************************\

Copyright (c) Intel Corporation (2011, 2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BE_DynamicLib.cpp

\*****************************************************************************/

#include "BE_DynamicLib.h"
#include <ocl_string_exception.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using namespace std;
using namespace Intel::OpenCL::Utils;

namespace Intel{ namespace OpenCL { namespace DeviceBackend { namespace Utils{

BE_DynamicLib::BE_DynamicLib(void) :
m_hLibrary(nullptr)
{
}

BE_DynamicLib::~BE_DynamicLib(void) 
{
  Close();
}

void BE_DynamicLib::Load(const char* pLibName)
{
  try
  {
    Intel::OpenCL::Utils::OclDynamicLib::Load(pLibName);
  }
  catch (ocl_string_exception errMsg)
  {
    throw Intel::OpenCL::DeviceBackend::Exceptions::DynamicLibException(errMsg.what());
  }
}

ptrdiff_t BE_DynamicLib::GetFuncPtr(const char* funcName)
{
  try
  {
    return GetFunctionPtrByName(funcName);
  }
  catch (ocl_string_exception errMsg)
  {
    throw Intel::OpenCL::DeviceBackend::Exceptions::DynamicLibException(errMsg.what());
  }
}

}}}}

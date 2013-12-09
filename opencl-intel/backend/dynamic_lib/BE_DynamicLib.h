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

File Name:  DynamicLib.h

\*****************************************************************************/

#pragma once

#include <stdexcept>
#include "cl_device_api.h"
#include <DynamicLib.h>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

namespace Exceptions{
class DynamicLibException: public std::runtime_error
{
public:
  virtual ~DynamicLibException() throw(){}
  DynamicLibException(std::string dllname) : runtime_error(dllname){
  }
};

}//namespace Exceptions

namespace Utils {

class BE_DynamicLib : public Intel::OpenCL::Utils::DynamicLib
{
public:
  BE_DynamicLib(void);
  ~BE_DynamicLib(void);

  // Loads a dynamically link library into process address space
  // Input
  //		pLibName	- A pointer to null terminated string that describes library file name
  void Load(const char* pLibName);

  // Release all allocated resourses and unloads the library
  void Close();

  // Returns the pointer to exported function within a loaded module
  void* GetFuncPtr(const char* funcName);

private:
  void* m_hLibrary;		// A handle to loaded library

};

}}}}

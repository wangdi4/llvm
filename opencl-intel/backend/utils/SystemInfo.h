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

File Name:  SystemInfo.h

\*****************************************************************************/

#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include <cstddef>
namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class SystemInfo
{
public:
  SystemInfo(void);
  ~SystemInfo(void);

  static unsigned long long HostTime();
  static void GetModuleDirectory(char* szModuleDir, size_t strLen);

};

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __SYSTEM_INFO_H__
/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OCLAddressSpace.h

\*****************************************************************************/

#ifndef __OCL_ADDRESS_SPACE_H__
#define __OCL_ADDRESS_SPACE_H__

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

class OCLAddressSpace
{
  public:
    enum spaces {
      Private = 0,
      Global = 1,
      Constant = 2,
      Local = 3,
      Global_EndianHost = 4,
      Constant_EndianHost = 5
    };
};

#define getAddressSpaceMask(ID) (1 << (ID))

inline bool isInSpace (int spaceID, int spaceMask) {
  return ((getAddressSpaceMask(spaceID) & spaceMask) != 0);
}

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __OCL_ADDRESS_SPACE_H__
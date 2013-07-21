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
      Generic = 4
    };
};

#define getAddressSpaceMask(ID) (1 << (ID))

inline bool isInSpace (int spaceID, int spaceMask) {
  return ((getAddressSpaceMask(spaceID) & spaceMask) != 0);
}

#define IS_ADDR_SPACE_PRIVATE(space)  (space == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Private)
#define IS_ADDR_SPACE_GLOBAL(space)   (space == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Global)
#define IS_ADDR_SPACE_CONSTANT(space) (space == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Constant)
#define IS_ADDR_SPACE_LOCAL(space)    (space == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Local)
#define IS_ADDR_SPACE_GENERIC(space)  (space == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Generic)

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __OCL_ADDRESS_SPACE_H__
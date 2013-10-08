/*=================================================================================
 Copyright (c) 2012, Intel Corporation
 Subject to the terms and conditions of the Master Development License
 Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
 OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
 ==================================================================================*/

#ifndef __OCL_ADDRESS_SPACE_H__
#define __OCL_ADDRESS_SPACE_H__

namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

namespace OCLAddressSpace
{
    enum spaces {
      Private = 0,
      Global = 1,
      Constant = 2,
      Local = 3,
      LastStaticAddrSpace = Local,
      Generic = 4
    };
}

#define getAddressSpaceMask(ID) (1 << (ID))

inline bool isInSpace (int spaceID, int spaceMask) {
  return ((getAddressSpaceMask(spaceID) & spaceMask) != 0);
}

#define IS_ADDR_SPACE_PRIVATE(space)  ((space) == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Private)
#define IS_ADDR_SPACE_GLOBAL(space)   ((space) == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Global)
#define IS_ADDR_SPACE_CONSTANT(space) ((space) == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Constant)
#define IS_ADDR_SPACE_LOCAL(space)    ((space) == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Local)
#define IS_ADDR_SPACE_GENERIC(space)  ((space) == Intel::OpenCL::DeviceBackend::Utils::OCLAddressSpace::Generic)

}}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Utils {

#endif // __OCL_ADDRESS_SPACE_H__

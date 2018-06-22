// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

#if INTEL_CUSTOMIZATION
//
// INTEL CONFIDENTIAL
//
// Modifications, Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
// Workaround for compatibility issue of libffi.
// We create a stub library with soname=libffi.so to be linked with x86_64
// plugin.
#include <ffi.h>
ffi_type ffi_type_void;
ffi_type ffi_type_pointer;
extern ffi_status ffi_prep_cif(ffi_cif *cif, ffi_abi abi, unsigned int nargs,
                               ffi_type *rtype, ffi_type **atypes) {
  return FFI_OK;
}

extern void ffi_call(ffi_cif *cif, void (*fn)(void), void *rvalue,
                     void **avalue) {
}
#endif // INTEL_CUSTOMIZATION

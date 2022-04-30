#if INTEL_CUSTOMIZATION
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

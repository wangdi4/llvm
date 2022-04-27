/* INTEL_FEATURE_XUCC */
/*===------------- vt_intrin.h - XuCC intrinsics ---------------------------===
 *
 * Copyright (C) 2022 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===-----------------------------------------------------------------------===
*/
#ifndef VT_INTRIN_INCLUDED
#define VT_INTRIN_INCLUDED

#ifndef __XUCC__
#error "Must enable xucc before including vt_intrin.h"
#endif

#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__))

/*
 * void _vmxon(void *vmxonp)
 *
 * Puts the logical processor in VMX operation with no current VMCS.  vmxonp
 * is a 4KB-aligned physical address that references the VMXON region.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmxon(void *vmxonp) {
  __asm__ ("vmxon %0" : : "m" (vmxonp) : "memory");
}

/*
 * void _vmxoff(void)
 *
 * Takes the logical processor out of VMX operation.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmxoff(void) {
  __asm__ ("vmxoff" : : : "memory");
}

/*
 * void _vmcall(void)
 *
 * Cause a VM exit, registering the appropriate exit reason.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmcall(void) {
  __asm__ ("vmcall" : : : "memory");
}

/*
 * void _vmlaunch(void)
 *
 * Launch virtual machine managed by the current VMCS.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmlaunch(void) {
  __asm__ ("vmlaunch" : : : "memory");
}

/*
 * void _vmresume(void)
 *
 * Resume virtual machine managed by the current VMCS.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmresume(void) {
   __asm__ ("vmresume" : : : "memory");
}

/*
 * void _vmclear(void *vmcsp)
 *
 * This function ensures that VMCS data for the VMCS specified by vmcsp are
 * copied to the VMCS region in memory.  It also initializes parts of the VMCS
 * region.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmclear(void *vmcsp) {
    __asm__ ("vmclear %0" : : "m" (vmcsp) : "memory");
}

/*
 * void _vmptrld(void *vmcsp)
 *
 * Marks the current VMCS pointer valid and loads it with vmcsp.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmptrld(void *vmcsp) {
    __asm__ ("vmptrld %0" : : "m" (vmcsp) : "memory");
}

/*
 * void* _vmptrst(void)
 *
 * Returns the current VMCS pointer.
 */
static __inline__ void* __DEFAULT_FN_ATTRS _vmptrst(void) {
    void *_vmcsp;
    __asm__ ("vmptrst %0" : "=m" (_vmcsp) : : "memory");
    return _vmcsp;
}

/*
 * void _vmwrite(__int64 field_id, __int64 value)
 *
 * Writes value to the VMCS field specific by field_id.  If the VMCS field is
 * shorter than 64 bits, the upper bits of value are ignored.  If the VMCS
 * field is longer than 64 bits, the high bits of the field are set to 0.
 */
static __inline__ void __DEFAULT_FN_ATTRS _vmwrite(__int64 field_id,
                                                   __int64 value) {
  __asm__("vmwrite %1, %0" : "=r"(field_id) : "r"(value) : "memory");
}


/*
 * __int64 _vmread(__int64 field_id)
 *
 * Reads the VMCS field specific by field_id.  If the VMCS field is shorter
 * than 64 bits, it is zero extended to 64 bits.  If the VMCS field is longer
 * than 64 bits, the upper bits are not read. Only the lower 64 bits are
 * returned.
 */
static __inline__ __int64 __DEFAULT_FN_ATTRS _vmread(__int64 field_id) {
  __int64 _result;
  __asm__("vmread %1, %0" : "=r"(_result) : "r"(field_id) : "memory");
  return _result;
}

/*
 * void _invept(void* descrp, __int64 type)
 *
 * Invalidate TLB entries and paging structure caches that were derived from
 * extended page tables.  Invalidation is based on the INVEPT type specified
 * by "type" and the INVEPT descriptor pointed to by "descrp".
 */
static __inline__ void _invept(void *descrp, __int64 type) {
  __asm__("invept %a1, %0" : : "r"(type), "r"(descrp) : "memory");
}

/*
 * void _invvpid(void* descrp, __int64 type)
 *
 * Invalidate TLB entries and paging structure caches based on
 * virtual-processor identifier.  Invalidation is based on the INVVPID type
 * specified by "type" and the INVVPID descriptor pointed to by descrp.
 */
static __inline__ void _invvpid(void *descrp, __int64 type) {
  __asm__("invvpid %a1, %0" : : "r"(type), "r"(descrp) : "memory");
}

#endif // VT_INTRIN_INCLUDED
/* end INTEL_FEATURE_XUCC */

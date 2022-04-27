/* INTEL_FEATURE_XUCC */

/*===------------ pppe_intrin.h - XuCC intrinsics --------------------------===
 *
 * Copyright (C) 2022 Intel Corporation. All rights reserved.
 *
 * The information and source code contained herein is the exclusive property
 * of Intel Corporation and may not be disclosed, examined or reproduced in
 * whole or in part without explicit written authorization from the company.
 *
 *===-----------------------------------------------------------------------===
*/

#ifndef PPPE_INTRIN_INCLUDED
#define PPPE_INTRIN_INCLUDED

#ifndef __XUCC__
#error "Must enable xucc before including pppe_intrin.h"
#endif

#define __DEFAULT_FN_ATTRS                                                     \
  __attribute__((__always_inline__, __nodebug__))

/*
 * loadstoreseg_param_t type is defined in 'ia32.h'.
 * typedef struct {
 * int base;
 * } loadstoreseg_param_t;
*/

/*
 * void _int1(void)
 *
 * This intrinsic issues an int1 instruction.  It is considered by the
 * compiler to have unpredictable side effects, so it effectively acts as a
 * memory barrier.
 */
static __inline__ void __DEFAULT_FN_ATTRS _int1(void) {
  __asm__ __volatile__("int1" : : : "memory");
}

/*
 * void _iret(void)
 *
 * This intrinsic issues the iret instruction.  It is considered by the
 * compiler to have unpredictable side effects, so it effectively acts as a
 * memory barrier.  The compiler may treat this intrinsic as "no return",
 * meaning that any code following the intrinsic may be assumed by the compiler
 * to be dead.
 */
static __inline__ void __DEFAULT_FN_ATTRS _iret(void) {
   __asm__ __volatile__("iret" : : : "memory");
}

/*
 * __int64 _spbusmsg_read(__int64 msg)
 *
 * This intrinsic issues a special bug message read where msg indicates the bus
 * message to issue.  The result data is returned.  The compiler considers this
 * intrinsic a memory barrier and will not reorder it relative to other memory
 * operations.
 */
// FIXME: The implementation of ICC is "spbusmsg %0, %1". It's strange that the
// '%1' is in the usual output location.
static __inline__ __int64 __DEFAULT_FN_ATTRS _spbusmsg_read(__int64 msg) {
    __int64 _spbusmsg_read_result;
    __asm__("spbusmsg %1, %0"
            : "=r"(_spbusmsg_read_result)
            : "r"(msg)
            : "memory");
    return _spbusmsg_read_result;
}

/*
 * void _spbusmsg_write(__int64 msg, __int64 data)
 *
 * This intrinsic issues a special bug message write where msg indicates the
 * bus message to issue, and data holds the package associated with the
 * message.  The compiler considers this intrinsic a memory barrier and will
 * not reorder it relative to other memory operations.
 */
// FIXME: what's the difference between _spbusmsg_read and _spbusmsg_write?
// FIXME: Is the order of msg and data right?
static __inline__ void __DEFAULT_FN_ATTRS _spbusmsg_write(__int64 msg,
                                                          __int64 data) {
  __asm__("spbusmsg %0, %1" : : "r"(data), "r"(msg) : "memory");
}

/*
 * void _spbusmsg_nodata(__int64 msg)
 *
 * This intrinsic issues a special bug message where msg indicates the bus
 * message to issue.  This intrinsic is used for messages that are neither read
 * nor write and have no associated data.  The compiler considers this
 * intrinsic a memory barrier and will not reorder it relative to other memory
 * operations.
 */
static __inline__ void __DEFAULT_FN_ATTRS _spbusmsg_nodata(__int64 msg) {
  __asm__("spbusmsg %0, %0" : : "r" (msg) :"memory");
}

/*
 * __int64 _guestloadlin(void *p)
 *
 * This intrinsic performs a load from linear address p subject to the guest
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a 64-bit
 * pointer, but the actual pointer size used in the load is determined by the
 * Asize field of the MSR.  The loaded data is returned as a 64-bit value, but
 * the actual size of the read data is determined by the Osize field of the
 * MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a1.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory read is not
 *                       explicit in the asm interface.
 */
static __inline__ __int64 __DEFAULT_FN_ATTRS _guestloadlin(void *p) {
  __int64 _guestload_result;
  __asm__("gmovlin %a1, %0" : "=r"(_guestload_result) : "r"(p) : "memory");
  return _guestload_result;
}

/*
 * void _gueststorelin(void *p, __int64 data)
 *
 * This intrinsic performs a store to linear address p subject to the guest
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a
 * 64-bit pointer, but the actual pointer size used in the store is determined
 * by the Asize field of the MSR.  The data to be stored is passed as a 64-bit
 * value, but the actual size of the written data is determined by the Osize
 * field of the MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a0.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory write is not
 *                       explicit in the asm interface.
 */
/*
 * FIXME: The format of gmovlin is: GMOVLIN r16/32/64, m16/32/64 (INTEL FORMAT).
 *        It doesn't have 'rm' format. However, ICC supports this.
static __inline__ void __DEFAULT_FN_ATTRS _gueststorelin(void *p,
                                                         __int64 data) {
  __asm__("gmovlin %1, %a0" : : "r"(p), "r"(data) : "memory");
}
*/

/*
 * __int64 _guestloadpphys(void *p)
 *
 * This intrinsic does a load from physical address p subject to the platform
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a 64-bit
 * pointer, but the actual pointer size used in the load is determined by the
 * Asize field of the MSR.  The loaded data is returned as a 64-bit value, but
 * the actual size of the read data is determined by the Osize field of the
 * MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a1.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory read is not
 *                       explicit in the asm interface.
 */
static __inline__ __int64 __DEFAULT_FN_ATTRS _guestloadpphys(void *p) {
  __int64 _ploadphys_result;
  __asm__("gmovpphys %a1, %0" : "=r"(_ploadphys_result) : "r"(p) : "memory");
  return _ploadphys_result;
}

/*
 * void _gueststorepphys(void *p, __int64 data)
 *
 * This intrinsic does a store to physical address p subject to the platform
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a 64-bit
 * pointer, but the actual pointer size used in the store is determined by the
 * Asize field of the MSR.  The data to be stored is passed as a 64-bit value,
 * but the actual size of the written data is determined by the Osize field of
 * the MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a0.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory write is not
 *                       explicit in the asm interface.
 */

static __inline__ void __DEFAULT_FN_ATTRS _gueststorepphys(void *p,
                                                           __int64 data) {
  __asm__("gmovpphys %1, %a0" : : "r"(p), "r"(data) : "memory");
}

/*
 * __int64 _guestloadgphys(void *p)
 *
 * This intrinsic performs a load from physical address p subject to the guest
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a 64-bit
 * pointer, but the actual pointer size used in the load is determined by the
 * Asize field of the MSR.  The loaded data is returned as a 64-bit value, but
 * the actual size of the read data is determined by the Osize field of the
 * MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a1.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory read is not
 *                       explicit in the asm interface.
static __inline__ __int64 __DEFAULT_FN_ATTRS _guestloadgphys(*p) {
  __int64 _guestloadgphys_result;
  __asm__("gmovgphys %a1, %0"
          : "=r"(_guestloadgphys_result)
          : "r"(p)
          : "memory");
  return _guestloadgphys_result;
}
*/

/*
 * void _gueststoregphys(void *p, __int64 data)
 *
 * This intrinsic performs a store to physical address p subject to the guest
 * context defined by the MILLICODE_MACROINSTRUCTION_ALIAS MSR.  p is a 64-bit
 * pointer, but the actual pointer size used in the store is determined by the
 * Asize field of the MSR.  The data to be stored is passed as a 64-bit value,
 * but the actual size of the written data is determined by the Osize field of
 * the MSR.
 *
 * Implementation note:  The memory operand to this intrinsic is implemented by
 *                       assigning the pointer value to a register and then
 *                       emitting it as an address using %a0.  This is
 *                       necessary to prevent the compiler from trying to use a
 *                       scaled index addressing mode, which would potentially
 *                       be wrong since the scale is read from the MSR, not the
 *                       instruction.  An implication of this choice is that we
 *                       may not make the most efficient use of legal
 *                       addressing modes such as base + offset.  Another
 *                       implication is that we must mark the intrinsic with a
 *                       memory clobber, because the memory write is not
 *                       explicit in the asm interface.
static __inline__ void __DEFAULT_FN_ATTRS _gueststoregphys(void *p,
                                                           __int64 data) {
  __asm__("gmovgphys %1, %a0" : : "r"(p), "r"(data) : "memory");
}
*/

/*
 * void _saveworld(void *p, __int64 mask)
 *
 * This intrinsic conditionally saves CR0/2/3/4, segment register, and EFER
 * state values specified by the "mask" parameter. These values are saved
 * into memory pointed to by "*p". mask is a 64bit bitfield containing
 * values that indicate which data needs to be saved.
 */
static __inline__ void __DEFAULT_FN_ATTRS _saveworld(void *p, __int64 mask) {
  __asm__("svworld %1, %a0" : : "r"(p), "r"(mask) : "memory");
}

/*
 * void _restoreworld(const void *p, __int64 mask)
 *
 * This intrinsic conditionally restores CR0/2/3/4, segment register, and EFER
 * state values specified by the "mask" parameter. These values are loaded from
 * the memory pointed to by "*p". mask is a 64bit bitfield containing values
 * that indicate which data needs to be restored.
 *
 * Implementation note:  We mark this intrinsic with a memory clobber, because
 *                       the memory read is not explicit in the asm interface.
 */
static __inline__ void __DEFAULT_FN_ATTRS _restoreworld(const void *p,
                                                        __int64 mask) {
  __asm__("rsworld %0, %a1" : : "r"(mask), "r"(p) : "memory");
}

//
// __int64 _gtranslate_rd_noepc(__int64 physaddr, __int64 linaddr, void *mem)
// __int64 _gtranslate_wr_noepc(__int64 physaddr, __int64 linaddr, void *mem)
// __int64 _gtranslate_rd_epc(__int64 physaddr, __int64 linaddr, void *mem)
// __int64 _gtranslate_wr_epc(__int64 physaddr, __int64 linaddr, void *mem)
//
// These intrinsics perform a tickle operation on the memory operand to check
// for fault conditions and return various memory attributes. The memory
// access is always 1 byte.
// Fault conditions are reported as XuCode exceptions.
//
// The memory parameter to the intrinsic determines the effective address
// of the operation.
//
// These intrinsics return the memory attributes returned by the paging/EPT
// hardware. This is stored in RCX, and is the return value of the intrinsics.
// Operands physaddr and linaddr are outputs that reflect the platform
// physical address, and linear address. physaddr is assigned to RAX, and
// linaddr is assigned to RBX.
//
// Different flavors are:
//     Read without EPC override
//     Read/write without EPC override
//     Read with EPC override
//     Read/write without EPC override
//
#define _gtranslate_intrinsics(flavor, physaddr, linaddr, mem)                 \
  ({                                                                           \
    __int64 _physaddr;                                                         \
    __int64 _linaddr;                                                          \
    __int64 _mem_attrs;                                                        \
    void *_mem = (mem);                                                        \
                                                                               \
    __asm__(#flavor " %3"                                                      \
            : "=c"(_mem_attrs), "=a"(_physaddr), "=b"(_linaddr)                \
            : "m"(*(char *)_mem));                                             \
                                                                               \
    (physaddr) = _physaddr;                                                    \
    (linaddr) = _linaddr;                                                      \
                                                                               \
    _mem_attrs;                                                                \
  })

#define _gtranslate_rd_noepc(physaddr, linaddr, mem)                           \
  _gtranslate_intrinsics(gtranslaterd_noepc, physaddr, linaddr, mem)
#define _gtranslate_wr_noepc(physaddr, linaddr, mem)                           \
  _gtranslate_intrinsics(gtranslatewr_noepc, physaddr, linaddr, mem)
#define _gtranslate_rd_epc(physaddr, linaddr, mem)                             \
  _gtranslate_intrinsics(gtranslaterd_epc, physaddr, linaddr, mem)
#define _gtranslate_wr_epc(physaddr, linaddr, mem)                             \
  _gtranslate_intrinsics(gtranslatewr_epc, physaddr, linaddr, mem)

//
// char _getbasekey(__int64 basekey_hi, __int64 basekey_lo, int svn)
//
// <TO DO : Details of this intrinsic need to be added at a later
//  point when we have more details on the intstruction. As of now,
//  there is no PAS, and we have very little information>
//
// This intrinsic takes a 32bit input in a register, which represents
// the requested SVN, where:
//    [7:0]  = PatchAtReset
//    [31:8] = LatePatch; [31:16] must be 0xFFFF (not enforced by the ISA)
//
// The return value of the intrinsic is the resulting ZF, which is
// set to either "0" (if RFLAGS.ZF == 0), or "1" (if RFLAGS.ZF == 1).
//
// The two output parameters: basekey_hi, and basekey_lo, will contain
// the hi64 and lo64 bits of the resulting 128bit TCB_BASE_KEY (RDX:RAX).
//
#define _getbasekey(basekey_hi, basekey_lo, svn)                               \
  ({                                                                           \
    __int64 _keylo;                                                            \
    __int64 _keyhi;                                                            \
    char _new_zf;                                                              \
    __int64 _svn = (__int64)(svn);                                             \
                                                                               \
    __asm__("getbasekey  %3\n\t"                                               \
            "sete %2"                                                          \
            : "=d"(_keyhi), "=a"(_keylo), "=r"(_new_zf)                        \
            : "r"(_svn));                                                      \
                                                                               \
    (basekey_lo) = _keylo;                                                     \
    (basekey_hi) = _keyhi;                                                     \
                                                                               \
    _new_zf;                                                                   \
  })

/*
 * __int64 _cmodeselect(__int64 false_val, __int64 true_val, __int64 cond)
 *
 * This intrinsic performs a conditional select based on specific pieces and
 * combinations of machine state as specified by the cond argument.  The
 * operation is
 *
 *    result = cond ? true_val : false_val;
 */
static __inline__ __int64 __DEFAULT_FN_ATTRS _cmodeselect(__int64 false_val,
                                                       __int64 true_val,
                                                       __int64 cond) {
  __int64 _cmodeselect_result;
  __asm__("cmodemov %2, %1, %0" :
          "=r" (_cmodeselect_result) :
          "r" (true_val),
          "c" (cond),
          "0" (false_val));
  return _cmodeselect_result;
}

/*
 * void _loadseg_cs(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * cs register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_cs(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%cs:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_cs(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * cs register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_cs(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%cs:%0" :
           "=m" (_storeseg_result.base) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _loadseg_ds(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * ds register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_ds(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%ds:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_ds(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * ds register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_ds(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%ds:%0" :
           "=m" (_storeseg_result) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _loadseg_es(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * es register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_es(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%es:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_es(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * es register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_es(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%es:%0" :
           "=m" (_storeseg_result) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _loadseg_fs(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * fs register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_fs(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%fs:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_fs(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * fs register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_fs(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%fs:%0" :
           "=m" (_storeseg_result) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _loadseg_gs(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * gs register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_gs(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%gs:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_gs(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * gs register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_gs(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%gs:%0" :
           "=m" (_storeseg_result) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _loadseg_ss(loadstoreseg_param_t seginfo)
 *
 * This intrinsic loads the segment information from seginfo into the hardware
 * ss register.
 *
 * Implementation note:  The current implementation requires that the seginfo
 *                       value passed to the function be an lvalue.  That is
 *                       because the address of the structure passed to this
 *                       function is placed in a register and then emitted in
 *                       the loadseg instruction as %a0.  That prevents the
 *                       compiler from using a scaled-indexed addressing mode,
 *                       which would potentially be incorrect, because the
 *                       scale is read from the MSR and not the instruction.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_loadseg_ss(loadstoreseg_param_t seginfo) {
  __asm__("loadseg %%ss:%a0" : : "r"(seginfo.base) : "memory");
}

/*
 * loadstoreseg_param_t _storeseg_ss(void)
 *
 * This intrinsic returns the segment information contained in the hardware
 * ss register.
 */
static __inline__ loadstoreseg_param_t __DEFAULT_FN_ATTRS
_storeseg_ss(void) {
  loadstoreseg_param_t _storeseg_result;
  __asm__ ("storeseg %%ss:%0" :
           "=m" (_storeseg_result) : :
           "memory");
  return _storeseg_result;
}

/*
 * void _set_ldtr(unsigned short sel)
 *
 * Loads the specified segment information into the local descriptor table
 * register (LDTR).
 *
 * Implementation note: The current implementation requires that the seginfo
 *                      value passed to the function be an lvalue.  That is
 *                      because we want to take its address to assign to the
 *                      temporary variable _seginfop.  That is strictly for
 *                      type checking purposes.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_set_ldtr(unsigned short sel) {
  __asm__ ("lldt %0" : : "r" (sel) : "memory");
}

/*
 * unsigned short _get_ldtr(void)
 *
 * Returns the segment information from the local descriptor table register
 * (LDTR).
 */
static __inline__ unsigned short __DEFAULT_FN_ATTRS _get_ldtr(void) {
  unsigned short _seginfo;
  __asm__ ("sldt %0" : "=m" (_seginfo) : : "memory");
  return _seginfo;
}

/*
 * void _set_tr(unsigned short sel)
 *
 * Loads the specified segment information into the task register.
 *
 * Implementation note: The current implementation requires that the seginfo
 *                      value passed to the function be an lvalue.  That is
 *                      because we want to take its address to assign to the
 *                      temporary variable _seginfop.  That is strictly for
 *                      type checking purposes.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_set_tr(unsigned short sel) {
  __asm__ ("ltr %0" : : "r" (sel) : "memory");
}

/*
 * unsigned short _get_tr(void)
 *
 * Returns the segment information from the task register.
 */
static __inline__ unsigned short __DEFAULT_FN_ATTRS  _get_tr(void) {
  unsigned short _seginfo;
  __asm__ ("str %0" : "=m" (_seginfo) : : "memory");
  return _seginfo;
}

/*
 * void _asidswitch_tlbflush(__int64 flush_type, void *p)
 *
 * This intrinsic issues the asidswitch_tlbflush instruction.  flush_type
 * indicates the desired type of flush, while p provides a linear address,
 * which is only relevant for some types of flushes.
 */
static __inline__ void __DEFAULT_FN_ATTRS
_asidswitch_tlbflush(__int64 flush_type, void *p) {
  __asm__("asidswitch_tlbflush %1, %0" : : "r"(flush_type), "r"(p) : "memory");
}

#endif // PPPE_INTRIN_INCLUDED
/* end INTEL_FEATURE_XUCC */

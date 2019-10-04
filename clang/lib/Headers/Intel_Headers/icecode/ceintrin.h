/*===---- ceintrin.h - IceCode intrinsics -----------------------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===------------------------------------------------------------------------===
 */

#ifndef __IMMINTRIN_H
#error "Never use <ceintrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __CEINTRIN_H
#define __CEINTRIN_H
#ifdef __x86_64__

typedef enum {
  _CE_SEGMENT_NONE = 0,
  _CE_SEGMENT_CS = 1,
  _CE_SEGMENT_DS = 2,
  _CE_SEGMENT_SS = 3,
  _CE_SEGMENT_ES = 4,
  _CE_SEGMENT_FS = 5,
  _CE_SEGMENT_GS = 6
} _CE_SEGMENT_ENUM;

/* Define the default attributes for the functions in this file. */
#define __DEFAULT_FN_ATTRS __attribute__((__always_inline__, __nodebug__, __target__("icecode-mode")))

/* Functions for IceCode ISA */
static __inline__ short __DEFAULT_FN_ATTRS
_ce_loadpphys16(void *mem) {
  return __builtin_ia32_icecode_loadpphys_16(mem);
}

static __inline__ int __DEFAULT_FN_ATTRS
_ce_loadpphys32(void *mem) {
  return __builtin_ia32_icecode_loadpphys_32(mem);
}

static __inline__ long long __DEFAULT_FN_ATTRS
_ce_loadpphys64(void *mem) {
  return __builtin_ia32_icecode_loadpphys_64(mem);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storepphys16(short reg, void *mem) {
  __builtin_ia32_icecode_storepphys_16(reg, mem);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storepphys32(int reg, void *mem) {
  __builtin_ia32_icecode_storepphys_32(reg, mem);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storepphys64(long long reg, void *mem) {
  __builtin_ia32_icecode_storepphys_64(reg, mem);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_cs(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_CS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_ds(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_DS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_ss(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_SS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_es(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_ES);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_fs(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_FS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_loadseg_gs(void *mem) {
  __builtin_ia32_icecode_loadseg(mem, _CE_SEGMENT_GS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_cs(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_CS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_ds(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_DS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_ss(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_SS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_es(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_ES);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_fs(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_FS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_storeseg_gs(void *mem) {
  __builtin_ia32_icecode_storeseg(mem, _CE_SEGMENT_GS);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_gtranslaterd_epc(void *mem, long long __info[4]) {
  __asm__ __volatile__ ("gtranslaterd_epc %a4" : "=a"(__info[0]), "=b"(__info[1]),
                                                 "=c"(__info[2]), "=d"(__info[3])
                                               : "r"(mem));
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_gtranslatewr_epc(void *mem, long long __info[4]) {
  __asm__ __volatile__ ("gtranslatewr_epc %a4" : "=a"(__info[0]), "=b"(__info[1]),
                                                 "=c"(__info[2]), "=d"(__info[3])
                                               : "r"(mem));
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_gtranslaterd_noepc(void *mem, long long __info[4]) {
  __asm__ __volatile__ ("gtranslaterd_noepc %a4" : "=a"(__info[0]), "=b"(__info[1]),
                                                   "=c"(__info[2]), "=d"(__info[3])
                                                 : "r"(mem));
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_gtranslatewr_noepc(void *mem, long long __info[4]) {
  __asm__ __volatile__ ("gtranslatewr_noepc %a4" : "=a"(__info[0]), "=b"(__info[1]),
                                                   "=c"(__info[2]), "=d"(__info[3])
                                                 : "r"(mem));
}

#define _ce_bcast_seg_state() __asm__ __volatile__ ("bcast_seg_state")

static __inline__ unsigned __DEFAULT_FN_ATTRS
_ce_creg_xchg32(unsigned creg, unsigned value) {
  return __builtin_ia32_icecode_creg_xchg_32(creg, value);
}

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_ce_creg_xchg64(unsigned creg, unsigned long long value) {
  return __builtin_ia32_icecode_creg_xchg_64(creg, value);
}

static __inline__ unsigned __DEFAULT_FN_ATTRS
_ce_fscp_xchg32(unsigned fscp, unsigned value) {
  return __builtin_ia32_icecode_fscp_xchg_32(fscp, value);
}

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_ce_fscp_xchg64(unsigned fscp, unsigned long long value) {
  return __builtin_ia32_icecode_fscp_xchg_64(fscp, value);
}

static __inline__ unsigned __DEFAULT_FN_ATTRS
_ce_creg_read32(unsigned creg) {
  return __builtin_ia32_icecode_creg_read_32(creg);
}

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_ce_creg_read64(unsigned creg) {
  return __builtin_ia32_icecode_creg_read_64(creg);
}

static __inline__ unsigned __DEFAULT_FN_ATTRS
_ce_fscp_read32(unsigned fscp) {
  return __builtin_ia32_icecode_fscp_read_32(fscp);
}

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_ce_fscp_read64(unsigned fscp) {
  return __builtin_ia32_icecode_fscp_read_64(fscp);
}

static __inline__ unsigned char __DEFAULT_FN_ATTRS
_ce_portin8(void *port) {
  return __builtin_ia32_icecode_portin_8(port);
}

static __inline__ unsigned short __DEFAULT_FN_ATTRS
_ce_portin16(void *port) {
  return __builtin_ia32_icecode_portin_16(port);
}

static __inline__ unsigned int __DEFAULT_FN_ATTRS
_ce_portin32(void *port) {
  return __builtin_ia32_icecode_portin_32(port);
}

static __inline__ unsigned long long __DEFAULT_FN_ATTRS
_ce_portin64(void *port) {
  return __builtin_ia32_icecode_portin_64(port);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_portout8(unsigned char value, void *port) {
  __builtin_ia32_icecode_portout_8(value, port);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_portout16(unsigned short value, void *port) {
  __builtin_ia32_icecode_portout_16(value, port);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_portout32(unsigned int value, void *port) {
  __builtin_ia32_icecode_portout_32(value, port);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_portout64(unsigned long long value, void *port) {
  __builtin_ia32_icecode_portout_64(value, port);
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_sta_special(void *mem) {
  __builtin_ia32_icecode_sta_special(mem);
}

#define _ce_nr_read(ireg) __builtin_ia32_icecode_nr_read(ireg)

static __inline__ void __DEFAULT_FN_ATTRS
_ce_ucodecall(unsigned target) {
  __builtin_ia32_icecode_ucodecall(target);
}

#define _ce_cmodemov(_SRC1, _SRC2, _COND) \
  __builtin_ia32_icecode_cmodemov((_SRC1), (_SRC2), (_COND))

#define _ce_sigeventjump(_SRC1, _SRC2, _IMM) \
  __asm__ __volatile__ ("sigeventjump $" #_IMM ", %0, %1" :: "q"(_SRC1), "a"(_SRC2))

#define _ce_sserialize() __asm__ __volatile__ ("sserialize")
#define _ce_nop_set_sb() __asm__ __volatile__ ("nop_set_sb")
#define _ce_nop_read_sb() __asm__ __volatile__ ("nop_read_sb")
#define _ce_get_excl_acc() __asm__ __volatile__ ("get_excl_acc")
#define _ce_release_excl_acc() __asm__ __volatile__ ("release_excl_acc")
#define _ce_virt_nuke_point() __asm__ __volatile__ ("virt_nuke_point")
#define _ce_int_trap_point() __asm__ __volatile__ ("int_trap_point")

static __inline__ void __DEFAULT_FN_ATTRS
_ce_iceret(unsigned target) {
  __asm__ __volatile__ ("iceret %0" :: "a"(target));
}

static __inline__ void __DEFAULT_FN_ATTRS
_ce_iceret_indirect(unsigned target, unsigned value) {
  __asm__ __volatile__ ("iceret_indirect %0, %1" :: "b"(value), "a"(target));
}

#undef __DEFAULT_FN_ATTRS

#endif /* __x86_64__ */
#endif /* __CEINTRIN_H */

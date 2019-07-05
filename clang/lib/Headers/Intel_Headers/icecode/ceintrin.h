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

#ifndef __CEINTRIN_H
#define __CEINTRIN_H

#ifndef __ICECODE__
#error "Must enable icecode before including ceintrin.h"
#endif

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

#undef __DEFAULT_FN_ATTRS

#endif /* __CEINTRIN_H */

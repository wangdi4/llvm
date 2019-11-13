/* INTEL_FEATURE_ISA_AMX2 */
/*===------------ Intel_amx2intrin.h - AMX2 intrinsics -*- C++ -*------------===
*
* Copyright (C) 2019 Intel Corporation. All rights reserved.
*
* The information and source code contained herein is the exclusive property
* of Intel Corporation and may not be disclosed, examined or reproduced in
* whole or in part without explicit written authorization from the company.
*
* ===---------------------------------------------------------------------=== */

#ifndef __IMMINTRIN_H
#error "Never use <Intel_amx2intrin.h> directly; include <immintrin.h> instead."
#endif /* __IMMINTRIN_H */

#ifndef __AMX2INTRIN_H
#define __AMX2INTRIN_H
#ifdef __x86_64__

// Transpose
#define _tile_2rpntlvw(tdst, base, stride, src)                                \
  __builtin_ia32_t2rpntlvw(tdst, base, stride, src)
#define _tile_2rpntlvwt1(tdst, base, stride, src)                              \
  __builtin_ia32_t2rpntlvwt1(tdst, base, stride, src)
#define _tile_2transposew(tdst, base, stride, src)                             \
  __builtin_ia32_t2transposew(tdst, base, stride, src)
#define _tile_2transposewt1(tdst, base, stride, src)                           \
  __builtin_ia32_t2transposewt1(tdst, base, stride, src)

// Reduce
#define _tile_coladdbcastps(tdst, tsrc)                                        \
  __builtin_ia32_tcoladdbcastps(tdst, tsrc)
#define _tile_coladdps(mem_dst, tsrc) __builtin_ia32_tcoladdps(mem_dst, tsrc)

// Memory
#define _tile_broadcastrowd(tdst, mem_src)                                     \
  __builtin_ia32_tbroadcastrowd(tdst, mem_src)
#define _tile_gatherrowd(tdst, base, index)                                    \
  __builtin_ia32_tgatherrowd(tdst, base, index)
#define _tile_gatherrowdt1(tdst, base, index)                                  \
  __builtin_ia32_tgatherrowdt1(tdst, base, index)
#define _tile_gatherrowq(tdst, base, index)                                    \
  __builtin_ia32_tgatherrowq(tdst, base, index)
#define _tile_gatherrowqt1(tdst, base, index)                                  \
  __builtin_ia32_tgatherrowqt1(tdst, base, index)
#define _tile_scatterrowd(base, index, tsrc)                                   \
  __builtin_ia32_tscatterrowd(base, index, tsrc)
#define _tile_scatterrowdt1(base, index, tsrc)                                 \
  __builtin_ia32_tscatterrowdt1(base, index, tsrc)
#define _tile_scatterrowq(base, index, tsrc)                                   \
  __builtin_ia32_tscatterrowq(base, index, tsrc)
#define _tile_scatterrowqt1(base, index, tsrc)                                 \
  __builtin_ia32_tscatterrowqt1(base, index, tsrc)
#define _tile_storehd(base, stride, tsrc)                                      \
  __builtin_ia32_tstorehd(base, stride, tsrc)
#define _tile_storehdt1(base, stride, tsrc)                                    \
  __builtin_ia32_tstorehdt1(base, stride, tsrc)
#define _tile_storentd(base, stride, tsrc)                                     \
  __builtin_ia32_tstorentd(base, stride, tsrc)
#define _tile_storeqd(base, stride, tsrc)                                      \
  __builtin_ia32_tstoreqd(base, stride, tsrc)
#define _tile_storeqdt1(base, stride, tsrc)                                    \
  __builtin_ia32_tstoreqdt1(base, stride, tsrc)
#define _tile_storerowd(mem_dst, tsrc) __builtin_ia32_tstorerowd(mem_dst, tsrc)

// Format
#define _tile_blendvd(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tblendvd(tdst, tsrc1, tsrc2)
#define _tile_interleaveeb(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_tinterleaveeb(tdst, tsrc1, tsrc2)
#define _tile_interleaveew(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_tinterleaveew(tdst, tsrc1, tsrc2)
#define _tile_interleaveob(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_tinterleaveob(tdst, tsrc1, tsrc2)
#define _tile_interleaveow(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_tinterleaveow(tdst, tsrc1, tsrc2)
#define _tile_narrowb(tdst, tsrc, imm) __builtin_ia32_tnarrowb(tdst, tsrc, imm)
#define _tile_narroww(tdst, tsrc, imm) __builtin_ia32_tnarroww(tdst, tsrc, imm)
#define _tile_permb_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tpermb_reg(tdst, tsrc1, tsrc2)
#define _tile_permb_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tpermb_mem(tdst, tsrc1, mem_src2)
#define _tile_permd_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tpermd_reg(tdst, tsrc1, tsrc2)
#define _tile_permd_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tpermd_mem(tdst, tsrc1, mem_src2)
#define _tile_permw_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tpermw_reg(tdst, tsrc1, tsrc2)
#define _tile_permw_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tpermw_mem(tdst, tsrc1, mem_src2)
#define _tile_widenb(tdst, tsrc, imm) __builtin_ia32_twidenb(tdst, tsrc, imm)
#define _tile_widenw(tdst, tsrc, imm) __builtin_ia32_twidenw(tdst, tsrc, imm)

// Element
#define _tile_addps_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_taddps_reg(tdst, tsrc1, tsrc2)
#define _tile_addps_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_taddps_mem(tdst, tsrc1, mem_src2)
#define _tile_andd_reg(tdst, tsrc1, tsrc2)                                     \
  __builtin_ia32_tandd_reg(tdst, tsrc1, tsrc2)
#define _tile_andd_mem(tdst, tsrc1, mem_src2)                                  \
  __builtin_ia32_tandd_mem(tdst, tsrc1, mem_src2)
#define _tile_andnd_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tandnd_reg(tdst, tsrc1, tsrc2)
#define _tile_andnd_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tandnd_mem(tdst, tsrc1, mem_src2)
#define _tile_cmpps_reg(tdst, tsrc1, tsrc2, imm)                               \
  __builtin_ia32_tcmpps_reg(tdst, tsrc1, tsrc2, imm)
#define _tile_cmpps_mem(tdst, tsrc1, mem_src2, imm)                            \
  __builtin_ia32_tcmpps_mem(tdst, tsrc1, mem_src2, imm)
#define _tile_cvtb2ps(tdst, tsrc) __builtin_ia32_tcvtb2ps(tdst, tsrc)
#define _tile_cvtbf162ps(tdst, tsrc) __builtin_ia32_tcvtbf162ps(tdst, tsrc)
#define _tile_cvtd2ps(tdst, tsrc) __builtin_ia32_tcvtd2ps(tdst, tsrc)
#define _tile_cvtps2bf16(tdst, tsrc) __builtin_ia32_tcvtps2bf16(tdst, tsrc)
#define _tile_cvtps2bs(tdst, tsrc) __builtin_ia32_tcvtps2bs(tdst, tsrc)
#define _tile_cvtps2ubs(tdst, tsrc) __builtin_ia32_tcvtps2ubs(tdst, tsrc)
#define _tile_cvtub2ps(tdst, tsrc) __builtin_ia32_tcvtub2ps(tdst, tsrc)
#define _tile_fmaddps_reg(tsrc1_dst, tsrc2, tsrc3)                             \
  __builtin_ia32_tfmaddps_reg(tsrc1_dst, tsrc2, tsrc3)
#define _tile_fmaddps_mem(tsrc1_dst, tsrc2, mem_src3)                          \
  __builtin_ia32_tfmaddps_mem(tsrc1_dst, tsrc2, mem_src3)
#define _tile_fmsubps_reg(tsrc1_dst, tsrc2, tsrc3)                             \
  __builtin_ia32_tfmsubps_reg(tsrc1_dst, tsrc2, tsrc3)
#define _tile_fmsubps_mem(tsrc1_dst, tsrc2, mem_src3)                          \
  __builtin_ia32_tfmsubps_mem(tsrc1_dst, tsrc2, mem_src3)
#define _tile_fnmaddps_reg(tsrc1_dst, tsrc2, tsrc3)                            \
  __builtin_ia32_tfnmaddps_reg(tsrc1_dst, tsrc2, tsrc3)
#define _tile_fnmaddps_mem(tsrc1_dst, tsrc2, mem_src3)                         \
  __builtin_ia32_tfnmaddps_mem(tsrc1_dst, tsrc2, mem_src3)
#define _tile_fnmsubps_reg(tsrc1_dst, tsrc2, tsrc3)                            \
  __builtin_ia32_tfnmsubps_reg(tsrc1_dst, tsrc2, tsrc3)
#define _tile_fnmsubps_mem(tsrc1_dst, tsrc2, mem_src3)                         \
  __builtin_ia32_tfnmsubps_mem(tsrc1_dst, tsrc2, mem_src3)
#define _tile_maxps_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tmaxps_reg(tdst, tsrc1, tsrc2)
#define _tile_maxps_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tmaxps_mem(tdst, tsrc1, mem_src2)
#define _tile_minps_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tminps_reg(tdst, tsrc1, tsrc2)
#define _tile_minps_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tminps_mem(tdst, tsrc1, mem_src2)
#define _tile_mulps_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tmulps_reg(tdst, tsrc1, tsrc2)
#define _tile_mulps_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tmulps_mem(tdst, tsrc1, mem_src2)
#define _tile_ord_reg(tdst, tsrc1, tsrc2)                                      \
  __builtin_ia32_tord_reg(tdst, tsrc1, tsrc2)
#define _tile_ord_mem(tdst, tsrc1, mem_src2)                                   \
  __builtin_ia32_tord_mem(tdst, tsrc1, mem_src2)
#define _tile_rcp14ps(tdst, tsrc) __builtin_ia32_trcp14ps(tdst, tsrc)
#define _tile_reduceps(tdst, tsrc, imm)                                        \
  __builtin_ia32_treduceps(tdst, tsrc, imm)
#define _tile_scalefps_reg(tdst, tsrc1, tsrc2)                                 \
  __builtin_ia32_tscalefps_reg(tdst, tsrc1, tsrc2)
#define _tile_scalefps_mem(tdst, tsrc1, mem_src2)                              \
  __builtin_ia32_tscalefps_mem(tdst, tsrc1, mem_src2)
#define _tile_slld(tdst, tsrc, imm) __builtin_ia32_tslld(tdst, tsrc, imm)
#define _tile_srld(tdst, tsrc, imm) __builtin_ia32_tsrld(tdst, tsrc, imm)
#define _tile_srlvd_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tsrlvd_reg(tdst, tsrc1, tsrc2)
#define _tile_srlvd_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tsrlvd_mem(tdst, tsrc1, mem_src2)
#define _tile_subps_reg(tdst, tsrc1, tsrc2)                                    \
  __builtin_ia32_tsubps_reg(tdst, tsrc1, tsrc2)
#define _tile_subps_mem(tdst, tsrc1, mem_src2)                                 \
  __builtin_ia32_tsubps_mem(tdst, tsrc1, mem_src2)
#define _tile_xord_reg(tdst, tsrc1, tsrc2)                                     \
  __builtin_ia32_txord_reg(tdst, tsrc1, tsrc2)
#define _tile_xord_mem(tdst, tsrc1, mem_src2)                                  \
  __builtin_ia32_txord_mem(tdst, tsrc1, mem_src2)

// FP16
#define _tile_dpfp16ps(tsrc1_dst, tsrc2, tsrc3)                                \
  __builtin_ia32_tdpfp16ps(tsrc1_dst, tsrc2, tsrc3)

// AMX2 AVX512
#define _tile_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6, tsrc7,\
  tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)        \
  __builtin_ia32_tile16move(tdst, tsrc1, tsrc2, tsrc3, tsrc4, tsrc5, tsrc6,    \
  tsrc7, tsrc8, tsrc9, tsrc10, tsrc11, tsrc12, tsrc13, tsrc14, tsrc15, tsrc16)

#define _tile_tilemovrowei(tsrc1, tsrc2)  __builtin_ia32_tilemovei(tsrc1, tsrc2)
#define _tile_tilemovrowee(tsrc1, tsrc2)  __builtin_ia32_tilemovee(tsrc1, tsrc2)
#define _tile_tilemovrowex(tsrc1, tsrc2)  __builtin_ia32_tilemovex(tsrc1, tsrc2)

// BF16EVEX
#define _tile_dpbf16pse(tsrc1_dst, tsrc2, tsrc3)                               \
  __builtin_ia32_tdpbf16pse(tsrc1_dst, tsrc2, tsrc3)

// INT8EVEX
#define _tile_dpbssde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbssde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbsude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbsude(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbusde(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbusde(tsrc1_dst, tsrc2, tsrc3)
#define _tile_dpbuude(tsrc1_dst, tsrc2, tsrc3)                                 \
  __builtin_ia32_tdpbuude(tsrc1_dst, tsrc2, tsrc3)

// TILEEVEX
#define _tile_loadde(dst, base, stride)  __builtin_ia32_tileloadde64((dst),    \
  ((const void *)(base)), (__SIZE_TYPE__)(stride))
#define _tile_stream_loadde(dst, base, stride)                                 \
  __builtin_ia32_tileloaddt1e64((dst), ((const void *)(base)),                 \
  (__SIZE_TYPE__)(stride))
#define _tile_storede(src, base, stride)                                       \
  __builtin_ia32_tilestorede64((src), ((void *)(base)),                        \
  (__SIZE_TYPE__)(stride))
#define _tile_tilemove(tdst, tsrc)  __builtin_ia32_tilemove(tdst, tsrc)
#define _tile_zeroe(tile)       __builtin_ia32_tilezeroe(tile)

#endif /* __x86_64__ */
#endif /* __AMX2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX2 */

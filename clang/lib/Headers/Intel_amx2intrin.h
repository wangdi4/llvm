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
#define _tile_2rpntlvw(src, base, index, scale, tile)                          \
  __asm__ volatile("t2rpntlvw %0, (%1,%2," #scale                              \
                   "), %%tmm" #tile ::"r"((unsigned long long)(src)),          \
                   "r"(base), "r"((unsigned long long)(index)))
#define _tile_2rpntlvwt1(src, base, index, scale, tile)                        \
  __asm__ volatile("t2rpntlvwt1 %0, (%1,%2," #scale                            \
                   "), %%tmm" #tile ::"r"((unsigned long long)(src)),          \
                   "r"(base), "r"((unsigned long long)(index)))
#define _tile_2transposew(src, base, index, scale, tile)                       \
  __asm__ volatile("t2transposew %0, (%1,%2," #scale                           \
                   "), %%tmm" #tile ::"r"((unsigned long long)(src)),          \
                   "r"(base), "r"((unsigned long long)(index)))
#define _tile_2transposewt1(src, base, index, scale, tile)                     \
  __asm__ volatile("t2transposewt1 %0, (%1,%2," #scale                         \
                   "), %%tmm" #tile ::"r"((unsigned long long)(src)),          \
                   "r"(base), "r"((unsigned long long)(index)))
// Reduce
#define _tile_coladdbcastps(tile1, tile2)                                      \
  __asm__ volatile("tcoladdbcastps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_coladdps(tile, mem)                                              \
  __asm__ volatile("tcoladdps %%tmm" #tile ", %0" ::"m"(mem))
// Memory
#define _tile_broadcastrowd(mem, tile)                                         \
  __asm__ volatile("tbroadcastrowd %0, %%tmm" #tile ::"m"(mem))
#define _tile_gatherrowd(base, index, tile)                                    \
  __asm__ volatile("tgatherrowd (%0,%1), %%tmm" #tile ::"r"(base), "r"(index))
#define _tile_gatherrowdt1(base, index, tile)                                  \
  __asm__ volatile("tgatherrowdt1 (%0,%1), %%tmm" #tile ::"r"(base), "r"(index))
#define _tile_gatherrowq(base, index, tile)                                    \
  __asm__ volatile("tgatherrowq (%0,%1), %%tmm" #tile ::"r"(base), "r"(index))
#define _tile_gatherrowqt1(base, index, tile)                                  \
  __asm__ volatile("tgatherrowqt1 (%0,%1), %%tmm" #tile ::"r"(base), "r"(index))
#define _tile_scatterrowd(tile, base, index)                                   \
  __asm__ volatile("tscatterrowd %%tmm" #tile ", (%0,%1)" ::"r"(base),         \
                   "r"(index))
#define _tile_scatterrowdt1(tile, base, index)                                 \
  __asm__ volatile("tscatterrowdt1 %%tmm" #tile ", (%0,%1)" ::"r"(base),       \
                   "r"(index))
#define _tile_scatterrowq(tile, base, index)                                   \
  __asm__ volatile("tscatterrowq %%tmm" #tile ", (%0,%1)" ::"r"(base),         \
                   "r"(index))
#define _tile_scatterrowqt1(tile, base, index)                                 \
  __asm__ volatile("tscatterrowqt1 %%tmm" #tile ", (%0,%1)" ::"r"(base),       \
                   "r"(index))
#define _tile_storehd(tile, base, index, scale)                                \
  __asm__ volatile("tstorehd %%tmm" #tile ", (%0,%1," #scale ")" ::"r"(base),  \
                   "r"((unsigned long long)(index)))
#define _tile_storehdt1(tile, base, index, scale)                              \
  __asm__ volatile("tstorehdt1 %%tmm" #tile ", (%0,%1," #scale                 \
                   ")" ::"r"(base),                                            \
                   "r"((unsigned long long)(index)))
#define _tile_storentd(tile, base, index, scale)                               \
  __asm__ volatile("tstorentd %%tmm" #tile ", (%0,%1," #scale ")" ::"r"(base), \
                   "r"((unsigned long long)(index)))
#define _tile_storeqd(tile, base, index, scale)                                \
  __asm__ volatile("tstoreqd %%tmm" #tile ", (%0,%1," #scale ")" ::"r"(base),  \
                   "r"((unsigned long long)(index)))
#define _tile_storeqdt1(tile, base, index, scale)                              \
  __asm__ volatile("tstoreqdt1 %%tmm" #tile ", (%0,%1," #scale                 \
                   ")" ::"r"(base),                                            \
                   "r"((unsigned long long)(index)))
#define _tile_storerowd(tile, mem)                                             \
  __asm__ volatile("tstorerowd %%tmm" #tile ", %0" ::"m"(mem))
// Format
#define _tile_blendvd(tile1, tile2, tile3)                                     \
  __asm__ volatile("tblendvd %%tmm" #tile1 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile3 ::)
#define _tile_interleaveeb(tile1, tile2, tile3)                                \
  __asm__ volatile("tinterleaveeb %%tmm" #tile1 ", %%tmm" #tile2 ", "          \
                   "%%tmm" #tile3 ::)
#define _tile_interleaveew(tile1, tile2, tile3)                                \
  __asm__ volatile("tinterleaveew %%tmm" #tile1 ", %%tmm" #tile2 ", "          \
                   "%%tmm" #tile3 ::)
#define _tile_interleaveob(tile1, tile2, tile3)                                \
  __asm__ volatile("tinterleaveob %%tmm" #tile1 ", %%tmm" #tile2 ", "          \
                   "%%tmm" #tile3 ::)
#define _tile_interleaveow(tile1, tile2, tile3)                                \
  __asm__ volatile("tinterleaveow %%tmm" #tile1 ", %%tmm" #tile2 ", "          \
                   "%%tmm" #tile3 ::)
#define _tile_narrowb(imm, tile1, tile2)                                       \
  __asm__ volatile("tnarrowb %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_narroww(imm, tile1, tile2)                                       \
  __asm__ volatile("tnarroww %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_permb_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tpermb %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_permb_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tpermb %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_permd_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tpermd %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_permd_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tpermd %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_permw_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tpermw %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_permw_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tpermw %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_widenb(imm, tile1, tile2)                                        \
  __asm__ volatile("twidenb %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_widenw(imm, tile1, tile2)                                        \
  __asm__ volatile("twidenw %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
// Element
#define _tile_addps_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("taddps %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_addps_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("taddps %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_andd_reg(tile1, tile2, tile3)                                    \
  __asm__ volatile("tandd %%tmm" #tile1 ", %%tmm" #tile2 ", "                  \
                   "%%tmm" #tile3 ::)
#define _tile_andd_mem(mem, tile2, tile3)                                      \
  __asm__ volatile("tandd %0, %%tmm" #tile2 ", "                               \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_andnd_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tandnd %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_andnd_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tandnd %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_cmpps_reg(imm, tile1, tile2, tile3)                              \
  __asm__ volatile("tcmpps %0, %%tmm" #tile1 ", %%tmm" #tile2 ", "             \
                   "%%tmm" #tile3 ::"i"(imm))
#define _tile_cmpps_mem(imm, mem, tile2, tile3)                                \
  __asm__ volatile("tcmpps %0, %1, %%tmm" #tile2 ", "                          \
                   "%%tmm" #tile3 ::"i"(imm),                                  \
                   "m"(mem))
#define _tile_cvtb2ps(tile1, tile2)                                            \
  __asm__ volatile("tcvtb2ps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtbf162ps(tile1, tile2)                                         \
  __asm__ volatile("tcvtbf162ps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtd2ps(tile1, tile2)                                            \
  __asm__ volatile("tcvtd2ps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtps2bf16(tile1, tile2)                                         \
  __asm__ volatile("tcvtps2bf16 %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtps2bs(tile1, tile2)                                           \
  __asm__ volatile("tcvtps2bs %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtps2ubs(tile1, tile2)                                          \
  __asm__ volatile("tcvtps2ubs %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_cvtub2ps(tile1, tile2)                                           \
  __asm__ volatile("tcvtub2ps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_fmaddps_reg(tile1, tile2, tile3)                                 \
  __asm__ volatile("tfmaddps %%tmm" #tile1 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile3 ::)
#define _tile_fmaddps_mem(mem, tile2, tile3)                                   \
  __asm__ volatile("tfmaddps %0, %%tmm" #tile2 ", "                            \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_fmsubps_reg(tile1, tile2, tile3)                                 \
  __asm__ volatile("tfmsubps %%tmm" #tile1 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile3 ::)
#define _tile_fmsubps_mem(mem, tile2, tile3)                                   \
  __asm__ volatile("tfmsubps %0, %%tmm" #tile2 ", "                            \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_fnmaddps_reg(tile1, tile2, tile3)                                \
  __asm__ volatile("tfnmaddps %%tmm" #tile1 ", %%tmm" #tile2 ", "              \
                   "%%tmm" #tile3 ::)
#define _tile_fnmaddps_mem(mem, tile2, tile3)                                  \
  __asm__ volatile("tfnmaddps %0, %%tmm" #tile2 ", "                           \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_fnmsubps_reg(tile1, tile2, tile3)                                \
  __asm__ volatile("tfnmsubps %%tmm" #tile1 ", %%tmm" #tile2 ", "              \
                   "%%tmm" #tile3 ::)
#define _tile_fnmsubps_mem(mem, tile2, tile3)                                  \
  __asm__ volatile("tfnmsubps %0, %%tmm" #tile2 ", "                           \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_maxps_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tmaxps %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_maxps_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tmaxps %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_minps_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tminps %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_minps_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tminps %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_mulps_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tmulps %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_mulps_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tmulps %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_ord_reg(tile1, tile2, tile3)                                     \
  __asm__ volatile("tord %%tmm" #tile1 ", %%tmm" #tile2 ", "                   \
                   "%%tmm" #tile3 ::)
#define _tile_ord_mem(mem, tile2, tile3)                                       \
  __asm__ volatile("tord %0, %%tmm" #tile2 ", "                                \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_rcp14ps(tile1, tile2)                                            \
  __asm__ volatile("trcp14ps %%tmm" #tile1 ", %%tmm" #tile2 ::)
#define _tile_reduceps(imm, tile1, tile2)                                      \
  __asm__ volatile("treduceps %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_scalefps_reg(tile1, tile2, tile3)                                \
  __asm__ volatile("tscalefps %%tmm" #tile1 ", %%tmm" #tile2 ", "              \
                   "%%tmm" #tile3 ::)
#define _tile_scalefps_mem(mem, tile2, tile3)                                  \
  __asm__ volatile("tscalefps %0, %%tmm" #tile2 ", "                           \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_slld(imm, tile1, tile2)                                          \
  __asm__ volatile("tslld %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_srld(imm, tile1, tile2)                                          \
  __asm__ volatile("tsrld %0, %%tmm" #tile1 ", %%tmm" #tile2 ::"i"(imm))
#define _tile_srlvd_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tsrlvd %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_srlvd_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tsrlvd %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_subps_reg(tile1, tile2, tile3)                                   \
  __asm__ volatile("tsubps %%tmm" #tile1 ", %%tmm" #tile2 ", "                 \
                   "%%tmm" #tile3 ::)
#define _tile_subps_mem(mem, tile2, tile3)                                     \
  __asm__ volatile("tsubps %0, %%tmm" #tile2 ", "                              \
                   "%%tmm" #tile3 ::"m"(mem))
#define _tile_xord_reg(tile1, tile2, tile3)                                    \
  __asm__ volatile("txord %%tmm" #tile1 ", %%tmm" #tile2 ", "                  \
                   "%%tmm" #tile3 ::)
#define _tile_xord_mem(mem, tile2, tile3)                                      \
  __asm__ volatile("txord %0, %%tmm" #tile2 ", "                               \
                   "%%tmm" #tile3 ::"m"(mem))
// FP16
#define _tile_dpfp16ps(tile1, tile2, tile3)                                    \
  __asm__ volatile("tdpfp16ps %%tmm" #tile1 ", %%tmm" #tile2 ", "              \
                   "%%tmm" #tile3 ::)
// Tile to AVX512
#define _tile_movrowe(tile, row)                                               \
  __extension__({                                                              \
    __m512 __zmm;                                                              \
    __asm__ volatile("tilemovrowe %1, %%tmm" #tile ", %0"                      \
                     : "=v"(__zmm)                                             \
                     : "ir"(row));                                             \
    __zmm;                                                                     \
  })
#define _tile_movrowe_x(tile, row)                                             \
  __extension__({                                                              \
    __m512 __zmm;                                                              \
    __asm__ volatile("tilemovrowe %1, %%tmm" #tile ", %0"                      \
                     : "=v"(__zmm)                                             \
                     : "v"(row));                                              \
    __zmm;                                                                     \
  })
#define _tile_move(tile1, tile2)                                               \
  __asm__ volatile("tilemove %%tmm" #tile2 ", %%tmm" #tile1 ::)
#define _tile_16move(tile, zmmx)                                               \
  __asm__ volatile("tile16move %%zmm" #zmmx ", %%tmm" #tile ::)
#define _tile_loadde(dst, base, stride)                                        \
  __asm__ volatile("tileloadde (%0,%1,1), %%tmm" #dst ::"r"(base),             \
                   "r"((unsigned long long)(stride)))
#define _tile_loaddt1e(dst, base, stride)                                      \
  __asm__ volatile("tileloaddt1e  (%0,%1,1), %%tmm" #dst ::"r"(base),          \
                   "r"((unsigned long long)(stride)))
#define _tile_storede(src, base, stride)                                       \
  __asm__ volatile("tilestorede  %%tmm" #src ", (%0,%1,1)" ::"r"(base),        \
                   "r"((unsigned long long)(stride)))
#define _tile_zeroe(src) __asm__ volatile("tilezeroe %%tmm" #src::)
#define _tile_dpbf16pse(tile1, tile2, tile3)                                   \
  __asm__ volatile("tdpbf16pse %%tmm" #tile3 ", %%tmm" #tile2 ", "             \
                   "%%tmm" #tile1 ::)
#define _tile_dpbssde(tile1, tile2, tile3)                                     \
  __asm__ volatile("tdpbssde %%tmm" #tile3 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile1 ::)
#define _tile_dpbsude(tile1, tile2, tile3)                                     \
  __asm__ volatile("tdpbsude %%tmm" #tile3 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile1 ::)
#define _tile_dpbusde(tile1, tile2, tile3)                                     \
  __asm__ volatile("tdpbusde %%tmm" #tile3 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile1 ::)
#define _tile_dpbuude(tile1, tile2, tile3)                                     \
  __asm__ volatile("tdpbuude %%tmm" #tile3 ", %%tmm" #tile2 ", "               \
                   "%%tmm" #tile1 ::)
#endif /* __x86_64__ */
#endif /* __AMX2INTRIN_H */
/* end INTEL_FEATURE_ISA_AMX2 */

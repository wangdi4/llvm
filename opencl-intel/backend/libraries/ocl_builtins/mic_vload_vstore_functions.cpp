// Copyright (c) 2006-2011 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  mic_vload_vstore_functions.cpp
///////////////////////////////////////////////////////////

#if defined (__MIC__) || defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#include <intrin.h>

#include "mic_cl_vloadvstore_declaration.h"

///
#define OCL_INTR_LD_iGn(func, rnd)   \
    OCL_INTR_LD_iCn(func, rnd)               \
    OCL_INTR_LD_iSn(func, rnd)               \
    OCL_INTR_LD_iIn(func, rnd)               \
    OCL_INTR_LD_iLn(func, rnd)               \

#define OCL_INTR_LD_uGn(func, rnd)           \
    OCL_INTR_LD_uCn(func, rnd)               \
    OCL_INTR_LD_uSn(func, rnd)               \
    OCL_INTR_LD_uIn(func, rnd)               \
    OCL_INTR_LD_uLn(func, rnd)               \

#define OCL_INTR_LD_fGn(func, rnd)           \
    OCL_INTR_LD_Fn(func, rnd)                \
    OCL_INTR_LD_Dn(func, rnd)                \

#define OCL_INTR_LDHALF_fGn(func, rnd)       \
    OCL_INTR_LDHALF_Fn(func, rnd)            \

#define OCL_INTR_LD_Gn(func, rnd)            \
    OCL_INTR_LD_iGn(func, rnd)               \
    OCL_INTR_LD_uGn(func, rnd)               \
    OCL_INTR_LD_fGn(func, rnd)               \

#define OCL_INTR_ST_iGn(func, rnd)           \
    OCL_INTR_ST_iCn(func, rnd)               \
    OCL_INTR_ST_iSn(func, rnd)               \
    OCL_INTR_ST_iIn(func, rnd)               \
    OCL_INTR_ST_iLn(func, rnd)               \

#define OCL_INTR_ST_uGn(func, rnd)           \
    OCL_INTR_ST_uCn(func, rnd)               \
    OCL_INTR_ST_uSn(func, rnd)               \
    OCL_INTR_ST_uIn(func, rnd)               \
    OCL_INTR_ST_uLn(func, rnd)               \

#define OCL_INTR_ST_fGn(func, rnd)           \
    OCL_INTR_ST_Fn(func, rnd)                \
    OCL_INTR_ST_Dn(func, rnd)                \

#define OCL_INTR_ST_Gn(func, rnd)            \
    OCL_INTR_ST_iGn(func, rnd)               \
    OCL_INTR_ST_uGn(func, rnd)               \
    OCL_INTR_ST_fGn(func, rnd)               \

#define OCL_INTR_STHALF_fGn(func, rnd)       \
    OCL_INTR_STHALF_Fn(func, rnd)            \
    OCL_INTR_STHALF_Dn(func, rnd)            \

/// OpenCL Sepc 1.2 (rev 15), Section 6.12.7, Table 6.15

OCL_INTR_LD_Gn      (vload,             )
OCL_INTR_ST_Gn      (vstore,            )
OCL_INTR_LDHALF_fGn (vload_half,        )
OCL_INTR_STHALF_fGn (vstore_half,       )
OCL_INTR_STHALF_fGn (vstore_half,   _rte)
OCL_INTR_STHALF_fGn (vstore_half,   _rtz)
OCL_INTR_STHALF_fGn (vstore_half,   _rtp)
OCL_INTR_STHALF_fGn (vstore_half,   _rtn)
OCL_INTR_LDHALF_fGn (vloada_half,       )
OCL_INTR_STHALF_fGn (vstorea_half,      )
OCL_INTR_STHALF_fGn (vstorea_half,  _rte)
OCL_INTR_STHALF_fGn (vstorea_half,  _rtz)
OCL_INTR_STHALF_fGn (vstorea_half,  _rtp)
OCL_INTR_STHALF_fGn (vstorea_half,  _rtn)

int16 __attribute__((overloadable)) vload16(size_t offset, const __private int *p)
{
    return mask_vload16(0xFFFF, offset, p);
}
int16 __attribute__((overloadable)) mask_vload16(ushort m16, size_t offset, const __private int *p)
{
    int16 r = _mm512_undefined_epi32();
    switch (m16) {
        case 0xFFFF:
            r = _mm512_loadunpackld(r, p + (offset * 16), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            r = _mm512_loadunpackhd(r, p + (offset * 16), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            break;
        case 0x00FF:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 8), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 8), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            break;
        case 0x000F:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 4), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 4), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            break;
        case 0x0007:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 3), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 3), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            break;
        case 0x0003:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 2), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 2), _MM_FULLUPC_NONE, _MM_HINT_NONE);
            break;
        case 0x0001:
            r = _mm512_loadd(p + offset, _MM_FULLUPC_NONE, _MM_BROADCAST_1X16, _MM_HINT_NONE);
            break;
    }
    return r;
}

uint16 __attribute__((overloadable)) vload16(size_t offset, const __private uint *p)
{
    return as_uint16(vload16(offset, (const int *)p));
}
uint16 __attribute__((overloadable)) mask_vload16(ushort m16, size_t offset, const __private uint *p)
{
    return as_uint16(mask_vload16(m16, offset, (const int *)p));
}

long8 __attribute__((overloadable)) vload8(size_t offset, const __private long *p)
{
    return mask_vload8(0xFF, offset, p);
}
long8 __attribute__((overloadable)) mask_vload8(uchar m8, size_t offset, const __private long *p)
{
    long8 r = _mm512_undefined_epi64();
    switch (m8) {
        case 0xFF:
            r = _mm512_loadunpacklq(r, p + (offset * 8), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            r = _mm512_loadunpackhq(r, p + (offset * 8), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            break;
        case 0x0F:
            r = _mm512_mask_loadunpacklq(r, m8, p + (offset * 4), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhq(r, m8, p + (offset * 4), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            break;
        case 0x07:
            r = _mm512_mask_loadunpacklq(r, m8, p + (offset * 3), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhq(r, m8, p + (offset * 3), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            break;
        case 0x03:
            r = _mm512_mask_loadunpacklq(r, m8, p + (offset * 2), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhq(r, m8, p + (offset * 2), _MM_FULLUPC64_NONE, _MM_HINT_NONE);
            break;
        case 0x01:
            r = _mm512_loadq(p + offset, _MM_FULLUPC64_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
            break;
    }
    return r;
}

ulong8 __attribute__((overloadable)) vload8(size_t offset, const __private ulong *p)
{
    return as_ulong8(vload8(offset, (const long *)p));
}
ulong8 __attribute__((overloadable)) mask_vload8(uchar m8, size_t offset, const __private ulong *p)
{
    return as_ulong8(mask_vload8(m8, offset, (const long *)p));
}

float16 __attribute__((overloadable)) vload16(size_t offset, const __private float *p)
{
    return as_float16(vload16(offset, (const int *)p));
}
float16 __attribute__((overloadable)) mask_vload16(ushort m16, size_t offset, const __private float *p)
{
    return as_float16(mask_vload16(m16, offset, (const int *)p));
}

double8 __attribute__((overloadable)) vload8(size_t offset, const __private double *p)
{
    return as_double8(vload8(offset, (const long *)p));
}
double8 __attribute__((overloadable)) mask_vload8(uchar m8, size_t offset, const __private double *p)
{
    return as_double8(mask_vload8(m8, offset, (const long *)p));
}

void __attribute__((overloadable)) vstore16(int16 x, size_t offset, __private int *p)
{
    mask_vstore16(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore16(ushort m16, int16 x, size_t offset, __private int *p)
{
    switch (m16) {
        case 0xFFFF:
            _mm512_packstoreld(p + (offset * 16), x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_packstorehd(p + (offset * 16), x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_packstoreld(p + (offset * 8), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 8), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x000F:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 4), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x0007:
            _mm512_mask_packstoreld(p + (offset * 3), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 3), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x0003:
            _mm512_mask_packstoreld(p + (offset * 2), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 2), m16, x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x0001:
            _mm512_stored(p + offset, x, _MM_DOWNC_NONE, _MM_SUBSET32_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstore16(uint16 x, size_t offset, __private uint *p)
{
    vstore16(as_int16(x), offset, (int *)p);
}
void __attribute__((overloadable)) mask_vstore16(ushort m16, uint16 x, size_t offset, __private uint *p)
{
    mask_vstore16(m16, as_int16(x), offset, (int *)p);
}

void __attribute__((overloadable)) vstore8(long8 x, size_t offset, __private long *p)
{
    mask_vstore8(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore8(uchar m8, long8 x, size_t offset, __private long *p)
{
    switch (m8) {
        case 0xFF:
            _mm512_packstoreld(p + (offset * 8), x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            _mm512_packstorehd(p + (offset * 8), x, _MM_DOWNC_NONE, _MM_HINT_NONE);
            break;
        case 0x0F:
            _mm512_mask_packstorelq(p + (offset * 4), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehq(p + (offset * 4), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            break;
        case 0x07:
            _mm512_mask_packstorelq(p + (offset * 3), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehq(p + (offset * 3), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            break;
        case 0x03:
            _mm512_mask_packstorelq(p + (offset * 2), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            _mm512_mask_packstorehq(p + (offset * 2), m8, x, _MM_DOWNC64_NONE, _MM_HINT_NONE);
            break;
        case 0x01:
            _mm512_storeq(p + offset, x, _MM_DOWNC64_NONE, _MM_SUBSET64_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstore8(ulong8 x, size_t offset, __private ulong *p)
{
    vstore8(as_long8(x), offset, (long *)p);
}
void __attribute__((overloadable)) mask_vstore8(uchar m8, ulong8 x, size_t offset, __private ulong *p)
{
    mask_vstore8(m8, as_long8(x), offset, (long *)p);
}

void __attribute__((overloadable)) vstore16(float16 x, size_t offset, __private float *p)
{
    vstore16(as_int16(x), offset, (int *)p);
}
void __attribute__((overloadable)) mask_vstore16(ushort m16, float16 x, size_t offset, __private float *p)
{
    mask_vstore16(m16, as_int16(x), offset, (int *)p);
}

void __attribute__((overloadable)) vstore8(double8 x, size_t offset, __private double *p)
{
    vstore8(as_long8(x), offset, (long *)p);
}
void __attribute__((overloadable)) mask_vstore8(uchar m8, double8 x, size_t offset, __private double *p)
{
    mask_vstore8(m8, as_long8(x), offset, (long *)p);
}

float16 __attribute__((overloadable)) vload_half16(size_t offset, const __private half *p)
{
    return mask_vload_half16(0xFFFF, offset, p);
}
float16 __attribute__((overloadable)) mask_vload_half16(ushort m16, size_t offset, const __private half *p)
{
    float16 r = _mm512_undefined_ps();
    switch (m16) {
        case 0xFFFF:
            r = _mm512_loadunpackld(r, p + (offset * 16), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            r = _mm512_loadunpackhd(r, p + (offset * 16), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 8), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 8), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x000F:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 4), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 4), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0007:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 3), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 3), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0003:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 2), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            r = _mm512_mask_loadunpackhd(r, m16, p + (offset * 2), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0001:
            r = _mm512_loadd(p + offset, _MM_FULLUPC_FLOAT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
            break;
    }
    return r;
}

void __attribute__((overloadable)) vstore_half16(float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half16(ushort m16, float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstore_half16_rte(float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16_rte(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half16_rte(ushort m16, float16 x, size_t offset, __private half *p)
{
    switch (m16) {
        case 0xFFFF:
            _mm512_packstoreld(p + (offset * 16), x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            _mm512_packstorehd(p + (offset * 16), x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_packstoreld(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x000F:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0007:
            _mm512_mask_packstoreld(p + (offset * 3), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 3), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0003:
            _mm512_mask_packstoreld(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0001:
            _mm512_stored(p + offset, x, _MM_DOWNC_FLOAT16, _MM_SUBSET32_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstore_half16_rtz(float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16_rtz(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half16_rtz(ushort m16, float16 x, size_t offset, __private half *p)
{
    switch (m16) {
        case 0xFFFF:
            _mm512_packstoreld(p + (offset * 16), x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            _mm512_packstorehd(p + (offset * 16), x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0xFF:
            _mm512_mask_packstoreld(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x0F:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x07:
            _mm512_mask_packstoreld(p + (offset * 3), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 3), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x03:
            _mm512_mask_packstoreld(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            _mm512_mask_packstorehd(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x01:
            _mm512_stored(p + offset, x, _MM_DOWNC_FLOAT16RZ, _MM_SUBSET32_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstore_half16_rtp(float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16_rtp(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half16_rtp(ushort m16, float16 x, size_t offset, __private half *p)
{
    // Ignore NaN and negative values and handle positive values only.
    // +infinity is fine as its mantissa is all 0s.
    ushort p16;
    p16 = _mm512_mask_cmpord_ps(m16, (float16)0, x);
    p16 = _mm512_mask_cmplt_ps(p16, (float16)0, x);
    // float: S E8 M23
    //  half: S E5 M10
    // g bit M12
    // r bit M11
    // s bit M10-0
    ushort g16, r16, s16;
    g16 = _mm512_mask_test_epi32(p16, x, (uint16)(1U << 12));
    r16 = _mm512_mask_test_epi32(p16, x, (uint16)(1U << 11));
    s16 = _mm512_mask_test_epi32(p16, x, (uint16)((1U << 11) - 1U));
    // Set g, r, s bits if any of them (round to positive infinity) is 1s (and
    // will be rounded in round-nearest-even.)
    x = _mm512_mask_or_epi32(x, g16 | r16 | s16, x, (uint16)(7U << 10));
    mask_vstore_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstore_half16_rtn(float16 x, size_t offset, __private half *p)
{
    mask_vstore_half16_rtn(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half16_rtn(ushort m16, float16 x, size_t offset, __private half *p)
{
    // Ignore NaN and positive values and handle negative values only.
    // -infinity is fine as its mantissa is all 0s.
    ushort n16;
    n16 = _mm512_mask_cmpord_ps(m16, x, (float16)0);
    n16 = _mm512_mask_cmplt_ps(n16, x, (float16)0);
    // float: S E8 M23
    //  half: S E5 M10
    // g bit M12
    // r bit M11
    // s bit M10-0
    ushort g16, r16, s16;
    g16 = _mm512_mask_test_epi32(n16, x, (uint16)(1U << 12));
    r16 = _mm512_mask_test_epi32(n16, x, (uint16)(1U << 11));
    s16 = _mm512_mask_test_epi32(n16, x, (uint16)((1U << 11) - 1U));
    // Set g, r, s bits if any of them (round to positive infinity) is 1s (and
    // will be rounded in round-nearest-even.)
    x = _mm512_mask_or_epi32(x, g16 | r16 | s16, x, (uint16)(7U << 10));
    mask_vstore_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstore_half8(double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half8(uchar m8, double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8_rte(m8, x, offset, p);
}

void __attribute__((overloadable)) vstore_half8_rte(double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8_rte(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half8_rte(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_NEAREST);
    mask_vstore_half16_rte((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstore_half8_rtz(double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8_rtz(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half8_rtz(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_TOWARD_ZERO);
    mask_vstore_half16_rtz((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstore_half8_rtp(double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8_rtp(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half8_rtp(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_UP);
    mask_vstore_half16_rtp((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstore_half8_rtn(double8 x, size_t offset, __private half *p)
{
    mask_vstore_half8_rtn(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstore_half8_rtn(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_DOWN);
    mask_vstore_half16_rtn((ushort)m8, r, offset, p);
}

float16 __attribute__((overloadable)) vloada_half16(size_t offset, const __private half *p)
{
    return mask_vloada_half16(0xFFFF, offset, p);
}
float16 __attribute__((overloadable)) mask_vloada_half16(ushort m16, size_t offset, const __private half *p)
{
    // NOTE: loadunpackhd is not necessary as (p+offset*m) must be aligned to
    // sizeof(half{n}), where m = 1, 2, 4, 4, 8, and 16 if n = 1, 2, 3, 4, 8, and 16.
    float16 r = _mm512_undefined_ps();
    switch (m16) {
        case 0xFFFF:
            r = _mm512_loadunpackld(r, p + (offset * 16), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 8), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x000F:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 4), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0007:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 4), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0003:
            r = _mm512_mask_loadunpackld(r, m16, p + (offset * 2), _MM_FULLUPC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0001:
            r = _mm512_loadd(p + offset, _MM_FULLUPC_FLOAT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
            break;
    }
    return r;
}

void __attribute__((overloadable)) vstorea_half16(float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half16(ushort m16, float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstorea_half16_rte(float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16_rte(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half16_rte(ushort m16, float16 x, size_t offset, __private half *p)
{
    // NOTE: packstorehd is not necessary as (p+offset*m) must be aligned to
    // sizeof(half{n}), where m = 1, 2, 4, 4, 8, and 16 if n = 1, 2, 3, 4, 8, and 16.
    switch (m16) {
        case 0xFFFF:
            _mm512_packstoreld(p + (offset * 16), x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_packstoreld(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x000F:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0007:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0003:
            _mm512_mask_packstoreld(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x0001:
            _mm512_stored(p + offset, x, _MM_DOWNC_FLOAT16, _MM_SUBSET32_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstorea_half16_rtz(float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16_rtz(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half16_rtz(ushort m16, float16 x, size_t offset, __private half *p)
{
    // NOTE: packstorehd is not necessary as (p+offset*m) must be aligned to
    // sizeof(half{n}), where m = 1, 2, 4, 4, 8, and 16 if n = 1, 2, 3, 4, 8, and 16.
    switch (m16) {
        case 0xFFFF:
            _mm512_packstoreld(p + (offset * 16), x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0xFF:
            _mm512_mask_packstoreld(p + (offset * 8), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x0F:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x07:
            _mm512_mask_packstoreld(p + (offset * 4), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x03:
            _mm512_mask_packstoreld(p + (offset * 2), m16, x, _MM_DOWNC_FLOAT16RZ, _MM_HINT_NONE);
            break;
        case 0x01:
            _mm512_stored(p + offset, x, _MM_DOWNC_FLOAT16RZ, _MM_SUBSET32_1, _MM_HINT_NONE);
            break;
    }
}

void __attribute__((overloadable)) vstorea_half16_rtp(float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16_rtp(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half16_rtp(ushort m16, float16 x, size_t offset, __private half *p)
{
    // Ignore NaN and negative values and handle positive values only.
    // +infinity is fine as its mantissa is all 0s.
    ushort p16;
    p16 = _mm512_mask_cmpord_ps(m16, (float16)0, x);
    p16 = _mm512_mask_cmplt_ps(p16, (float16)0, x);
    // float: S E8 M23
    //  half: S E5 M10
    // g bit M12
    // r bit M11
    // s bit M10-0
    ushort g16, r16, s16;
    g16 = _mm512_mask_test_epi32(p16, x, (uint16)(1U << 12));
    r16 = _mm512_mask_test_epi32(p16, x, (uint16)(1U << 11));
    s16 = _mm512_mask_test_epi32(p16, x, (uint16)((1U << 11) - 1U));
    // Set g, r, s bits if any of them (round to positive infinity) is 1s (and
    // will be rounded in round-nearest-even.)
    x = _mm512_mask_or_epi32(x, g16 | r16 | s16, x, (uint16)(7U << 10));
    mask_vstorea_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstorea_half16_rtn(float16 x, size_t offset, __private half *p)
{
    mask_vstorea_half16_rtn(0xFFFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half16_rtn(ushort m16, float16 x, size_t offset, __private half *p)
{
    // Ignore NaN and positive values and handle negative values only.
    // -infinity is fine as its mantissa is all 0s.
    ushort n16;
    n16 = _mm512_mask_cmpord_ps(m16, x, (float16)0);
    n16 = _mm512_mask_cmplt_ps(n16, x, (float16)0);
    // float: S E8 M23
    //  half: S E5 M10
    // g bit M12
    // r bit M11
    // s bit M10-0
    ushort g16, r16, s16;
    g16 = _mm512_mask_test_epi32(n16, x, (uint16)(1U << 12));
    r16 = _mm512_mask_test_epi32(n16, x, (uint16)(1U << 11));
    s16 = _mm512_mask_test_epi32(n16, x, (uint16)((1U << 11) - 1U));
    // Set g, r, s bits if any of them (round to positive infinity) is 1s (and
    // will be rounded in round-nearest-even.)
    x = _mm512_mask_or_epi32(x, g16 | r16 | s16, x, (uint16)(7U << 10));
    mask_vstorea_half16_rte(m16, x, offset, p);
}

void __attribute__((overloadable)) vstorea_half8(double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half8(uchar m8, double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8_rte(m8, x, offset, p);
}

void __attribute__((overloadable)) vstorea_half8_rte(double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8_rte(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half8_rte(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_NEAREST);
    mask_vstorea_half16_rte((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstorea_half8_rtz(double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8_rtz(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half8_rtz(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_TOWARD_ZERO);
    mask_vstorea_half16_rtz((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstorea_half8_rtp(double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8_rtp(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half8_rtp(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_UP);
    mask_vstorea_half16_rtp((ushort)m8, r, offset, p);
}

void __attribute__((overloadable)) vstorea_half8_rtn(double8 x, size_t offset, __private half *p)
{
    mask_vstorea_half8_rtn(0xFF, x, offset, p);
}
void __attribute__((overloadable)) mask_vstorea_half8_rtn(uchar m8, double8 x, size_t offset, __private half *p)
{
    float16 r = _mm512_mask_cvtl_pd2ps(x, m8, x, _MM_ROUND_MODE_DOWN);
    mask_vstorea_half16_rtn((ushort)m8, r, offset, p);
}

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)

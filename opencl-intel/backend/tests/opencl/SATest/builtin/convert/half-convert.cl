#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#define CONVERT_FP16(VF)                                                       \
  kernel void convert_fp16_v##VF(                                              \
      global uchar##VF *ucData, global ushort##VF *usData,                     \
      global uint##VF *uiData, global ulong##VF *ulData,                       \
      global char##VF *cData, global short##VF *sData, global int##VF *iData,  \
      global long##VF *lData, global float##VF *fData,                         \
      global double##VF *dData, global half##VF *hData) {                      \
    size_t gid = get_global_id(0);                                             \
    uchar##VF uc = ucData[gid];                                                \
    ushort##VF us = usData[gid];                                               \
    uint##VF ui = uiData[gid];                                                 \
    ulong##VF ul = ulData[gid];                                                \
    char##VF c = cData[gid];                                                   \
    short##VF s = sData[gid];                                                  \
    int##VF i = iData[gid];                                                    \
    long##VF l = lData[gid];                                                   \
    float##VF f = fData[gid];                                                  \
    double##VF d = dData[gid];                                                 \
    half##VF h = hData[gid];                                                   \
                                                                               \
    half##VF v =                                                               \
        convert_half##VF(uc) + convert_half##VF(us) + convert_half##VF(ui) +   \
        convert_half##VF(ul) + convert_half##VF(c) + convert_half##VF(s) +     \
        convert_half##VF(i) + convert_half##VF(l) + convert_half##VF(f) +      \
        convert_half##VF(h) + convert_half##VF(d) +                            \
                                                                               \
        convert_half##VF##_rte(uc) + convert_half##VF##_rte(us) +              \
        convert_half##VF##_rte(ui) + convert_half##VF##_rte(ul) +              \
        convert_half##VF##_rte(c) + convert_half##VF##_rte(s) +                \
        convert_half##VF##_rte(i) + convert_half##VF##_rte(l) +                \
        convert_half##VF##_rte(f) + convert_half##VF##_rte(h) +                \
        convert_half##VF##_rte(d) +                                            \
                                                                               \
        convert_half##VF##_rtp(uc) + convert_half##VF##_rtp(us) +              \
        convert_half##VF##_rtp(ui) + convert_half##VF##_rtp(ul) +              \
        convert_half##VF##_rtp(c) + convert_half##VF##_rtp(s) +                \
        convert_half##VF##_rtp(i) + convert_half##VF##_rtp(l) +                \
        convert_half##VF##_rtp(f) + convert_half##VF##_rtp(h) +                \
        convert_half##VF##_rtp(d) +                                            \
                                                                               \
        convert_half##VF##_rtn(uc) + convert_half##VF##_rtn(us) +              \
        convert_half##VF##_rtn(ui) + convert_half##VF##_rtn(ul) +              \
        convert_half##VF##_rtn(c) + convert_half##VF##_rtn(s) +                \
        convert_half##VF##_rtn(i) + convert_half##VF##_rtn(l) +                \
        convert_half##VF##_rtn(f) + convert_half##VF##_rtn(h) +                \
        convert_half##VF##_rtn(d) +                                            \
                                                                               \
        convert_half##VF##_rtz(uc) + convert_half##VF##_rtz(us) +              \
        convert_half##VF##_rtz(ui) + convert_half##VF##_rtz(ul) +              \
        convert_half##VF##_rtz(c) + convert_half##VF##_rtz(s) +                \
        convert_half##VF##_rtz(i) + convert_half##VF##_rtz(l) +                \
        convert_half##VF##_rtz(f) + convert_half##VF##_rtz(h) +                \
        convert_half##VF##_rtz(d);                                             \
                                                                               \
    ucData[gid] =                                                              \
        convert_uchar##VF(v) + convert_uchar##VF##_rte(v) +                    \
        convert_uchar##VF##_rtp(v) + convert_uchar##VF##_rtn(v) +              \
        convert_uchar##VF##_rtz(v) + convert_uchar##VF##_sat(v) +              \
        convert_uchar##VF##_sat_rte(v) + convert_uchar##VF##_sat_rtp(v) +      \
        convert_uchar##VF##_sat_rtn(v) + convert_uchar##VF##_sat_rtz(v);       \
                                                                               \
    usData[gid] =                                                              \
        convert_ushort##VF(v) + convert_ushort##VF##_rte(v) +                  \
        convert_ushort##VF##_rtp(v) + convert_ushort##VF##_rtn(v) +            \
        convert_ushort##VF##_rtz(v) + convert_ushort##VF##_sat(v) +            \
        convert_ushort##VF##_sat_rte(v) + convert_ushort##VF##_sat_rtp(v) +    \
        convert_ushort##VF##_sat_rtn(v) + convert_ushort##VF##_sat_rtz(v);     \
                                                                               \
    uiData[gid] =                                                              \
        convert_uint##VF(v) + convert_uint##VF##_rte(v) +                      \
        convert_uint##VF##_rtp(v) + convert_uint##VF##_rtn(v) +                \
        convert_uint##VF##_rtz(v) + convert_uint##VF##_sat(v) +                \
        convert_uint##VF##_sat_rte(v) + convert_uint##VF##_sat_rtp(v) +        \
        convert_uint##VF##_sat_rtn(v) + convert_uint##VF##_sat_rtz(v);         \
                                                                               \
    ulData[gid] =                                                              \
        convert_ulong##VF(v) + convert_ulong##VF##_rte(v) +                    \
        convert_ulong##VF##_rtp(v) + convert_ulong##VF##_rtn(v) +              \
        convert_ulong##VF##_rtz(v) + convert_ulong##VF##_sat(v) +              \
        convert_ulong##VF##_sat_rte(v) + convert_ulong##VF##_sat_rtp(v) +      \
        convert_ulong##VF##_sat_rtn(v) + convert_ulong##VF##_sat_rtz(v);       \
                                                                               \
    cData[gid] =                                                               \
        convert_char##VF(v) + convert_char##VF##_rte(v) +                      \
        convert_char##VF##_rtp(v) + convert_char##VF##_rtn(v) +                \
        convert_char##VF##_rtz(v) + convert_char##VF##_sat(v) +                \
        convert_char##VF##_sat_rte(v) + convert_char##VF##_sat_rtp(v) +        \
        convert_char##VF##_sat_rtn(v) + convert_char##VF##_sat_rtz(v);         \
                                                                               \
    sData[gid] =                                                               \
        convert_short##VF(v) + convert_short##VF##_rte(v) +                    \
        convert_short##VF##_rtp(v) + convert_short##VF##_rtn(v) +              \
        convert_short##VF##_rtz(v) + convert_short##VF##_sat(v) +              \
        convert_short##VF##_sat_rte(v) + convert_short##VF##_sat_rtp(v) +      \
        convert_short##VF##_sat_rtn(v) + convert_short##VF##_sat_rtz(v);       \
                                                                               \
    iData[gid] = convert_int##VF(v) + convert_int##VF##_rte(v) +               \
                 convert_int##VF##_rtp(v) + convert_int##VF##_rtn(v) +         \
                 convert_int##VF##_rtz(v) + convert_int##VF##_sat(v) +         \
                 convert_int##VF##_sat_rte(v) + convert_int##VF##_sat_rtp(v) + \
                 convert_int##VF##_sat_rtn(v) + convert_int##VF##_sat_rtz(v);  \
                                                                               \
    lData[gid] =                                                               \
        convert_long##VF(v) + convert_long##VF##_rte(v) +                      \
        convert_long##VF##_rtp(v) + convert_long##VF##_rtn(v) +                \
        convert_long##VF##_rtz(v) + convert_long##VF##_sat(v) +                \
        convert_long##VF##_sat_rte(v) + convert_long##VF##_sat_rtp(v) +        \
        convert_long##VF##_sat_rtn(v) + convert_long##VF##_sat_rtz(v);         \
                                                                               \
    fData[gid] = convert_float##VF(v) + convert_float##VF##_rte(v) +           \
                 convert_float##VF##_rtp(v) + convert_float##VF##_rtn(v) +     \
                 convert_float##VF##_rtz(v);                                   \
                                                                               \
    dData[gid] = convert_double##VF(v) + convert_double##VF##_rte(v) +         \
                 convert_double##VF##_rtp(v) + convert_double##VF##_rtn(v) +   \
                 convert_double##VF##_rtz(v);                                  \
  }

#ifndef VF16

CONVERT_FP16()
CONVERT_FP16(2)
CONVERT_FP16(3)
CONVERT_FP16(4)
CONVERT_FP16(8)

#else

CONVERT_FP16(16)

#endif

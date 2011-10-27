#ifndef __TARGET_HEADERS_H__
#define __TARGET_HEADERS_H__

namespace Intel {
// CPU enumeration
  enum ECPU{
    CPU_PENTIUM       = 0,
    CPU_FIRST         = CPU_PENTIUM,
    CPU_NOCONA        = 1,
    CPU_CORE2,
    CPU_PENRYN,
    CPU_COREI7,
    CPU_SANDYBRIDGE,
    CPU_HASWELL,
    MIC_KNIGHTSFERRY,  // MIC Cards must appear last
    CPU_LAST
  };
// CPU Features enumeration
  enum ECPUFeatureSupport{
    CFS_NONE    = 0x0000,
    CFS_SSE2    = 1,
    CFS_SSE3    = 1 << 1,
    CFS_SSSE3   = 1 << 2,
    CFS_SSE41   = 1 << 3,
    CFS_SSE42   = 1 << 4,
    CFS_AVX1	  = 1 << 5,
	  CFS_AVX2    = 1 << 6,
	  CFS_FMA     = 1 << 7
  };
}
#endif

// vim:ts=2:sw=2:et:

include "avx512f/avx512f.td"


// includes AVX-512 F, CD, BW, DQ, VL.
class AVX512 : AVX512F {
  include "avx512/common.td"
  include "avx512/sub_group_builtin_impl.td"
}


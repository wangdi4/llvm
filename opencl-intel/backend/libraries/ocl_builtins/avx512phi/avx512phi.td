// vim:ts=2:sw=2:et:

include "avx512f/avx512f.td"

// includes AVX-512 F, CD, ER, PF.
class AVX512Phi : AVX512F {
  include "avx512phi/math.td"
}


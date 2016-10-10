// vim:ts=2:sw=2:et:

include "avx2/AVX2.td"


class AVX512F : AVX2 {
  include "avx512f/math.td"
  include "avx512f/common.td"
  include "avx512f/conversion.td"
  include "avx512f/vloadstore.td"
  include "avx512f/integers.td"
  include "avx512f/relational.td"
  include "avx512f/masked_load_store.td"
  include "avx512f/transpose.td"
}


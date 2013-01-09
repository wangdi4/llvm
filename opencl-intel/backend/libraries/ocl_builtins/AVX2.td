// vim:ts=2:sw=2:et:

include "AVX.td"

class AVX2 : AVX {
  include "avx2/math.td"
  include "avx2/geometric.td"
  include "avx2/integers.td"
  include "avx2/misc.td"
}


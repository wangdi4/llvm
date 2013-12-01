// vim:ts=2:sw=2:et:

include "avx2/AVX2.td"


class KNL : AVX2 {

  include "KNLni/math.td"
  include "KNLni/common.td"
}


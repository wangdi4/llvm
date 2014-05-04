// vim:ts=2:sw=2:et:

include "avx2/AVX2.td"


class KNL : AVX2 {
  include "KNLni/math.td"
  include "KNLni/common.td"
  include "KNLni/conversion.td"
  include "KNLni/vloadstore.td"
  include "KNLni/integers.td"
  include "KNLni/relational.td"
  include "KNLni/masked_load_store.td"
  include "KNLni/transpose.td"
}


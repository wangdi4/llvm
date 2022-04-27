// vim:ts=2:sw=2:et:

include "avx512phi/avx512phi.td"
// knl - 64 bit
def target : AVX512Phi {
  let Target = "b3";
}

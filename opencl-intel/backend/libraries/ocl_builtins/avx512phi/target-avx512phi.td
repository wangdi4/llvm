// vim:ts=2:sw=2:et:

include "avx512phi/avx512phi.td"

def target : AVX512Phi {
  let Target = "xxx"; //to be injected from the command line
}

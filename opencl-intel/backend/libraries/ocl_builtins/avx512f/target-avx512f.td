// vim:ts=2:sw=2:et:

include "avx512f/avx512f.td"

def target : AVX512F {
  let Target = "xxx"; //to be injected from the command line
}

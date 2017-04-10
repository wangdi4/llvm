// vim:ts=2:sw=2:et:

include "avx512/avx512.td"

def target : AVX512 {
  let Target = "xxx"; //to be injected from the command line
}

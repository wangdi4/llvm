include "avx512f/avx512f.td"

defset list<OclBuiltinImpl> amx_impls = {
  include "amx/matrix_impl.td"
}

// includes AVX-512 F
class AMX : AVX512F {
    let Impls = amx_impls;
}

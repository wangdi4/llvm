// vim:ts=2:sw=2:et:

include "avx/AVX.td"

defset list<OclBuiltinImpl> avx2_impls = {
include "avx2/math_impl.td"
}

// Declare this helpers here due to they use ocl_char32 type with in define only for avx2 and higher
let Types = [v4i8],
    Attrs = [INTERNAL_INLINE],
    NeedForwardDecl = 1 in {
  def __ocl_load_transpose_char4x8_avx2_helper :
   OclBuiltin<"__ocl_load_transpose_char4x8_avx2_helper", (outs), (ins char32:$In, pv8gentype:$Out0, pv8gentype:$Out1, pv8gentype:$Out2, pv8gentype:$Out3),  [], [], 0, 0>, PRIVATE;
  def __ocl_transpose_store_char4x8_avx2_helper :
   OclBuiltin<"__ocl_transpose_store_char4x8_avx2_helper", (outs), (ins pchar32:$Out, v8gentype:$In0, v8gentype:$In1, v8gentype:$In2, v8gentype:$In3),  [], [], 0, 0>, PRIVATE;
}

class AVX2 : AVX {

  include "avx2/math.td"
  include "avx2/geometric.td"
  include "avx2/integers.td"
  include "avx2/misc.td"
  include "avx2/vloadstore.td"
  include "avx2/transpose.td"
  include "avx2/masked_load_store.td"
  include "avx2/sub_group_builtin_impl.td"

  let Impls = avx2_impls;
}

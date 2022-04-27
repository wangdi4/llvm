// vim:ts=2:sw=2:et:

include "avx512f/avx512f.td"

defset list<OclBuiltinImpl> avx512_impls = {

// intel_sub_group_ballot
foreach vf = [8, 16] in {
  defvar builtin = !cast<OclBuiltin>("intel_sub_group_ballot_vf" # vf);
  defvar shuffle_sequence = !foldl([]<int>, Range<0, vf>.Tout, acc, i, acc # [i, vf, vf, vf]);
  def builtin # _avx512_impl : OclBuiltinImpl<builtin, [!cast<OclType>("v" # vf # "i32")], 0, [{
    // This implementation only applicable as long as CPU supports sub group size <= 32,
    // anything above that would have to extend to the second element of uint4 return vector.
    __mmask$VF mask = _mm$WIDTH_cmp_epi32_mask($Arg1VarName, _mm$WIDTH_setzero_ps(), _MM_CMPINT_NE);
    uint$VF res = (uint$VF)(uint)_mm$WIDTH_mask_cmp_epi32_mask(mask, $Arg0VarName, _mm$WIDTH_setzero_ps(), _MM_CMPINT_NE);
    uint$VF zero = (uint$VF) 0;
    return __builtin_shufflevector(res, zero, $SEQUENCE);
  }], [["$VF", !cast<string>(vf)], ["$WIDTH", !cast<string>(!mul(vf, 32))], ["$SEQUENCE", !interleave(shuffle_sequence, ",")]]>;
}

} // defset avx512_impls

// includes AVX-512 F, CD, BW, DQ, VL.
class AVX512 : AVX512F {
  include "avx512/common.td"
  include "avx512/sub_group_builtin_impl.td"

  let Impls = avx512_impls;
}


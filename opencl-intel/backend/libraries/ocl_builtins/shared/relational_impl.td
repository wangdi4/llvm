foreach name = ["bitselect"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar float_types = ExpandTypesByVFAndFlatten<[v1f32, v1f64], [2, 4, 8, 16]>.Tout;
  defvar float_code = bitselect_f_code;
  def name # _float_impl: OclBuiltinImpl<builtin, float_types, 0, float_code>;

  defvar integer_types = ExpandTypesByVFAndFlatten<
    [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [2, 4, 8, 16]>.Tout;
  defvar integer_code = bitselect_i_code;
  def name # _integer_impl: OclBuiltinImpl<builtin, integer_types, 0, integer_code>;
}

// Expand
foreach name = [
    "cos_rm", "divide_rm", "exp_rm", "exp2_rm", "exp10_rm",
    "log_rm", "log2_rm", "pow_rm", "sin_rm", "tan_rm",
    "sincos_pas_rm", "sincos_las_rm", "sincos_gas_rm", "sincos_gen_rm"] in {
  defvar builtin = !cast<OclBuiltin>(name);
  defvar types = [v32f32, v64f32];
  defvar num_args = !size(builtin.Ins);
  defvar expand_code = !cond(
      !ne(!find(name, "sincos_"), -1): Expand_2Ptr,
      !eq(num_args, 1): Expand1,
      !eq(num_args, 2): Expand2);
  def name # _expand_v32v64 : OclBuiltinImpl<builtin, types, 0, expand_code>;
}

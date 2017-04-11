foreach vec = ["v2", "v4", "v8", "v16"] in {
foreach typ = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
let Types = [!cast<OclType>(#vec#typ)] in {
  let NeedForwardDecl = 1 in {
  def read_pipe_2_intel_#vec#typ       : OclBuiltin<"__read_pipe_2_intel_"#vec#typ,       (outs gentype:$ret),   (ins global_struct_pipe_ptr:$p, pv1i32:$status), [], [], 1, 0>;
  def read_pipe_2_bl_intel_#vec#typ    : OclBuiltin<"__read_pipe_2_bl_intel_"#vec#typ,    (outs gentype:$ret),   (ins global_struct_pipe_ptr:$p), [], [], 1, 0>;
  def write_pipe_2_intel_#vec#typ      : OclBuiltin<"__write_pipe_2_intel_"#vec#typ,      (outs v1i32:$status),  (ins global_struct_pipe_ptr:$p, gentype:$src), [], [], 1, 0>;
  def write_pipe_2_bl_intel_#vec#typ   : OclBuiltin<"__write_pipe_2_bl_intel_"#vec#typ,   (outs v1i32:$status),  (ins global_struct_pipe_ptr:$p, gentype:$src), [], [], 1, 0>;

}}}}

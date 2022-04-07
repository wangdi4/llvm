foreach vec = ["v2", "v4", "v8", "v16"] in {
foreach typ = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {

let Types = [!cast<OclType>(vec#typ)],
    NeedForwardDecl = 1 in {
      def read_pipe_2_fpga_#vec#typ       : OclBuiltin<"__read_pipe_2_fpga_"#vec#typ,       (outs gentype:$ret),   (ins read_only_pipe:$pp, pv1i32:$status), [], [], 1, 0>;
      def write_pipe_2_fpga_#vec#typ      : OclBuiltin<"__write_pipe_2_fpga_"#vec#typ,      (outs v1i32:$status),  (ins write_only_pipe:$pp, gentype:$src), [], [], 1, 0>;
}

}}

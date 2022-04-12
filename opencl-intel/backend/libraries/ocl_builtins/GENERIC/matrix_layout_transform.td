let Types = [v1i8, v1i16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
  def matrix_layout_transform_rowmajor_to_vnni : OclBuiltin<"matrix_layout_transform_rowmajor_to_vnni", (outs), (ins pgentype:$_in, pgentype:$_out, i32gentype:$K, i32gentype:$N, i32gentype:$stride),[],[]>;
}

let Types = [v1i8, v1i16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
  def matrix_layout_transform_colmajor_to_rowmajor : OclBuiltin<"matrix_layout_transform_colmajor_to_rowmajor", (outs), (ins pgentype:$_in, pgentype:$_out, i32gentype:$K, i32gentype:$N, i32gentype:$stride),[],[]>;
}

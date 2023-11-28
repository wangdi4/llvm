// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

let Types = [v1i8, v1i16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
  def matrix_layout_transform_rowmajor_to_vnni : OclBuiltin<"matrix_layout_transform_rowmajor_to_vnni", (outs), (ins pgentype:$_in, pgentype:$_out, i32gentype:$K, i32gentype:$N, i32gentype:$stride),[],[]>;
}

let Types = [v1i8, v1i16, v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
  def matrix_layout_transform_colmajor_to_rowmajor : OclBuiltin<"matrix_layout_transform_colmajor_to_rowmajor", (outs), (ins pgentype:$_in, pgentype:$_out, i32gentype:$K, i32gentype:$N, i32gentype:$stride),[],[]>;
}

let Types = [v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = true in {
  def matrix_layout_transform_rowmajor_to_colmajor : OclBuiltin<"matrix_layout_transform_rowmajor_to_colmajor", (outs), (ins pgentype:$_in, pgentype:$_out, i32gentype:$K, i32gentype:$N, i32gentype:$stride),[],[]>;
}

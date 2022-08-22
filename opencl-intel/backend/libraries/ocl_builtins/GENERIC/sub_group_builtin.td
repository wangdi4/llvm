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

defvar khr_shuffle_gentype = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

defvar intel_shuffle_gentype = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64, v1i8, v1u8, v1i16, v1u16,
                                v2i32, v4i32, v8i32, v16i32, v3i32,
                                v2u32, v4u32, v8u32, v16u32, v3u32,
                                v2f32, v4f32, v8f32, v16f32, v3f32,
                                v2i64, v4i64, v8i64, v16i64, v3i64,
                                v2u64, v4u64, v8u64, v16u64, v3u64,
                                v2i8 , v4i8 , v8i8 , v16i8 , v3i8 ,
                                v2u8 , v4u8 , v8u8 , v16u8 , v3u8 ,
                                v2i16, v4i16, v8i16, v16i16, v3i16,
                                v2u16, v4u16, v8u16, v16u16, v3u16];

defvar broadcast_extended_types = [v2i8, v2u8, v2i16, v2u16, v2i32, v2u32, v2i64, v2u64, v2f32, v2f64,
                                   v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32, v3f64,
                                   v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64, v4f32, v4f64,
                                   v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64, v8f32, v8f64,
                                   v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64];

let VectorAttrs = [KERNEL_CALL_ONCE] in {

//
// Ballot (NEO-like)
//
// NOT overloadable
let NeedForwardDecl = 1 in {
  let Types = [ v1i32 ] in {
    def intel_sub_group_ballot_igc : OclBuiltin<"intel_sub_group_ballot", (outs ugentype:$ret), (ins bool:$pred), [], [], 1, 0>;
  }
  let Types = [ v4i32 ]  in {
    def intel_sub_group_ballot_igc_vf4 : OclBuiltin<"intel_sub_group_ballot_vf4", (outs ugentype:$ret), (ins bool:$pred), [], [], 1, 0>;
  }
  let Types = [ v8i32 ]  in {
    def intel_sub_group_ballot_igc_vf8 : OclBuiltin<"intel_sub_group_ballot_vf8", (outs ugentype:$ret), (ins bool:$pred), [], [], 1, 0>;
  }
  let Types = [ v16i32 ] in {
    def intel_sub_group_ballot_igc_vf16 : OclBuiltin<"intel_sub_group_ballot_vf16", (outs ugentype:$ret), (ins bool:$pred), [], [], 1, 0>;
  }
}


//
// Sub group functions and their vectorized versions
//
let NeedForwardDecl = 1,
Attrs = [OVERLOADABLE] in {
  let Types = [ v1i32 ] in {
    def sub_group_all  : OclBuiltin<"sub_group_all",  (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
    def sub_group_any  : OclBuiltin<"sub_group_any",  (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
  }

  let Types = [ v4i32,  v8i32,  v16i32, v32i32, v64i32] in {
    def sub_group_all_vec  : OclBuiltin<"sub_group_all",  (outs gentype:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
    def sub_group_any_vec  : OclBuiltin<"sub_group_any",  (outs gentype:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
  }

  let Types = [ v1i32, v1u32, v1i64, v1u64, v1f32, v1f64 ] in {
    def sub_group_broadcast  : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id), [], [], 1, 0>;
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["add", "min", "max"] in {
        def sub_group_#func#_#op : OclBuiltin<"sub_group_"#func#_#op, (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
      }
    }
  }

  let Types = [v1i8, v1u8, v1i16, v1u16] in {
    def sub_group_broadcast_cs : OclBuiltin<"intel_sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id), [], [], 1, 0>;
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["add", "min", "max"] in {
        def sub_group_#func#_#op#_cs : OclBuiltin<"intel_sub_group_"#func#_#op, (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
      }
    }
  }

  let Types = broadcast_extended_types in {
    def sub_group_broadcast_extended : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_extended : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids), [], [], 1, 0>;
  }

  let Types = VFType<broadcast_extended_types, "_avx">.Tout in {
    def sub_group_broadcast_extended_avx : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, v4u32:$vec_mask), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_extended_avx : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids, v4u32:$vec_mask), [], [], 1, 0>;
  }

  let Types = VFType<broadcast_extended_types, "_avx2">.Tout in {
    def sub_group_broadcast_extended_avx2 : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, v8u32:$vec_mask), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_extended_avx2 : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids, v8u32:$vec_mask), [], [], 1, 0>;
  }

  let Types = VFType<broadcast_extended_types, "_avx512">.Tout in {
    def sub_group_broadcast_extended_avx512 : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, v16u32:$vec_mask), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_extended_avx512 : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids, v16u32:$vec_mask), [], [], 1, 0>;
  }

  let Types = [ v4i32, v8i32, v16i32, v32i32, v64i32,
                v4u32, v8u32, v16u32, v32u32, v64u32,
                v4i64, v8i64, v16i64, v32i64, v64i64,
                v4u64, v8u64, v16u64, v32u64, v64u64,
                v4f32, v8f32, v16f32, v32f32, v64f32,
                v4f64, v8f64, v16f64, v32f64, v64f64 ] in {
    def sub_group_broadcast_vec : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, u32gentype:$vec_mask), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_vec : OclBuiltin<"sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids, u32gentype:$vec_mask), [], [], 1, 0>;
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["add", "min", "max"] in {
        def sub_group_#func#_#op#_vec : OclBuiltin<"sub_group_"#func#_#op, (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
      }
    }
  }

  let Types = [v4i8 , v4u8 , v4i16 , v4u16 ,
               v8i8 , v8u8 , v8i16 , v8u16 ,
               v16i8, v16u8, v16i16, v16u16,
               v32i8, v32u8, v32i16, v32u16,
               v64i8, v64u8, v64i16, v64u16] in {
    def sub_group_broadcast_vec_cs : OclBuiltin<"intel_sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, u32gentype:$vec_mask), [], [], 1, 0>;
    def sub_group_broadcast_assume_uniform_vec_cs : OclBuiltin<"intel_sub_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u32gentype:$sub_group_local_ids, u32gentype:$vec_mask), [], [], 1, 0>;
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["add", "min", "max"] in {
        def sub_group_#func#_#op#_vec_cs : OclBuiltin<"intel_sub_group_"#func#_#op, (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
      }
    }
  }

  let Types = [v1i32] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["and", "or", "xor"] in {
      def sub_group_non_uniform_#func#_logical_#op : OclBuiltin<"sub_group_non_uniform_"#func#"_logical_"#op, (outs gentype:$ret), (ins gentype:$wsrc), [], [], 1, 0>;
      }
    }
  }

  let Types = [v4i32, v8i32, v16i32] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["and", "or", "xor"] in {
      def sub_group_non_uniform_#func#_logical_#op#_vec : OclBuiltin<"sub_group_non_uniform_"#func#"_logical_"#op, (outs gentype:$ret), (ins gentype:$wsrc, u32gentype:$vec_mask), [], [], 1, 0>;
      }
    }
  }

  let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["and", "or", "xor"] in {
      def sub_group_non_uniform_#func#_#op : OclBuiltin<"sub_group_non_uniform_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$wsrc), [], [], 1, 0>;
      }
    }
  }

  let Types = [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64,
               v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64,
               v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      foreach op = ["and", "or", "xor"] in {
      def sub_group_non_uniform_#func#_#op#_vec : OclBuiltin<"sub_group_non_uniform_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$wsrc, u32gentype:$vec_mask), [], [], 1, 0>;
      }
    }
  }

  let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32,
               v1f32, v1f64, v1i64, v1u64] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      def sub_group_non_uniform_#func#_#mul : OclBuiltin<"sub_group_non_uniform_"#func#"_mul", (outs gentype:$ret), (ins gentype:$wsrc), [], [], 1, 0>;
    }
  }

  let Types = [ v4i8 , v8i8 , v16i8 ,
                v4u8 , v8u8 , v16u8 ,
                v4i16, v8i16, v16i16,
                v4u16, v8u16, v16u16,
                v4i32, v8i32, v16i32,
                v4u32, v8u32, v16u32,
                v4i64, v8i64, v16i64,
                v4u64, v8u64, v16u64,
                v4f32, v8f32, v16f32,
                v4f64, v8f64, v16f64 ] in {
    foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
      def sub_group_non_uniform_#func#_#mul_vec : OclBuiltin<"sub_group_non_uniform_"#func#"_mul", (outs gentype:$ret), (ins gentype:$wsrc, u32gentype:$vec_mask), [], [], 1, 0>;
    }
  }

  // Scalar ballot_next
  let Types = [v1i32] in {
    def intel_sub_group_ballot : OclBuiltin<"intel_sub_group_ballot", (outs v4u32:$ret), (ins gentype:$predicate), [], [], 1, 0>;
  }

  let Types = [v4i32] in {
    def intel_sub_group_ballot_vf4 : OclBuiltin<"intel_sub_group_ballot", (outs v16u32:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
  }

  let Types = [v8i32] in {
    def intel_sub_group_ballot_vf8 : OclBuiltin<"intel_sub_group_ballot", (outs v32u32:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
  }

  let Types = [v16i32] in {
    def intel_sub_group_ballot_vf16 : OclBuiltin<"intel_sub_group_ballot", (outs v64u32:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
  }

  // Declare "scalar" (not vectorized) versions of shuffle.
  let Types = intel_shuffle_gentype in {
        def sub_group_shuffle :     OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v1u32: $sub_group_local_id), [], [], 1, 0>;
        def sub_group_shuffle_xor : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v1u32: $value), [], [], 1, 0>;
        def sub_group_shuffle_down : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v1u32:$value), [], [], 1, 0>;
        def sub_group_shuffle_up : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v1u32:$value), [], [], 1, 0>;
      }

  def khr_sub_group_shuffle_up : OclBuiltin<"sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, v1u32:$delta), khr_shuffle_gentype, [], 1, 0>;
  def khr_sub_group_shuffle_down : OclBuiltin<"sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, v1u32:$delta), khr_shuffle_gentype, [], 1, 0>;
  foreach suffix = ["_avx", "_avx2", "_avx512", "_x32", "_x64"] in {
      defvar types = VFType<khr_shuffle_gentype, suffix>.Tout;
      defvar delta_mask = !head(VFType<[v1u32], suffix>.Tout);
      def khr_sub_group_shuffle_up#suffix : OclBuiltin<"sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$data, delta_mask:$delta, delta_mask:$vec_mask), types, [], 1, 0>;
      def khr_sub_group_shuffle_down#suffix : OclBuiltin<"sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$data, delta_mask:$delta, delta_mask:$vec_mask), types, [], 1, 0>;
  }

  // AVX (VF = 4)
  let Types = VFType<intel_shuffle_gentype, "_avx">.Tout in {
        def sub_group_shuffle_avx : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v4u32:$sub_group_local_id, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
      }

  // AVX2 (VF = 8)
  let Types = VFType<intel_shuffle_gentype, "_avx2">.Tout in {
        def sub_group_shuffle_avx2 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v8u32:$sub_group_local_id, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx2 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx2 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx2 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
      }

  // AVX512 (VF = 16)
  let Types = VFType<intel_shuffle_gentype, "_avx512">.Tout in {
        def sub_group_shuffle_avx512 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v16u32:$sub_group_local_id, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx512 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx512 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx512 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
      }

  // Rename sub_group_shuffle_avx/avx2/avx512 to sub_group_shuffle_x4/8/16
  // X32 shuffle
  let Types = VFType<intel_shuffle_gentype, "_x32">.Tout in {
        def sub_group_shuffle_x32 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v32u32:$sub_group_local_id, v32u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_x32 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v32u32:$value, v32u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_x32 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v32u32:$value, v32u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_x32 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v32u32:$value, v32u32:$vec_mask), [], [], 1, 0>;
  }

  // X64 shuffle
  let Types = VFType<intel_shuffle_gentype, "_x64">.Tout in {
        def sub_group_shuffle_x64 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v64u32:$sub_group_local_id, v64u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_x64 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v64u32:$value, v64u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_x64 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v64u32:$value, v64u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_x64 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v64u32:$value, v64u32:$vec_mask), [], [], 1, 0>;
  }

  // Sub group block read & write
  // uint
  // TBD: "ui" suffixed aliases for consitensy (cl_intel_subgroups_short)
  let Types = [ v1u32 ] in {
        // scalar
        def intel_sub_group_block_read  : OclBuiltin<"intel_sub_group_block_read",  (outs gentype:$ret), (ins cgpv1u32:$p), [], [], 1, 0>;
        def intel_sub_group_block_write : OclBuiltin<"intel_sub_group_block_write", (outs void:$ret), (ins gpv1u32:$p, gentype:$data), [], [], 1, 0>;
        def intel_sub_group_block_read_img  : OclBuiltin<"intel_sub_group_block_read",  (outs gentype:$ret), (ins image2d_ro:$img, v2i32:$coord), [], [], 1, 0>;
        def intel_sub_group_block_write_img : OclBuiltin<"intel_sub_group_block_write", (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, gentype:$data), [], [], 1, 0>;
        def intel_sub_group_block_read_img_rw  : OclBuiltin<"intel_sub_group_block_read",  (outs gentype:$ret), (ins image2d_rw:$img_rw, v2i32:$coord), [], [], 1, 0>;
        def intel_sub_group_block_write_img_rw : OclBuiltin<"intel_sub_group_block_write", (outs void:$ret), (ins image2d_rw:$img_rw, v2i32:$coord, gentype:$data), [], [], 1, 0>;
        foreach len = [ 2, 4, 8 ] in {
          def intel_sub_group_block_read#!cast<string>(len) : OclBuiltin<"intel_sub_group_block_read"#len,  (outs !cast<OclType>("v"#len#"u32"):$ret), (ins cgpv1u32:$p), [], [], 1, 0>;
          def intel_sub_group_block_write#!cast<string>(len) : OclBuiltin<"intel_sub_group_block_write"#len, (outs void:$ret), (ins gpv1u32:$p, !cast<OclType>("v"#len#"u32"):$data), [], [], 1, 0>;
          def intel_sub_group_block_read#!cast<string>(len)#_img  : OclBuiltin<"intel_sub_group_block_read"#len,  (outs !cast<OclType>("v"#len#"u32"):$ret), (ins image2d_ro:$img, v2i32:$coord), [], [], 1, 0>;
          def intel_sub_group_block_write#!cast<string>(len)#_img : OclBuiltin<"intel_sub_group_block_write"#len, (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, !cast<OclType>("v"#len#"u32"):$data), [], [], 1, 0>;
          def intel_sub_group_block_read#!cast<string>(len)#_img_rw  : OclBuiltin<"intel_sub_group_block_read"#len,  (outs !cast<OclType>("v"#len#"u32"):$ret), (ins image2d_rw:$img_rw, v2i32:$coord), [], [], 1, 0>;
          def intel_sub_group_block_write#!cast<string>(len)#_img_rw : OclBuiltin<"intel_sub_group_block_write"#len, (outs void:$ret), (ins image2d_rw:$img_rw, v2i32:$coord, !cast<OclType>("v"#len#"u32"):$data), [], [], 1, 0>;
        }
        // vector
        foreach len = [ 1, 2, 4, 8 ] in {
          foreach vf = [ 4, 8, 16, 32, 64 ] in {
            def intel_sub_group_block_read#!cast<string>(len)#"_"#!cast<string>(vf) :
              OclBuiltin<"intel_sub_group_block_read"#len#"_"#vf, (outs !cast<OclType>("v"#!mul(len, vf)#"u32"):$ret), (ins cgpv1u32:$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
            def intel_sub_group_block_write#!cast<string>(len)#"_"#!cast<string>(vf) :
              OclBuiltin<"intel_sub_group_block_write"#len#"_"#vf, (outs void:$ret), (ins gpv1u32:$data, !cast<OclType>("v"#!mul(len, vf)#"u32"):$src, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
            def intel_sub_group_block_read#!cast<string>(len)#_#!cast<string>(vf)#_img :
              OclBuiltin<"intel_sub_group_block_read"#len#"_"#vf,  (outs !cast<OclType>("v"#!mul(len, vf)#"u32"):$ret), (ins image2d_ro:$img, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
            def intel_sub_group_block_write#!cast<string>(len)#_#!cast<string>(vf)#_img :
              OclBuiltin<"intel_sub_group_block_write"#len#"_"#vf, (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, !cast<OclType>("v"#!mul(len, vf)#"u32"):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
            def intel_sub_group_block_read#!cast<string>(len)#_#!cast<string>(vf)#_img_rw :
              OclBuiltin<"intel_sub_group_block_read"#len#"_"#vf,  (outs !cast<OclType>("v"#!mul(len, vf)#"u32"):$ret), (ins image2d_rw:$img_rw, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
            def intel_sub_group_block_write#!cast<string>(len)#_#!cast<string>(vf)#_img_rw :
              OclBuiltin<"intel_sub_group_block_write"#len#"_"#vf, (outs void:$ret), (ins image2d_rw:$img_rw, v2i32:$coord, !cast<OclType>("v"#!mul(len, vf)#"u32"):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
          }
        }
      }

  let Types = [v1u64, v1u32, v1u16, v1u8] in {
    def intel_sub_group_block_read_us_img  : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix",   (outs gentype:$ret),  (ins image2d_ro:$img, v2i32:$coord), [], [], 1, 0>;
    def intel_sub_group_block_read_us_img_rw  : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix",   (outs gentype:$ret),  (ins image2d_rw:$img_rw, v2i32:$coord), [], [], 1, 0>;
    def intel_sub_group_block_write_us_img  : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix", (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, gentype:$data), [], [], 1, 0>;
    def intel_sub_group_block_write_us_img_rw  : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix", (outs void:$ret), (ins image2d_rw:$img, v2i32:$coord, gentype:$data), [], [], 1, 0>;
    foreach len = [2, 4, 8] in {
      def intel_sub_group_block_read_us#len#_img : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#"#len, (outs !cast<OclType>("gentype"#len):$ret), (ins image2d_ro:$img, v2i32:$coord), [], [], 1, 0>;
      def intel_sub_group_block_read_us#len#_img_rw : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#"#len, (outs !cast<OclType>("gentype"#len):$ret), (ins image2d_rw:$img_rw, v2i32:$coord), [], [], 1, 0>;
      def intel_sub_group_block_write_us#len#_img : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#"#len, (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, !cast<OclType>("gentype"#len):$data), [], [], 1, 0>;
      def intel_sub_group_block_write_us#len#_img_rw : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#"#len, (outs void:$ret), (ins image2d_rw:$img_rw, v2i32:$coord, !cast<OclType>("gentype"#len):$data), [], [], 1, 0>;
    }

    // vectorized
    foreach vf = [4, 8, 16, 32, 64] in {
      foreach len = [1, 2, 4, 8] in {
        def intel_sub_group_block_read_us#len#_#vf#_img  : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#"#len#_#vf,  (outs !cast<OclType>("gentype"#!mul(vf,len)):$ret),  (ins image2d_ro:$img, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
        def intel_sub_group_block_read_us#len#_#vf#_img_rw  : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#"#len#_#vf,  (outs !cast<OclType>("gentype"#!mul(vf,len)):$ret),  (ins image2d_rw:$img, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
        def intel_sub_group_block_write_us#len#_#vf#_img : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#"#len#_#vf,  (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, !cast<OclType>("gentype"#!mul(vf,len)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask),  [], [], 1, 0>;
        def intel_sub_group_block_write_us#len#_#vf#_img_rw  : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#"#len#_#vf,  (outs void:$ret), (ins image2d_rw:$img, v2i32:$coord, !cast<OclType>("gentype"#!mul(vf,len)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask),  [], [], 1, 0>;
      }
    }
  }
  // image block read/write for uchar16 type.
  def intel_sub_group_block_read_us16_img : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#16", (outs gentype16:$ret), (ins image2d_ro:$img, v2i32:$coord), [v1u8], [], 1, 0>;
  def intel_sub_group_block_read_us16_img_rw : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#16", (outs gentype16:$ret), (ins image2d_rw:$img, v2i32:$coord), [v1u8], [], 1, 0>;
  def intel_sub_group_block_write_us16_img : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#16", (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, gentype16:$data), [v1u8], [], 1, 0>;
  def intel_sub_group_block_write_us16_img_rw : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#16", (outs void:$ret), (ins image2d_rw:$img, v2i32:$coord, gentype16:$data), [v1u8], [], 1, 0>;
  foreach vf = [4, 8, 16, 32, 64] in {
    def intel_sub_group_block_read_us16_#vf#_img : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#16_"#vf, (outs !cast<OclType>("gentype"#!mul(16, vf)):$ret),(ins image2d_ro:$img, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
    def intel_sub_group_block_read_us16_#vf#_img_rw : OclBuiltin<"intel_sub_group_block_read_$RetSGBlockOpSuffix#16_"#vf, (outs !cast<OclType>("gentype"#!mul(16, vf)):$ret),(ins image2d_rw:$img, v2i32:$coord, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
    def intel_sub_group_block_write_us16_#vf#_img  : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#16_"#vf,  (outs void:$ret), (ins image2d_wo:$img, v2i32:$coord, !cast<OclType>("gentype"#!mul(16, vf)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
    def intel_sub_group_block_write_us16_#vf#_img_rw  : OclBuiltin<"intel_sub_group_block_write_$Arg2SGBlockOpSuffix#16_"#vf,  (outs void:$ret), (ins image2d_rw:$img, v2i32:$coord, !cast<OclType>("gentype"#!mul(16, vf)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
  }

  let Types = [v1u64, v1u32, v1u16, v1u8] in {
    def intel_sub_group_block_read_us : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix", (outs gentype:$ret),  (ins cgpgentype:$p), [], [], 1, 0>;
    def intel_sub_group_block_write_us  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix", (outs void:$ret), (ins gpgentype:$p, gentype:$data), [], [], 1, 0>;
    foreach len = [2, 4, 8] in {
      def intel_sub_group_block_read_us#len : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#"#len, (outs !cast<OclType>("gentype"#len):$ret), (ins cgpgentype:$p), [], [], 1, 0>;
      def intel_sub_group_block_write_us#len : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#"#len, (outs void:$ret), (ins gpgentype:$p, !cast<OclType>("gentype"#len):$data), [], [], 1, 0>;
    }
    // vectorized
    foreach len = [1, 2, 4, 8] in {
      foreach vf = [4, 8, 16, 32, 64] in {
        def intel_sub_group_block_read_us#len#_#vf : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#"#len#_#vf,  (outs !cast<OclType>("gentype"#!mul(len, vf)):$ret),  (ins cgpgentype:$p, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
        def intel_sub_group_block_write_us#len#_#vf : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#"#len#_#vf,  (outs void:$ret), (ins gpgentype:$p, !cast<OclType>("gentype"#!mul(len, vf)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [], [], 1, 0>;
      }
    }
  }
  // block read/write for uchar16.
  def intel_sub_group_block_read_us16 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#16", (outs gentype16:$ret), (ins cgpgentype:$p), [v1u8], [], 1, 0>;
  def intel_sub_group_block_write_us16 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#16", (outs void:$ret), (ins gpgentype:$p, gentype16:$data), [v1u8], [], 1, 0>;
  foreach vf = [4, 8, 16, 32, 64] in {
    def intel_sub_group_block_read_us16_#vf : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#16_"#vf, (outs !cast<OclType>("gentype"#!mul(16, vf)):$ret),(ins cgpgentype:$p, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
    def intel_sub_group_block_write_us16_#vf : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#16_"#vf, (outs void:$ret), (ins gpgentype:$p, !cast<OclType>("gentype"#!mul(16, vf)):$data, !cast<OclType>("v"#vf#"u32"):$vec_mask), [v1u8], [], 1, 0>;
  }
}

} // VectorAttrs = [KERNEL_CALL_ONCE]

//
// Vectorizer table for scalar and vectorized versions of sub group functions
//
foreach func_name = [ "sub_group_all", "sub_group_any" ] in {
  def func_name#_cell1  : MapCell<!cast<OclBuiltin>(func_name), v1i32>;
  def func_name#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), v4i32>;
  def func_name#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), v8i32>;
  def func_name#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), v16i32>;
  def func_name#_vectorized : MapRow<[
    !cast<MapCell>(func_name#"_cell1"),  //scalar
    nullCell,                            //v2
    !cast<MapCell>(func_name#"_cell4"),  //v4
    !cast<MapCell>(func_name#"_cell8"),  //v8
    !cast<MapCell>(func_name#"_cell16"), //16
    nullCell],                           //3
    0,1>;          //not scalarizable, packetizable
}

foreach func_name = [ "sub_group_broadcast", "sub_group_reduce_add", "sub_group_reduce_min", "sub_group_reduce_max",
                      "sub_group_scan_exclusive_add", "sub_group_scan_exclusive_min", "sub_group_scan_exclusive_max",
                      "sub_group_scan_inclusive_add", "sub_group_scan_inclusive_min", "sub_group_scan_inclusive_max"] in {
  foreach base_type = [ "i32", "u32", "i64", "u64", "f32", "f64" ] in {
    def func_name#_#base_type#_cell1  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v1"#base_type)>;
    def func_name#_#base_type#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), !cast<OclType>("v4"#base_type)>;
    def func_name#_#base_type#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), !cast<OclType>("v8"#base_type)>;
    def func_name#_#base_type#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"vec"), !cast<OclType>("v16"#base_type)>;
    def func_name#_#base_type#_vectorized : MapRow<[
    !cast<MapCell>(func_name#_#base_type#"_cell1"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#base_type#"_cell4"),  //v4
    !cast<MapCell>(func_name#_#base_type#"_cell8"),  //v8
    !cast<MapCell>(func_name#_#base_type#"_cell16"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "sub_group_broadcast", "sub_group_reduce_add", "sub_group_reduce_min", "sub_group_reduce_max",
                      "sub_group_scan_exclusive_add", "sub_group_scan_exclusive_min", "sub_group_scan_exclusive_max",
                      "sub_group_scan_inclusive_add", "sub_group_scan_inclusive_min", "sub_group_scan_inclusive_max"] in {
  foreach base_type = [ "i8", "u8", "i16", "u16" ] in {
    def func_name#_#base_type#_cell1  : MapCell<!cast<OclBuiltin>(func_name#_cs), !cast<OclType>("v1"#base_type)>;
    def func_name#_#base_type#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"vec_cs"), !cast<OclType>("v4"#base_type)>;
    def func_name#_#base_type#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"vec_cs"), !cast<OclType>("v8"#base_type)>;
    def func_name#_#base_type#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"vec_cs"), !cast<OclType>("v16"#base_type)>;
    def func_name#_#base_type#_vectorized : MapRow<[
    !cast<MapCell>(func_name#_#base_type#"_cell1"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#base_type#"_cell4"),  //v4
    !cast<MapCell>(func_name#_#base_type#"_cell8"),  //v8
    !cast<MapCell>(func_name#_#base_type#"_cell16"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "intel_sub_group_block_read", "intel_sub_group_block_write"] in {
  foreach len = [ "1" ] in {
    def func_name#_cell1  : MapCell<!cast<OclBuiltin>(func_name), v1u32>;
    def func_name#_cell4  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4"), v1u32>;
    def func_name#_cell8  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8"), v1u32>;
    def func_name#_cell16 : MapCell<!cast<OclBuiltin>(func_name#len#_#"16"), v1u32>;
    def func_name#_vectorized : MapRow<[
    !cast<MapCell>(func_name#"_cell1"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#"_cell4"),  //v4
    !cast<MapCell>(func_name#"_cell8"),  //v8
    !cast<MapCell>(func_name#"_cell16"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
  foreach len = [ "2", "4", "8" ] in {
    def func_name#_#len#_cell1  : MapCell<!cast<OclBuiltin>(func_name#len), v1u32>;
    def func_name#_#len#_cell4  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4"), v1u32>;
    def func_name#_#len#_cell8  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8"), v1u32>;
    def func_name#_#len#_cell16 : MapCell<!cast<OclBuiltin>(func_name#len#_#"16"), v1u32>;
    def func_name#_#len#_vectorized : MapRow<[
    !cast<MapCell>(func_name#_#len#"_cell1"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#len#"_cell4"),  //v4
    !cast<MapCell>(func_name#_#len#"_cell8"),  //v8
    !cast<MapCell>(func_name#_#len#"_cell16"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "intel_sub_group_block_read", "intel_sub_group_block_write"] in {
  foreach len = [ "1" ] in {
    def func_name#_cell1_img  : MapCell<!cast<OclBuiltin>(func_name#"_img"), v1u32>;
    def func_name#_cell4_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img"), v1u32>;
    def func_name#_cell8_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img"), v1u32>;
    def func_name#_cell16_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img"), v1u32>;
    def func_name#_vectorized_img : MapRow<[
    !cast<MapCell>(func_name#"_cell1_img"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#"_cell4_img"),  //v4
    !cast<MapCell>(func_name#"_cell8_img"),  //v8
    !cast<MapCell>(func_name#"_cell16_img"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
  foreach len = [ "2", "4", "8" ] in {
    def func_name#_#len#_cell1_img  : MapCell<!cast<OclBuiltin>(func_name#len#"_img"), v1u32>;
    def func_name#_#len#_cell4_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img"), v1u32>;
    def func_name#_#len#_cell8_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img"), v1u32>;
    def func_name#_#len#_cell16_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img"), v1u32>;
    def func_name#_#len#_vectorized_img : MapRow<[
    !cast<MapCell>(func_name#_#len#"_cell1_img"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#len#"_cell4_img"),  //v4
    !cast<MapCell>(func_name#_#len#"_cell8_img"),  //v8
    !cast<MapCell>(func_name#_#len#"_cell16_img"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "intel_sub_group_block_read", "intel_sub_group_block_write"] in {
  foreach len = [ "1" ] in {
    def func_name#_cell1_img_rw  : MapCell<!cast<OclBuiltin>(func_name#"_img_rw"), v1u32>;
    def func_name#_cell4_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img_rw"), v1u32>;
    def func_name#_cell8_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img_rw"), v1u32>;
    def func_name#_cell16_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img_rw"), v1u32>;
    def func_name#_vectorized_img_rw : MapRow<[
    !cast<MapCell>(func_name#"_cell1_img_rw"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#"_cell4_img_rw"),  //v4
    !cast<MapCell>(func_name#"_cell8_img_rw"),  //v8
    !cast<MapCell>(func_name#"_cell16_img_rw"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
  foreach len = [ "2", "4", "8" ] in {
    def func_name#_#len#_cell1_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#"_img_rw"), v1u32>;
    def func_name#_#len#_cell4_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img_rw"), v1u32>;
    def func_name#_#len#_cell8_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img_rw"), v1u32>;
    def func_name#_#len#_cell16_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img_rw"), v1u32>;
    def func_name#_#len#_vectorized_img_rw : MapRow<[
    !cast<MapCell>(func_name#_#len#"_cell1_img_rw"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#len#"_cell4_img_rw"),  //v4
    !cast<MapCell>(func_name#_#len#"_cell8_img_rw"),  //v8
    !cast<MapCell>(func_name#_#len#"_cell16_img_rw"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "intel_sub_group_block_read_us", "intel_sub_group_block_write_us"] in {
  foreach type = [ v1u64, v1u32, v1u16, v1u8 ] in {
    foreach len = [ "1" ] in {
      def func_name#!cast<string>(type)#_cell1  : MapCell<!cast<OclBuiltin>(func_name), type>;
      def func_name#!cast<string>(type)#_cell4  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4"), type>;
      def func_name#!cast<string>(type)#_cell8  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8"), type>;
      def func_name#!cast<string>(type)#_cell16 : MapCell<!cast<OclBuiltin>(func_name#len#_#"16"), type>;
      def func_name#!cast<string>(type)#_vectorized : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell1"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell4"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell8"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell16"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
    foreach len = [ "2", "4", "8" ] in {
      def func_name#!cast<string>(type)#_#len#_cell1  : MapCell<!cast<OclBuiltin>(func_name#len), type>;
      def func_name#!cast<string>(type)#_#len#_cell4  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4"), type>;
      def func_name#!cast<string>(type)#_#len#_cell8  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8"), type>;
      def func_name#!cast<string>(type)#_#len#_cell16 : MapCell<!cast<OclBuiltin>(func_name#len#_#"16"), type>;
      def func_name#!cast<string>(type)#_#len#_vectorized : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell1"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell4"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell8"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell16"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
  }
}

foreach func_name = [ "intel_sub_group_block_read_us", "intel_sub_group_block_write_us"] in {
  foreach type = [ v1u8, v1u16, v1u32, v1u64 ] in {
    foreach len = [ "1" ] in {
      def func_name#!cast<string>(type)#_cell1_img_rw  : MapCell<!cast<OclBuiltin>(func_name#"_img_rw"), type>;
      def func_name#!cast<string>(type)#_cell4_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img_rw"), type>;
      def func_name#!cast<string>(type)#_cell8_img_rw  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img_rw"), type>;
      def func_name#!cast<string>(type)#_cell16_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img_rw"), type>;
      def func_name#!cast<string>(type)#_vectorized_img_rw : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell1_img_rw"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell4_img_rw"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell8_img_rw"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell16_img_rw"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
    foreach len = [ "2", "4", "8" ] in {
      def func_name#!cast<string>(type)#_#len#_cell1_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#"_img_rw"), type>;
      def func_name#!cast<string>(type)#_#len#_cell4_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img_rw"), type>;
      def func_name#!cast<string>(type)#_#len#_cell8_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img_rw"), type>;
      def func_name#!cast<string>(type)#_#len#_cell16_img_rw : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img_rw"), type>;
      def func_name#!cast<string>(type)#_#len#_vectorized_img_rw : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell1_img_rw"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell4_img_rw"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell8_img_rw"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell16_img_rw"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
  }
}

foreach func_name = [ "intel_sub_group_block_read_us", "intel_sub_group_block_write_us"] in {
  foreach type = [ v1u8, v1u16, v1u32, v1u64 ] in {
    foreach len = [ "1" ] in {
      def func_name#!cast<string>(type)#_cell1_img  : MapCell<!cast<OclBuiltin>(func_name#"_img"), type>;
      def func_name#!cast<string>(type)#_cell4_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img"), type>;
      def func_name#!cast<string>(type)#_cell8_img  : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img"), type>;
      def func_name#!cast<string>(type)#_cell16_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img"), type>;
      def func_name#!cast<string>(type)#_vectorized_img : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell1_img"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell4_img"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell8_img"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#"_cell16_img"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
    foreach len = [ "2", "4", "8" ] in {
      def func_name#!cast<string>(type)#_#len#_cell1_img : MapCell<!cast<OclBuiltin>(func_name#len#"_img"), type>;
      def func_name#!cast<string>(type)#_#len#_cell4_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"4_img"), type>;
      def func_name#!cast<string>(type)#_#len#_cell8_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"8_img"), type>;
      def func_name#!cast<string>(type)#_#len#_cell16_img : MapCell<!cast<OclBuiltin>(func_name#len#_#"16_img"), type>;
      def func_name#!cast<string>(type)#_#len#_vectorized_img : MapRow<[
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell1_img"),  //scalar
      nullCell,
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell4_img"),  //v4
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell8_img"),  //v8
      !cast<MapCell>(func_name#!cast<string>(type)#_#len#"_cell16_img"), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
  }
}

foreach func_name = [ "intel_sub_group_ballot_igc", "intel_sub_group_ballot" ] in {
  def func_name#_cell1  : MapCell<!cast<OclBuiltin>(func_name), v1i32>;
  def func_name#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"vf4"), v4i32>;
  def func_name#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"vf8"), v8i32>;
  def func_name#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"vf16"), v16i32>;
  def func_name#_vectorized : MapRow<[
    !cast<MapCell>(func_name#"_cell1"),  //scalar
    nullCell,                            //v2
    !cast<MapCell>(func_name#"_cell4"),  //v4
    !cast<MapCell>(func_name#"_cell8"),  //v8
    !cast<MapCell>(func_name#"_cell16"), //16
    nullCell],                           //3
    0,1>;          //not scalarizable, packetizable
}

foreach func_name = [ "sub_group_shuffle", "sub_group_shuffle_xor", "sub_group_shuffle_up", "sub_group_shuffle_down" ] in {
  foreach base_type = [ "i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64" ] in {
    def func_name#_#base_type#_cell1  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v1"#base_type)>;
    def func_name#_#base_type#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"avx"), !cast<OclType>("v4"#base_type)>;
    def func_name#_#base_type#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"avx2"), !cast<OclType>("v8"#base_type)>;
    def func_name#_#base_type#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"avx512"), !cast<OclType>("v16"#base_type)>;
    def func_name#_#base_type#_vectorized : MapRow<[
    !cast<MapCell>(func_name#_#base_type#"_cell1"),  //scalar
    nullCell,
    !cast<MapCell>(func_name#_#base_type#"_cell4"),  //v4
    !cast<MapCell>(func_name#_#base_type#"_cell8"),  //v8
    !cast<MapCell>(func_name#_#base_type#"_cell16"), //16
    nullCell],
    0,1>;          //not scalarizable, packetizable
  }
}

foreach func_name = [ "sub_group_shuffle", "sub_group_shuffle_xor", "sub_group_shuffle_up", "sub_group_shuffle_down" ] in {
  foreach base_type = [ "i8", "u8", "i16", "u16", "i32", "u32", "f32", "i64", "u64" ] in {
    foreach len = [ 2, 3, 4, 8, 16 ] in {
      def func_name#_#"v"#len#base_type#_cell1  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v"#len#base_type)>;
      def func_name#_#"v"#len#base_type#_cell4  : MapCell<!cast<OclBuiltin>(func_name#_#"avx"), !cast<OclType>("v"#!mul(4, len)#base_type)>;
      def func_name#_#"v"#len#base_type#_cell8  : MapCell<!cast<OclBuiltin>(func_name#_#"avx2"), !cast<OclType>("v"#!mul(8, len)#base_type)>;
      def func_name#_#"v"#len#base_type#_cell16 : MapCell<!cast<OclBuiltin>(func_name#_#"avx512"), !cast<OclType>("v"#!mul(16, len)#base_type)>;
      def func_name#_#"v"#len#base_type#_vectorized : MapRow<[
      !cast<MapCell>(func_name#_#"v"#len#base_type#_cell1),  //scalar
      nullCell,
      !cast<MapCell>(func_name#_#"v"#len#base_type#_cell4),  //v4
      !cast<MapCell>(func_name#_#"v"#len#base_type#_cell8),  //v8
      !cast<MapCell>(func_name#_#"v"#len#base_type#_cell16), //16
      nullCell],
      0,1>;          //not scalarizable, packetizable
    }
  }
}

//
// AliasMap
//

foreach suffix = ["", "_avx", "_avx2", "_avx512", "_x32", "_x64"] in {
  defvar types = VFType<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], suffix>.Tout;
  def subgroup_shuffle_alias#suffix : AliasMap<[
    ["sub_group_shuffle"#suffix, "sub_group_shuffle"],
    ["sub_group_shuffle_xor"#suffix, "sub_group_shuffle_xor"]], types>;
}


foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
  foreach op = ["add", "min", "max"] in {
    def sub_group_#func#_#op#_alias : AliasMap<[
    ["sub_group_"#func#"_"#op, "sub_group_non_uniform_"#func#"_"#op]
    ],[]>;
    def sub_group_#func#_#op#_cs_alias : AliasMap<[
    ["sub_group_"#func#"_"#op#"_cs", "sub_group_non_uniform_"#func#"_"#op, "sub_group_"#func#"_"#op]
    ],[]>;
    def sub_group_#func#_#op#_vec_alias : AliasMap<[
    ["sub_group_"#func#"_"#op#"_vec", "sub_group_non_uniform_"#func#"_"#op]
    ],[]>;
    def sub_group_#func#_#op#_vec_cs_alias : AliasMap<[
    ["sub_group_"#func#"_"#op#"_vec_cs", "sub_group_non_uniform_"#func#"_"#op, "sub_group_"#func#"_"#op]
    ],[]>;
  }
}

foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
  foreach op = ["mul", "logical_and", "logical_or", "logical_xor"] in {
    def sub_group_#func#_#op#_alias : AliasMap<[
    ["sub_group_non_uniform_"#func#"_"#op, "sub_group_"#func#"_"#op]
    ],[]>;
    def sub_group_#func#_#op#_vec_alias : AliasMap<[
    ["sub_group_non_uniform_"#func#"_"#op#"_vec", "sub_group_"#func#"_"#op]
    ],[]>;
  }
}

foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
  foreach op = ["and", "or", "xor"] in {
    def sub_group_#func#_#op#_alias : AliasMap<[
    ["sub_group_non_uniform_"#func#"_"#op, "sub_group_"#func#"_bitwise_"#op]
    ],[]>;
    def sub_group_#func#_#op#_vec_alias : AliasMap<[
    ["sub_group_non_uniform_"#func#"_"#op#"_vec", "sub_group_"#func#"_bitwise_"#op]
    ],[]>;
  }
}

//
// Generate VectInfo for DPCPPKernelVecClone
//

let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64] in {
  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    def sub_group_non_uniform_#func#_#"mul_vect_info" : VectInfo<[], !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_mul"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_mul_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_mul_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_mul_vec")>;
  }
}

let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64] in {
  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    foreach op = ["and", "or", "xor"] in {
    def sub_group_non_uniform_#func#_#op#_vect_info : VectInfo<[], !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec")>;
    }
  }
}

let Types = [v1i32] in {
  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    foreach op = ["logical_and", "logical_or", "logical_xor"] in {
    def sub_group_non_uniform_#func#_#op#_vect_info : VectInfo<[], !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec"), !cast<OclBuiltin>("sub_group_non_uniform_"#func#"_"#op#"_vec")>;
    }
  }
}

let Types = [v1i32] in {
  def sub_group_all_vect_info : VectInfo<[], sub_group_all, sub_group_all_vec, sub_group_all_vec, sub_group_all_vec, sub_group_all_vec, sub_group_all_vec>;
  def sub_group_any_vect_info : VectInfo<[], sub_group_any, sub_group_any_vec, sub_group_any_vec, sub_group_any_vec, sub_group_any_vec, sub_group_any_vec>;
}
let Types = [ v1i32, v1u32, v1i64, v1u64, v1f32, v1f64 ] in {
  let HandleAlias = 0 in {
    def sub_group_broadcast_vect_info : VectInfo<[], sub_group_broadcast, sub_group_broadcast_vec, sub_group_broadcast_vec, sub_group_broadcast_vec>;
    def sub_group_broadcast_assume_uniform_vect_info : VectInfo<[], sub_group_broadcast, sub_group_broadcast_assume_uniform_vec, sub_group_broadcast_assume_uniform_vec, sub_group_broadcast_assume_uniform_vec>;
  }

  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    foreach op = ["add", "min", "max"] in {
      defvar vec_variant = !cast<OclBuiltin>("sub_group_"#func#_#op#"_vec");
      def sub_group_#func#_#op#_vect_info : VectInfo<[], !cast<OclBuiltin>("sub_group_"#func#_#op), vec_variant, vec_variant, vec_variant, vec_variant, vec_variant>;
    }
  }
}

let Types = [v1i8, v1u8, v1i16, v1u16] in {
  let HandleAlias = 0 in {
    def sub_group_broadcast_cs_vect_info : VectInfo<[], sub_group_broadcast_cs, sub_group_broadcast_vec_cs, sub_group_broadcast_vec_cs, sub_group_broadcast_vec_cs>;
    def sub_group_broadcast_assume_uniform_cs_vect_info : VectInfo<[], sub_group_broadcast_cs, sub_group_broadcast_assume_uniform_vec_cs, sub_group_broadcast_assume_uniform_vec_cs, sub_group_broadcast_assume_uniform_vec_cs>;
  }

  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    foreach op = ["add", "min", "max"] in {
      defvar vec_variant = !cast<OclBuiltin>("sub_group_"#func#_#op#"_vec_cs");
      def sub_group_#func#_#op#_cs_vect_info : VectInfo<[], !cast<OclBuiltin>("sub_group_"#func#_#op#"_cs"), vec_variant, vec_variant, vec_variant, vec_variant, vec_variant>;
    }
  }
}

let Types = [v2i8, v2u8, v2i16, v2u16, v2i32, v2u32, v2i64, v2u64, v2f32, v2f64,
             v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32, v3f64,
             v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64, v4f32, v4f64,
             v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64, v8f32, v8f64,
             v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64] in {
def sub_group_broadcast_extended_vect_info : VectInfo<[], sub_group_broadcast_extended, sub_group_broadcast_extended_avx, sub_group_broadcast_extended_avx2, sub_group_broadcast_extended_avx512>;
def sub_group_broadcast_assume_uniform_extended_vect_info : VectInfo<[], sub_group_broadcast_assume_uniform_extended, sub_group_broadcast_assume_uniform_extended_avx, sub_group_broadcast_assume_uniform_extended_avx2, sub_group_broadcast_assume_uniform_extended_avx512>;
}
let Types = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64, v1i8, v1u8, v1i16, v1u16,
             v2i32, v4i32, v8i32, v16i32, v3i32,
             v2u32, v4u32, v8u32, v16u32, v3u32,
             v2f32, v4f32, v8f32, v16f32, v3f32,
             v2i64, v4i64, v8i64, v16i64, v3i64,
             v2u64, v4u64, v8u64, v16u64, v3u64,
             v2i8 , v4i8 , v8i8 , v16i8 , v3i8 ,
             v2u8 , v4u8 , v8u8 , v16u8 , v3u8 ,
             v2i16, v4i16, v8i16, v16i16, v3i16,
             v2u16, v4u16, v8u16, v16u16, v3u16] in {
def sub_group_shuffle_vect_info : VectInfo<[], sub_group_shuffle, sub_group_shuffle_avx, sub_group_shuffle_avx2, sub_group_shuffle_avx512, sub_group_shuffle_x32, sub_group_shuffle_x64>;
def sub_group_shuffle_xor_vect_info : VectInfo<[], sub_group_shuffle_xor, sub_group_shuffle_xor_avx, sub_group_shuffle_xor_avx2, sub_group_shuffle_xor_avx512, sub_group_shuffle_xor_x32, sub_group_shuffle_xor_x64>;
def sub_group_shuffle_down_vect_info : VectInfo<[], sub_group_shuffle_down, sub_group_shuffle_down_avx, sub_group_shuffle_down_avx2, sub_group_shuffle_down_avx512, sub_group_shuffle_down_x32, sub_group_shuffle_down_x64>;
def sub_group_shuffle_up_vect_info : VectInfo<[], sub_group_shuffle_up, sub_group_shuffle_up_avx, sub_group_shuffle_up_avx2, sub_group_shuffle_up_avx512, sub_group_shuffle_up_x32, sub_group_shuffle_up_x64>;
}

let Types = [v1u32], NeedWidenTypes = 0 in {
  def intel_sub_group_block_read_vect_info :VectInfo<[], intel_sub_group_block_read, intel_sub_group_block_read1_4, intel_sub_group_block_read1_8, intel_sub_group_block_read1_16, intel_sub_group_block_read1_32, intel_sub_group_block_read1_64>;
  def intel_sub_group_block_write_vect_info :VectInfo<[], intel_sub_group_block_write, intel_sub_group_block_write1_4, intel_sub_group_block_write1_8, intel_sub_group_block_write1_16, intel_sub_group_block_write1_32, intel_sub_group_block_write1_64>;
  def intel_sub_group_block_read_img_vect_info :VectInfo<[], intel_sub_group_block_read_img, intel_sub_group_block_read1_4_img, intel_sub_group_block_read1_8_img, intel_sub_group_block_read1_16_img, intel_sub_group_block_read1_32_img, intel_sub_group_block_read1_64_img>;
  def intel_sub_group_block_write_img_vect_info :VectInfo<[], intel_sub_group_block_write_img, intel_sub_group_block_write1_4_img, intel_sub_group_block_write1_8_img, intel_sub_group_block_write1_16_img, intel_sub_group_block_write1_32_img, intel_sub_group_block_write1_64_img>;
  def intel_sub_group_block_read_img_rw_vect_info :VectInfo<[], intel_sub_group_block_read_img_rw, intel_sub_group_block_read1_4_img_rw, intel_sub_group_block_read1_8_img_rw, intel_sub_group_block_read1_16_img_rw, intel_sub_group_block_read1_32_img_rw, intel_sub_group_block_read1_64_img_rw>;
  def intel_sub_group_block_write_img_rw_vect_info :VectInfo<[], intel_sub_group_block_write_img_rw, intel_sub_group_block_write1_4_img_rw, intel_sub_group_block_write1_8_img_rw, intel_sub_group_block_write1_16_img_rw, intel_sub_group_block_write1_32_img_rw, intel_sub_group_block_write1_64_img_rw>;
  foreach len = [2, 4, 8 ] in {
    def intel_sub_group_block_read#len#_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read"#len), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_4"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_8"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_16"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_32"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_64")>;
    def intel_sub_group_block_write#len#_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write"#len), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_4"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_8"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_16"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_32"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_64")>;
    def intel_sub_group_block_read#len#_img_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_img"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_4_img"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_8_img"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_16_img"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_32_img"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_64_img")>;
    def intel_sub_group_block_write#len#_img_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_img"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_4_img"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_8_img"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_16_img"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_32_img"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_64_img")>;
    def intel_sub_group_block_read#len#_img_rw_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_4_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_8_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_16_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_32_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read"#len#"_64_img_rw")>;
    def intel_sub_group_block_write#len#_img_rw_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_4_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_8_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_16_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_32_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write"#len#"_64_img_rw")>;
  }
}

let Types = [v1u8, v1u16, v1u32, v1u64], NeedWidenTypes = 0 in {
  def intel_sub_group_block_read_us_vect_info :VectInfo<[], intel_sub_group_block_read_us, intel_sub_group_block_read_us1_4, intel_sub_group_block_read_us1_8, intel_sub_group_block_read_us1_16, intel_sub_group_block_read_us1_32, intel_sub_group_block_read_us1_64>;
  def intel_sub_group_block_write_us_vect_info :VectInfo<[], intel_sub_group_block_write_us, intel_sub_group_block_write_us1_4, intel_sub_group_block_write_us1_8, intel_sub_group_block_write_us1_16, intel_sub_group_block_write_us1_32, intel_sub_group_block_write_us1_64>;
  def intel_sub_group_block_read_us_img_vect_info :VectInfo<[], intel_sub_group_block_read_us_img, intel_sub_group_block_read_us1_4_img, intel_sub_group_block_read_us1_8_img, intel_sub_group_block_read_us1_16_img, intel_sub_group_block_read_us1_32_img, intel_sub_group_block_read_us1_64_img>;
  def intel_sub_group_block_write_us_img_vect_info :VectInfo<[], intel_sub_group_block_write_us_img, intel_sub_group_block_write_us1_4_img, intel_sub_group_block_write_us1_8_img, intel_sub_group_block_write_us1_16_img, intel_sub_group_block_write_us1_32_img, intel_sub_group_block_write_us1_64_img>;
  def intel_sub_group_block_read_us_img_rw_vect_info :VectInfo<[], intel_sub_group_block_read_us_img_rw, intel_sub_group_block_read_us1_4_img_rw, intel_sub_group_block_read_us1_8_img_rw, intel_sub_group_block_read_us1_16_img_rw, intel_sub_group_block_read_us1_32_img_rw, intel_sub_group_block_read_us1_64_img_rw>;
  def intel_sub_group_block_write_us_img_rw_vect_info :VectInfo<[], intel_sub_group_block_write_us_img_rw, intel_sub_group_block_write_us1_4_img_rw, intel_sub_group_block_write_us1_8_img_rw, intel_sub_group_block_write_us1_16_img_rw, intel_sub_group_block_write_us1_32_img_rw, intel_sub_group_block_write_us1_64_img_rw>;
  foreach len = [2, 4, 8 ] in {
    def intel_sub_group_block_read_us#len#_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read_us"#len), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_4"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_8"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_16"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_32"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_64")>;
    def intel_sub_group_block_write_us#len#_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write_us"#len), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_4"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_8"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_16"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_32"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_64")>;
    def intel_sub_group_block_read_us#len#_img_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_img"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_4_img"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_8_img"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_16_img"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_32_img"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_64_img")>;
    def intel_sub_group_block_write_us#len#_img_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_img"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_4_img"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_8_img"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_16_img"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_32_img"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_64_img")>;
    def intel_sub_group_block_read_us#len#_img_rw_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_4_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_8_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_16_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_32_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_read_us"#len#"_64_img_rw")>;
    def intel_sub_group_block_write_us#len#_img_rw_vect_info :VectInfo<[], !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_4_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_8_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_16_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_32_img_rw"), !cast<OclBuiltin>("intel_sub_group_block_write_us"#len#"_64_img_rw")>;
  }
}

// Mapping for uchar16 block read/write
let Types = [v1u8], NeedWidenTypes=0 in {
    def intel_sub_group_block_read_us16_vect_info :VectInfo<[], intel_sub_group_block_read_us16, intel_sub_group_block_read_us16_4, intel_sub_group_block_read_us16_8, intel_sub_group_block_read_us16_16, intel_sub_group_block_read_us16_32, intel_sub_group_block_read_us16_64>;
    def intel_sub_group_block_write_us16_vect_info :VectInfo<[], intel_sub_group_block_write_us16, intel_sub_group_block_write_us16_4, intel_sub_group_block_write_us16_8, intel_sub_group_block_write_us16_16, intel_sub_group_block_write_us16_32, intel_sub_group_block_write_us16_64>;
    def intel_sub_group_block_read_us16_img_vect_info :VectInfo<[], intel_sub_group_block_read_us16_img, intel_sub_group_block_read_us16_4_img, intel_sub_group_block_read_us16_8_img, intel_sub_group_block_read_us16_16_img, intel_sub_group_block_read_us16_32_img, intel_sub_group_block_read_us16_64_img>;
    def intel_sub_group_block_write_us16_img_vect_info :VectInfo<[], intel_sub_group_block_write_us16_img, intel_sub_group_block_write_us16_4_img, intel_sub_group_block_write_us16_8_img, intel_sub_group_block_write_us16_16_img, intel_sub_group_block_write_us16_32_img, intel_sub_group_block_write_us16_64_img>;
    def intel_sub_group_block_read_us16_img_rw_vect_info :VectInfo<[], intel_sub_group_block_read_us16_img_rw, intel_sub_group_block_read_us16_4_img_rw, intel_sub_group_block_read_us16_8_img_rw, intel_sub_group_block_read_us16_16_img_rw, intel_sub_group_block_read_us16_32_img_rw, intel_sub_group_block_read_us16_64_img_rw>;
    def intel_sub_group_block_write_us16_img_rw_vect_info :VectInfo<[], intel_sub_group_block_write_us16_img_rw, intel_sub_group_block_write_us16_4_img_rw, intel_sub_group_block_write_us16_8_img_rw, intel_sub_group_block_write_us16_16_img_rw, intel_sub_group_block_write_us16_32_img_rw, intel_sub_group_block_write_us16_64_img_rw>;
}

let Types = [v1i32] in {
def intel_sub_group_ballot_vect_info : VectInfo<[], intel_sub_group_ballot, intel_sub_group_ballot_vf4, intel_sub_group_ballot_vf8, intel_sub_group_ballot_vf16>;
}
def  khr_sub_group_shuffle_up_vect_info : VectInfo<khr_shuffle_gentype, khr_sub_group_shuffle_up, khr_sub_group_shuffle_up_avx, khr_sub_group_shuffle_up_avx2, khr_sub_group_shuffle_up_avx512, khr_sub_group_shuffle_up_x32, khr_sub_group_shuffle_up_x64>;
def  khr_sub_group_shuffle_down_vect_info : VectInfo<khr_shuffle_gentype, khr_sub_group_shuffle_down, khr_sub_group_shuffle_down_avx, khr_sub_group_shuffle_down_avx2, khr_sub_group_shuffle_down_avx512, khr_sub_group_shuffle_up_x32, khr_sub_group_shuffle_up_x64>;

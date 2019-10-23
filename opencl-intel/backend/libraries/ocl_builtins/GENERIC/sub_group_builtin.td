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

  let Types = [ v4i32,  v8i32,  v16i32 ] in {
    def sub_group_all_vec  : OclBuiltin<"sub_group_all",  (outs gentype:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
    def sub_group_any_vec  : OclBuiltin<"sub_group_any",  (outs gentype:$ret), (ins gentype:$predicate, u32gentype:$vec_mask), [], [], 1, 0>;
  }

  let Types = [ v1i32, v1u32, v1i64, v1u64, v1f32, v1f64 ] in {
        def sub_group_broadcast           : OclBuiltin<"sub_group_broadcast",           (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id), [], [], 1, 0>;
        def sub_group_reduce_add          : OclBuiltin<"sub_group_reduce_add",          (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_reduce_min          : OclBuiltin<"sub_group_reduce_min",          (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_reduce_max          : OclBuiltin<"sub_group_reduce_max",          (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_exclusive_add  : OclBuiltin<"sub_group_scan_exclusive_add",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_exclusive_min  : OclBuiltin<"sub_group_scan_exclusive_min",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_exclusive_max  : OclBuiltin<"sub_group_scan_exclusive_max",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_inclusive_add  : OclBuiltin<"sub_group_scan_inclusive_add",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_inclusive_min  : OclBuiltin<"sub_group_scan_inclusive_min",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
        def sub_group_scan_inclusive_max  : OclBuiltin<"sub_group_scan_inclusive_max",  (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
      }

  // Vectorized reductions
  let Types = [ v4i32, v8i32, v16i32,
                v4u32, v8u32, v16u32,
                v4i64, v8i64, v16i64,
                v4u64, v8u64, v16u64,
                v4f32, v8f32, v16f32,
                v4f64, v8f64, v16f64 ] in {
        def sub_group_broadcast_vec           : OclBuiltin<"sub_group_broadcast",           (outs gentype:$ret), (ins gentype:$src, uint:$sub_group_local_id, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_reduce_add_vec          : OclBuiltin<"sub_group_reduce_add",          (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_reduce_min_vec          : OclBuiltin<"sub_group_reduce_min",          (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_reduce_max_vec          : OclBuiltin<"sub_group_reduce_max",          (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_exclusive_add_vec  : OclBuiltin<"sub_group_scan_exclusive_add",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_exclusive_min_vec  : OclBuiltin<"sub_group_scan_exclusive_min",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_exclusive_max_vec  : OclBuiltin<"sub_group_scan_exclusive_max",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_inclusive_add_vec  : OclBuiltin<"sub_group_scan_inclusive_add",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_inclusive_min_vec  : OclBuiltin<"sub_group_scan_inclusive_min",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
        def sub_group_scan_inclusive_max_vec  : OclBuiltin<"sub_group_scan_inclusive_max",  (outs gentype:$ret), (ins gentype:$src, u32gentype:$vec_mask), [], [], 1, 0>;
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
  let Types = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64,
               v2i32, v4i32, v8i32, v16i32,
               v2u32, v4u32, v8u32, v16u32,
               v2f32, v4f32, v8f32, v16f32] in {
        def sub_group_shuffle :     OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v1u32: $sub_group_local_id), [], [], 1, 0>;
        def sub_group_shuffle_xor : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v1u32: $value), [], [], 1, 0>;
        def sub_group_shuffle_down : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v1u32:$value), [], [], 1, 0>;
        def sub_group_shuffle_up : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v1u32:$value), [], [], 1, 0>;
      }

  // AVX (VF = 4)
  let Types = [v4i32, v4u32, v4i64, v4u64, v4f32, v4f64,
               v8i32, v16i32, v32i32, v64i32,
               v8u32, v16u32, v32u32, v64u32,
               v8f32, v16f32, v32f32, v64f32 ] in {
        def sub_group_shuffle_avx : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v4u32:$sub_group_local_id, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v4u32:$value, v4u32:$vec_mask), [], [], 1, 0>;
      }

  // AVX2 (VF = 8)
  let Types = [v8i32, v8u32, v8i64, v8u64, v8f32, v8f64,
               v16i32, v32i32, v64i32, v128i32,
               v16u32, v32u32, v64u32, v128u32,
               v16f32, v32f32, v64f32, v128f32 ] in {
        def sub_group_shuffle_avx2 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v8u32:$sub_group_local_id, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx2 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx2 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx2 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v8u32:$value, v8u32:$vec_mask), [], [], 1, 0>;
      }

  // AVX512 (VF = 16)
  let Types = [v16i32, v16u32, v16i64, v16u64, v16f32, v16f64,
               v32i32, v64i32, v128i32, v256i32,
               v32u32, v64u32, v128u32, v256u32,
               v32f32, v64f32, v128f32, v256f32 ] in {
        def sub_group_shuffle_avx512 : OclBuiltin<"intel_sub_group_shuffle", (outs gentype:$ret), (ins gentype:$data, v16u32:$sub_group_local_id, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_xor_avx512 : OclBuiltin<"intel_sub_group_shuffle_xor", (outs gentype:$ret), (ins gentype:$data, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_down_avx512 : OclBuiltin<"intel_sub_group_shuffle_down", (outs gentype:$ret), (ins gentype:$current, gentype:$next, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
        def sub_group_shuffle_up_avx512 : OclBuiltin<"intel_sub_group_shuffle_up", (outs gentype:$ret), (ins gentype:$previous, gentype:$current, v16u32:$value, v16u32:$vec_mask), [], [], 1, 0>;
      }

  // Sub group block read & write
  // uint
  // TBD: "ui" suffixed aliases for consitensy (cl_intel_subgroups_short)
  let Types = [ v1u32 ] in {
        // scalar
        def intel_sub_group_block_read  : OclBuiltin<"intel_sub_group_block_read",  (outs gentype:$ret), (ins cgpv1u32:$p), [], [], 1, 0>;
        def intel_sub_group_block_write : OclBuiltin<"intel_sub_group_block_write", (outs void:$ret), (ins gpv1u32:$p, gentype:$data), [], [], 1, 0>;
        foreach len = [ 2, 4, 8 ] in {
          def intel_sub_group_block_read#!cast<string>(len) : OclBuiltin<"intel_sub_group_block_read"#len,  (outs !cast<OclType>("v"#len#"u32"):$ret), (ins cgpv1u32:$p), [], [], 1, 0>;
          def intel_sub_group_block_write#!cast<string>(len) : OclBuiltin<"intel_sub_group_block_write"#len, (outs void:$ret), (ins gpv1u32:$p, !cast<OclType>("v"#len#"u32"):$data), [], [], 1, 0>;
        }
        // vector
        foreach len = [ 1, 2, 4, 8 ] in {
          foreach vf = [ 4, 8, 16 ] in {
            def intel_sub_group_block_read#!cast<string>(len)#"_"#!cast<string>(vf) :
              OclBuiltin<"intel_sub_group_block_read"#len#"_"#vf, (outs !cast<OclType>("v"#!mul(len, vf)#"u32"):$ret), (ins cgpv1u32:$data), [], [], 1, 0>;
            def intel_sub_group_block_write#!cast<string>(len)#"_"#!cast<string>(vf) :
              OclBuiltin<"intel_sub_group_block_write"#len#"_"#vf, (outs void:$ret), (ins gpv1u32:$data, !cast<OclType>("v"#!mul(len, vf)#"u32"):$src), [], [], 1, 0>;
          }
        }
      }
  // These are horribly inconvenient to represent in OpenCL TableGen.
  // Non-triviality comes from the need to represent the ret type for read and arg1 for write as multiple of base vector size and VF
  // and ushort
  let Types = [ v1u64, v1u32, v1u16, v1u8 ] in {
        def intel_sub_group_block_read_us  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix",   (outs gentype:$ret),  (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us2 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#2", (outs gentype2:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us4 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#4", (outs gentype4:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us8 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#8", (outs gentype8:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        // vectorized
        def intel_sub_group_block_read_us1_4  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#1_4",  (outs gentype4:$ret),  (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us1_8  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#1_8",  (outs gentype8:$ret),  (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us1_16 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#1_16", (outs gentype16:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us2_4  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#2_4",  (outs gentype8:$ret),  (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us2_8  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#2_8",  (outs gentype16:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us2_16 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#2_16", (outs gentype32:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us4_4  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#4_4",  (outs gentype16:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us4_8  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#4_8",  (outs gentype32:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us4_16 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#4_16", (outs gentype64:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us8_4  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#8_4",  (outs gentype32:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us8_8  : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#8_8",  (outs gentype64:$ret), (ins cgpgentype:$p), [], [], 1, 0>;
        def intel_sub_group_block_read_us8_16 : OclBuiltin<"intel_sub_group_block_read_$Arg0SGBlockOpSuffix#8_16", (outs gentype128:$ret),(ins cgpgentype:$p), [], [], 1, 0>;
        // write
        def intel_sub_group_block_write_us  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix", (outs void:$ret), (ins gpgentype:$p, gentype:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us2 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#2", (outs void:$ret), (ins gpgentype:$p, gentype2:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us4 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#4", (outs void:$ret), (ins gpgentype:$p, gentype4:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us8 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#8", (outs void:$ret), (ins gpgentype:$p, gentype8:$data), [], [], 1, 0>;
        // vectorized
        def intel_sub_group_block_write_us1_4  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#1_4",  (outs void:$ret), (ins gpgentype:$p, gentype4:$data),  [], [], 1, 0>;
        def intel_sub_group_block_write_us1_8  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#1_8",  (outs void:$ret), (ins gpgentype:$p, gentype8:$data),  [], [], 1, 0>;
        def intel_sub_group_block_write_us1_16 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#1_16", (outs void:$ret), (ins gpgentype:$p, gentype16:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us2_4  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#2_4",  (outs void:$ret), (ins gpgentype:$p, gentype8:$data),  [], [], 1, 0>;
        def intel_sub_group_block_write_us2_8  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#2_8",  (outs void:$ret), (ins gpgentype:$p, gentype16:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us2_16 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#2_16", (outs void:$ret), (ins gpgentype:$p, gentype32:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us4_4  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#4_4",  (outs void:$ret), (ins gpgentype:$p, gentype16:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us4_8  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#4_8",  (outs void:$ret), (ins gpgentype:$p, gentype32:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us4_16 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#4_16", (outs void:$ret), (ins gpgentype:$p, gentype64:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us8_4  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#8_4",  (outs void:$ret), (ins gpgentype:$p, gentype32:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us8_8  : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#8_8",  (outs void:$ret), (ins gpgentype:$p, gentype64:$data), [], [], 1, 0>;
        def intel_sub_group_block_write_us8_16 : OclBuiltin<"intel_sub_group_block_write_$Arg0SGBlockOpSuffix#8_16", (outs void:$ret), (ins gpgentype:$p, gentype128:$data),[], [], 1, 0>;
      }
}

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
  foreach base_type = [ "i32", "u32", "i64", "u64", "f32", "f64" ] in {
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
  foreach base_type = [ "i32", "u32", "f32" ] in { // omit ulong, long, double per cl_intel_subgroups spec
    foreach len = [ 2, 4, 8, 16 ] in {
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

// Copyright (C) 2023 Intel Corporation
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

include "GENERIC/ll_generation/definition.td"

// declare void @llvm.memcpy.p0i8.p0i8.i32(i8*, i8*, i32, i1)
let EmitMangledName = false in {
    def : LLDeclare<"llvm.memcpy.p4i8.p4i8.i64", [!cast<PointerType>("pAS4i8"), 
          !cast<PointerType>("pAS4i8"), !cast<Type>("i64"), !cast<Type>("i1")], void, "">;
}

class GetIRTypeName<string type_name> {
  string ret = !cond(
    !eq(type_name, "u8"): "i8",
    !eq(type_name, "u16") : "i16",
    !eq(type_name, "u32") : "i32",
    !eq(type_name, "u64") : "i64",
    true : type_name);
}
class GetSignedInfoByTypeName<string type_name> {
  bit ret = !cond(
    !or(!eq(type_name, "u8"), !eq(type_name, "u16"), !eq(type_name, "u32"), 
    !eq(type_name, "u64")): false, true : true);
}

class GetWidthByTypeName<string type_name> {
  string ret = !cond(
        !or(!eq(type_name, "i8"), !eq(type_name, "u8")): "1",
        !or(!eq(type_name, "i16"), !eq(type_name, "u16"), !eq(type_name, "f16")): "2",
        !or(!eq(type_name, "i32"), !eq(type_name, "u32"), !eq(type_name, "f32")): "4",
        !or(!eq(type_name, "i64"), !eq(type_name, "u64"), !eq(type_name, "f64")): "8");
}

defvar private_sort_copy_code = [{
    %src_ptr_i8 = bitcast {Args[0]} to i8 addrspace(4)*
    %per_item_size_i64 = sext {Args[1]} to i64
    %size = mul i64 %per_item_size_i64, {BIT_WIDTH}
    %pre_elements_size = mul {Args[3]}, %size
    %start = getelementptr i8, {Args[2]}, i64 %pre_elements_size
    br {Args[5]}, label %ToScratch, label %FromScratch
  ToScratch:
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %start, i8 addrspace(4)* %src_ptr_i8, i64 %size, i1 false)
    br label %exit
  FromScratch:
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %src_ptr_i8, i8 addrspace(4)* %start, i64 %size, i1 false)
    br label %exit
  exit:
    ret void
}];

defvar private_sort_copy_code_vec = [{
  entry:
    br label %for.cond
  for.cond:
    %sgid = phi i64 [ 0, %entry ], [ %add, %for.inc ]
    %cmp = icmp ult i64 %sgid, {VEC_LEN}
    br i1 %cmp, label %for.body, label %for.end
  for.body:
    %vecext = extractelement {Args[0]}, i64 %sgid
    %llidx = add {Args[3]}, %sgid
    call void @{CALL_FUNC}({EXT_TYPE} %vecext, {Args[1]}, {Args[2]}, i64 %llidx, {Args[4]}, {Args[5]})
    br label %for.inc
  for.inc:
    %add = add i64 %sgid, 1
    br label %for.cond
  for.end:
    ret void
}];

defvar private_key_value_sort_copy_code = [{
    %key_ptr_i8 = bitcast {Args[0]} to i8 addrspace(4)*
    %value_ptr_i8 = bitcast {Args[1]} to i8 addrspace(4)*
    %per_item_size_i64 = sext {Args[2]} to i64
    %key_per_item_size = mul i64 %per_item_size_i64, {KEY_BIT_WIDTH}
    %key_pre_elements_size = mul {Args[4]}, %key_per_item_size
    %key_start = getelementptr i8, {Args[3]}, i64 %key_pre_elements_size
    %key_total_size = mul {Args[5]}, %key_per_item_size
    %value_per_item_size = mul i64 %per_item_size_i64, {VALUE_BIT_WIDTH}
    %value_pre_elements_size = mul {Args[4]}, %value_per_item_size
    %value_start_idx = add i64 %key_total_size, %value_pre_elements_size
    %value_start = getelementptr i8, {Args[3]}, i64 %value_start_idx
    br {Args[6]}, label %ToScratch, label %FromScratch
  ToScratch:
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %key_start, i8 addrspace(4)* %key_ptr_i8, i64 %key_per_item_size, i1 false)
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %value_start, i8 addrspace(4)* %value_ptr_i8, i64 %value_per_item_size, i1 false)
    br label %exit
  FromScratch:
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %key_ptr_i8, i8 addrspace(4)* %key_start, i64 %key_per_item_size, i1 false)
    call void @llvm.memcpy.p4i8.p4i8.i64(i8 addrspace(4)* %value_ptr_i8, i8 addrspace(4)* %value_start, i64 %value_per_item_size, i1 false)
    br label %exit
  exit:
    ret void
}];

defvar private_key_value_sort_copy_code_vec = [{
  entry:
    br label %for.cond
  for.cond:
    %sgid = phi i64 [ 0, %entry ], [ %add, %for.inc ]
    %cmp = icmp ult i64 %sgid, {VEC_LEN}
    br i1 %cmp, label %for.body, label %for.end
  for.body:
    %key_vecext = extractelement {Args[0]}, i64 %sgid
    %value_vecext = extractelement {Args[1]}, i64 %sgid
    %llidx = add {Args[4]}, %sgid
    call void @{CALL_FUNC}({KEY_EXT_TYPE} %key_vecext, {VALUE_EXT_TYPE} %value_vecext, {Args[2]}, {Args[3]}, i64 %llidx, {Args[5]}, {Args[6]})
    br label %for.inc
  for.inc:
    %add = add i64 %sgid, 1
    br label %for.cond
  for.end:
    ret void
}];


// Copy memory from/to scratch memory for work group sort -----
// Implemented by llvm.memcpy
let EmitMangledName = true,
  FuncAttrs = ["memory(readwrite)", "nounwind"] in {
  defvar builtin = "__devicelib_default_work_group_private_sort_copy";
  defvar spread_builtin = "__devicelib_default_work_group_private_spread_sort_copy";
  foreach type_name = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
    defvar type = GetIRTypeName<type_name>.ret;
    defvar signed = GetSignedInfoByTypeName<type_name>.ret;
    // arg type
    defvar data_type = !cast<PointerType>("pAS4"#type);
    defvar temp_type = !cast<PointerType>("pAS4i8");
    defvar int_type = !cast<IntType>("i64");
    defvar flag_type = !cast<IntType>("i1");
    defvar size_type = !cast<IntType>("i32");
    // arg type mangle
    defvar data_mangle = ManglePointerType<data_type, /*signed*/ signed>.ret;
    defvar int_mangle = MangleType<int_type, /*signed*/ true>.ret;
    defvar flag_mangle = MangleType<flag_type>.ret;
    defvar temp_mangle = !cond(!eq(type_name, "i8"): "S0_", 
                               true: ManglePointerType<temp_type, true>.ret);
    defvar size_mangle = MangleType<size_type, /*signed*/ false>.ret;

    defvar bit_width = GetWidthByTypeName<type_name>.ret;
    // --- key-only, scalar
    defm sort_copy_builtin # type_name : LLDefine<builtin, [Value<data_type, "src_ptr">,  
        Value<size_type, "per_item_size">, Value<temp_type, "dst_ptr">,
        Value<int_type, "local_id">, Value<int_type, "local_size">, 
        Value<flag_type, "direction">], void, private_sort_copy_code, 
        [Macro<"BIT_WIDTH", !cast<string>(bit_width)>], 
        data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle>;
    
    defvar scalar_spread_func_name = "_Z" # !size(spread_builtin) # spread_builtin # 
          data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle;
    let EmitMangledName = false in
    def : LLDeclare<scalar_spread_func_name, [data_type, size_type, temp_type, int_type, int_type, flag_type], void, "">;

    // --- key-only, vector
    foreach vf = [4, 8, 16, 32, 64] in {
      // vector arg type
      defvar vec_data_type = VectorType<data_type, vf>;
      // vector arg mangle info
      defvar vec_data_mangle = MangleVectorOfPointersType<vec_data_type, signed>.ret;
      defvar scalar_func_name = "_Z" # !size(builtin) # builtin # 
          data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle;
      // vector work group key only copy define  
      defm sort_copy_builtin # type_name #vf : LLDefine<builtin, [Value<vec_data_type, "src_ptr">,  
          Value<size_type, "per_item_size">, Value<temp_type, "dst_ptr">,
          Value<int_type, "local_id">, Value<int_type, "local_size">, Value<flag_type, "direction">],
          void, private_sort_copy_code_vec, 
          [Macro<"BIT_WIDTH", !cast<string>(bit_width)>, Macro<"VEC_LEN", !cast<string>(vf)>,
           Macro<"CALL_FUNC", !cast<string>(scalar_func_name)>, Macro<"EXT_TYPE", !cast<string>(data_type.Name)>], 
          vec_data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle>;

      // vector work group key only spread copy define  
      defm sort_spread_copy_builtin # type_name #vf : LLDefine<spread_builtin, [Value<vec_data_type, "src_ptr">,  
          Value<size_type, "per_item_size">, Value<temp_type, "dst_ptr">,
          Value<int_type, "local_id">, Value<int_type, "local_size">, Value<flag_type, "direction">],
          void, private_sort_copy_code_vec, 
          [Macro<"BIT_WIDTH", !cast<string>(bit_width)>, Macro<"VEC_LEN", !cast<string>(vf)>,
           Macro<"CALL_FUNC", !cast<string>(scalar_spread_func_name)>, Macro<"EXT_TYPE", !cast<string>(data_type.Name)>], 
          vec_data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle>;
    }

    foreach value_type_name = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
      defvar value_bit_width = GetWidthByTypeName<value_type_name>.ret;
      defvar value_type = GetIRTypeName<value_type_name>.ret;
      defvar value_signed = GetSignedInfoByTypeName<value_type_name>.ret;
      defvar value_data_type = !cast<PointerType>("pAS4"#value_type);
      defvar value_data_mangle = !cond(!eq(type_name, value_type_name): "S0_", 
                                 true: ManglePointerType<value_data_type, /*signed*/ value_signed>.ret);
      defvar temp_mangle = !cond(!eq(type_name, "i8"): "S0_",
                                 !eq(value_type_name, "i8"): "S2_",
                                 true: ManglePointerType<temp_type, true>.ret);
      // --- key-value, scalar
      defm sort_copy_builtin # type_name # value_type_name : LLDefine<builtin, [Value<data_type, "key_ptr">,
          Value<value_data_type, "value_ptr">, Value<size_type, "per_item_number">, Value<temp_type, "dst_ptr">,
          Value<int_type, "local_id">, Value<int_type, "local_size">, Value<flag_type, "direction">], 
          void, private_key_value_sort_copy_code, 
          [Macro<"KEY_BIT_WIDTH", !cast<string>(bit_width)>, Macro<"VALUE_BIT_WIDTH", !cast<string>(value_bit_width)>],
          data_mangle # value_data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle>;

      defvar scalar_spread_func_name = "_Z" # !size(spread_builtin) # spread_builtin # 
            data_mangle # value_data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle;
      let EmitMangledName = false in
      def : LLDeclare<scalar_spread_func_name, 
            [data_type, value_data_type, size_type, temp_type, int_type, int_type, flag_type], void, "">;

      // --- key-value, vector
      foreach vf = [4, 8, 16, 32, 64] in {
        // vector arg type
        defvar vec_data_type = VectorType<data_type, vf>; // key
        defvar vec_value_data_type = VectorType<value_data_type, vf>; // value
        // generate mangle info
        defvar vec_data_mangle = MangleVectorOfPointersType<vec_data_type, signed>.ret; // key
        defvar vec_value_data_mangle = !cond(!eq(value_type_name, type_name): "S1_", 
                  true: MangleVectorOfPointersType<vec_value_data_type, value_signed>.ret); // value
        defvar value_temp_mangle_for_vec = !cond(!eq(type_name, "i8"): "S0_",
                                            !eq(value_type_name, "i8"): "S3_",    
                                            true: ManglePointerType<temp_type, true>.ret);
        defvar scalar_func_name = "_Z" # !size(builtin) # builtin # 
            data_mangle # value_data_mangle # size_mangle # temp_mangle # int_mangle # int_mangle # flag_mangle;
        // vector work group key value copy define  
        defm sort_copy_builtin # type_name # value_type_name #vf : LLDefine<builtin, [Value<vec_data_type, "key_ptr">,
            Value<vec_value_data_type, "value_ptr">, Value<size_type, "per_item_number">, Value<temp_type, "dst_ptr">,
            Value<int_type, "local_id">, Value<int_type, "local_size">, Value<flag_type, "direction">], 
            void, private_key_value_sort_copy_code_vec, 
            [Macro<"KEY_BIT_WIDTH", !cast<string>(bit_width)>, Macro<"VALUE_BIT_WIDTH", !cast<string>(value_bit_width)>,
             Macro<"VEC_LEN", !cast<string>(vf)>, Macro<"KEY_EXT_TYPE", !cast<string>(data_type.Name)>, 
             Macro<"VALUE_EXT_TYPE", !cast<string>(value_data_type.Name)>, Macro<"CALL_FUNC", !cast<string>(scalar_func_name)>],
            vec_data_mangle # vec_value_data_mangle # size_mangle # value_temp_mangle_for_vec # int_mangle # int_mangle # flag_mangle>;
        
        // vector work group key value spread copy define
        defm sort_spread_copy_builtin # type_name # value_type_name #vf : LLDefine<spread_builtin, [Value<vec_data_type, "key_ptr">,
            Value<vec_value_data_type, "value_ptr">, Value<size_type, "per_item_number">, Value<temp_type, "dst_ptr">,
            Value<int_type, "local_id">, Value<int_type, "local_size">, Value<flag_type, "direction">], 
            void, private_key_value_sort_copy_code_vec, 
            [Macro<"KEY_BIT_WIDTH", !cast<string>(bit_width)>, Macro<"VALUE_BIT_WIDTH", !cast<string>(value_bit_width)>,
             Macro<"VEC_LEN", !cast<string>(vf)>, Macro<"KEY_EXT_TYPE", !cast<string>(data_type.Name)>, 
             Macro<"VALUE_EXT_TYPE", !cast<string>(value_data_type.Name)>, 
             Macro<"CALL_FUNC", !cast<string>(scalar_spread_func_name)>],
            vec_data_mangle # vec_value_data_mangle # size_mangle # value_temp_mangle_for_vec # int_mangle # int_mangle # flag_mangle>;
      }
    }
  }
}

// ----- work group sort function vector version -----
// args will like <16 x i32 addrspace(4)*>
foreach func = ["ascending", "descending"] in {
  foreach method = ["joint_sort", "private_sort_close", "private_sort_spread"] in {
    foreach type_name = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
      defvar type = GetIRTypeName<type_name>.ret;
      defvar signed = GetSignedInfoByTypeName<type_name>.ret;
      // scalar arg type
      defvar data_type = !cast<PointerType>("pAS4"#type);
      defvar size_type = !cast<IntType>("i32");
      defvar temp_type = !cast<PointerType>("pAS4i8");
      // scalar arg type mangle info
      defvar data_mangle = ManglePointerType<data_type, signed>.ret;
      defvar size_mangle = MangleType<size_type, /*signed*/ false>.ret;
      defvar temp_mangle_for_decl = !cond(!eq(type_name, "i8"): "S0_",
                                    true: ManglePointerType<temp_type, true>.ret);
      defvar builtin = "__devicelib_default_work_group_"#method#"_"#func#"_p1"#type_name#"_u32_p1i8";
      // scalar key-only builtin declare
      defvar MangledName = "_Z" # !size(builtin) # builtin # data_mangle # size_mangle # temp_mangle_for_decl;
      let EmitMangledName = false in 
      def : LLDeclare<MangledName, [data_type, size_type, temp_type], void, "">;
      foreach value_type_name = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
        defvar kv_builtin = "__devicelib_default_work_group_"#method#"_"#func#"_p1"#type_name#"_p1"#value_type_name#"_u32_p1i8";
        defvar value_type = GetIRTypeName<value_type_name>.ret;
        defvar value_signed = GetSignedInfoByTypeName<value_type_name>.ret;
        // value scalar arg type
        defvar value_data_type = !cast<PointerType>("pAS4"#value_type);
        // value scalar arg type mangle
        defvar value_data_mangle = !cond(!eq(value_type_name, type_name): "S0_", 
                                        true: ManglePointerType<value_data_type, value_signed>.ret);
        defvar value_temp_mangle_for_decl = !cond(!eq(type_name, "i8"): "S0_",
                                          !eq(value_type_name, "i8"): "S2_",
                                          true: ManglePointerType<temp_type, true>.ret);
        // scalar key-value builtin declare
        defvar KVMangledName = "_Z" # !size(kv_builtin) # kv_builtin # data_mangle # value_data_mangle # size_mangle # value_temp_mangle_for_decl;
        let EmitMangledName = false in 
        def : LLDeclare<KVMangledName, [data_type, value_data_type, size_type, temp_type], void, "">;
      }
      // vector version
      let FuncAttrs = ["memory(readwrite)", "nounwind", "alwaysinline"] in {
        let EmitMangledName = true in 
        foreach vf = [4, 8, 16, 32, 64] in {
          // ----- key-only -----
          // vector arg type
          defvar vec_data_type = VectorType<data_type, vf>;
          // vector mask arg type
          defvar mask_type = !cast<VectorType>("v" # vf # "i32");    
          // vector arg mangle info
          defvar vec_data_mangle = MangleVectorOfPointersType<vec_data_type, signed>.ret;
          defvar mask_mangle = MangleVectorType<mask_type, /*signed*/ false>.ret;
          defvar temp_mangle_for_vec = !cond(!eq(type_name, "i8"): "S0_",      
                                        true: ManglePointerType<temp_type, true>.ret);
          // vector key-only builtin define
          defm work_group_sort # func # method # type_name # vf: LLDefine<builtin,
            [Value<vec_data_type, "first">,  Value<size_type, "n">, 
              Value<temp_type, "scratch">, Value<mask_type, "mask">], void, [{
              %vecext = extractelement {Args[0]}, i32 0
              call void @{MANGLED_FUNC}({EXT_TYPE} %vecext, {Args[1]}, {Args[2]})
              ret void
            }], [Macro<"MANGLED_FUNC", !cast<string>(MangledName)>, 
                  Macro<"EXT_TYPE", !cast<string>(data_type.Name)>], vec_data_mangle # size_mangle # temp_mangle_for_vec # mask_mangle>;
          
          // ----- key-value -----
          foreach value_type_name = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"] in {
            defvar kv_builtin = "__devicelib_default_work_group_"#method#"_"#func#"_p1"#type_name#"_p1"#value_type_name#"_u32_p1i8";
            defvar value_type = GetIRTypeName<value_type_name>.ret;
            defvar value_signed = GetSignedInfoByTypeName<value_type_name>.ret;
            // generate arg type
            defvar value_data_type = !cast<PointerType>("pAS4"#value_type);
            defvar vec_value_data_type = VectorType<value_data_type, vf>;
            // generate value mangle info
            defvar vec_value_data_mangle = !cond(!eq(value_type_name, type_name): "S1_", 
                      true: MangleVectorOfPointersType<vec_value_data_type, value_signed>.ret);

            // value scalar arg type mangle, , for call scalar sort
            defvar value_data_mangle = !cond(!eq(value_type_name, type_name): "S0_", 
                                            true: ManglePointerType<value_data_type, value_signed>.ret);
            defvar value_temp_mangle_for_decl = !cond(!eq(type_name, "i8"): "S0_",
                                                !eq(value_type_name, "i8"): "S2_",
                                                true: ManglePointerType<temp_type, true>.ret);
            defvar value_temp_mangle_for_vec = !cond(!eq(type_name, "i8"): "S0_",
                                                !eq(value_type_name, "i8"): "S3_",    
                                                true: ManglePointerType<temp_type, true>.ret);
            defvar KVMangledName = "_Z" # !size(kv_builtin) # kv_builtin # data_mangle # value_data_mangle # size_mangle # value_temp_mangle_for_decl;
            // vector key-value builtin define
            defm work_group_sort # func # method # type_name # value_type_name # vf: LLDefine<kv_builtin,
              [Value<vec_data_type, "key_first">,  Value<vec_value_data_type, "value_first">, 
                Value<size_type, "n">, Value<temp_type, "scratch">, Value<mask_type, "mask">], void, [{
                %key_vecext = extractelement {Args[0]}, i32 0
                %value_vecext = extractelement {Args[1]}, i32 0
                call void @{MANGLED_FUNC}({KEY_EXT_TYPE} %key_vecext, {VALUE_EXT_TYPE} %value_vecext, {Args[2]}, {Args[3]})
                ret void
              }], [Macro<"MANGLED_FUNC", !cast<string>(KVMangledName)>, 
                    Macro<"KEY_EXT_TYPE", !cast<string>(data_type.Name)>,
                    Macro<"VALUE_EXT_TYPE", !cast<string>(value_data_type.Name)>],
                  vec_data_mangle # vec_value_data_mangle # size_mangle # value_temp_mangle_for_vec # mask_mangle>;
          }
        }
      }
    }
  }
}


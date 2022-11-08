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

include "GENERIC/ll_generation/definition.td"

// __ocl_zext_, __ocl_sext_, _ocl_trunc_, __ocl_fpext_ and __ocl_fptrunc_
// implementations. Example output:
//
//   define i16 @__ocl_zext_v1i8_v1i16(i8 %x) #46 {
//     %1 = zext i8 %x to i16
//     ret i16 %1
//   }
//   attributes #46 = { nounwind memory(none) willreturn }

class ExtType<IntType src_type> {
  list<string> Types = !filter(t, ["i16", "i32", "i64"], !gt(!cast<IntType>(t).BitWidth, src_type.BitWidth));
}

class TruncType<IntType src_type> {
  list<string> Types = !filter(t, ["i8", "i16", "i32"], !lt(!cast<IntType>(t).BitWidth, src_type.BitWidth));
}

class FpExtType<FloatType src_type> {
  list<string> Types = !filter(t, ["f32", "f64"], !gt(!cast<FloatType>(t).BitWidth, src_type.BitWidth));
}

class FpTruncType<FloatType src_type> {
// keep using a list in filter in case we'll handling f32 as well in the future.
  list<string> Types = !filter(t, ["f16"], !lt(!cast<FloatType>(t).BitWidth, src_type.BitWidth));
}

defvar ExtOrTruncCode = [{
  %1 = {Opcode} {Args[0]} to {RetType}
  ret {RetType} %1
}];

let EmitMangledName = false,
    FuncAttrs = ["memory(none)", "willreturn", "nounwind"] in {
  foreach len = [1, 2, 3, 4, 8, 16, 32, 64] in {
    foreach src_type_name = ["i8", "i16", "i32"] in {
      defvar scalar_src_type = !cast<IntType>(src_type_name);
      defvar src_type = !if(!eq(len, 1), scalar_src_type, VectorType<scalar_src_type, len>);
      foreach dst_type_name = ExtType<scalar_src_type>.Types in {
        defvar dst_type = !if(!eq(len, 1), !cast<IntType>(dst_type_name), !cast<VectorType>("v" # len # dst_type_name));
        defvar builtin_zext = "__ocl_zext_v" # len # src_type_name # "_v" # len # dst_type_name;
        defm builtin_zext # _impl : LLDefine<builtin_zext, [Value<src_type, "x">], dst_type, ExtOrTruncCode, [Macro<"Opcode", "zext">]>;

        defvar builtin_sext = !subst("zext", "sext", builtin_zext);
        defm builtin_sext # _impl : LLDefine<builtin_sext, [Value<src_type, "x">], dst_type, ExtOrTruncCode, [Macro<"Opcode", "sext">]>;
      }
    }

    foreach src_type_name = ["i16", "i32", "i64"] in {
      defvar scalar_src_type = !cast<IntType>(src_type_name);
      defvar src_type = !if(!eq(len, 1), scalar_src_type, VectorType<scalar_src_type, len>);
      foreach dst_type_name = TruncType<scalar_src_type>.Types in {
        defvar dst_type = !if(!eq(len, 1), !cast<IntType>(dst_type_name), !cast<VectorType>("v" # len # dst_type_name));
        defvar builtin_trunc = "__ocl_trunc_v" # len # src_type_name # "_v" # len # dst_type_name;
        defm builtin_trunc # _impl : LLDefine<builtin_trunc, [Value<src_type, "x">], dst_type, ExtOrTruncCode, [Macro<"Opcode", "trunc">]>;
      }
    }

    foreach src_type_name = ["f16"] in {
      defvar scalar_src_type = !cast<FloatType>(src_type_name);
      defvar src_type = !if(!eq(len, 1), scalar_src_type, VectorType<scalar_src_type, len>);
      foreach dst_type_name = FpExtType<scalar_src_type>.Types in {
        defvar dst_type = !if(!eq(len, 1), !cast<FloatType>(dst_type_name), !cast<VectorType>("v" # len # dst_type_name));
        defvar builtin_zext = "__ocl_fpext_v" # len # src_type_name # "_v" # len # dst_type_name;
        defm builtin_zext # _impl : LLDefine<builtin_zext, [Value<src_type, "x">], dst_type, ExtOrTruncCode, [Macro<"Opcode", "fpext">]>;
      }
    }

    foreach src_type_name = ["f32", "f64"] in {
      defvar scalar_src_type = !cast<FloatType>(src_type_name);
      defvar src_type = !if(!eq(len, 1), scalar_src_type, VectorType<scalar_src_type, len>);
      foreach dst_type_name = FpTruncType<scalar_src_type>.Types in {
        defvar dst_type = !if(!eq(len, 1), !cast<FloatType>(dst_type_name), !cast<VectorType>("v" # len # dst_type_name));
        defvar builtin_trunc = "__ocl_fptrunc_v" # len # src_type_name # "_v" # len # dst_type_name;
        defm builtin_trunc # _impl : LLDefine<builtin_trunc, [Value<src_type, "x">], dst_type, ExtOrTruncCode, [Macro<"Opcode", "fptrunc">]>;
      }
    }
  } // foreach len
}

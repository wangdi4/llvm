// TableGen for intel_sub_group_block_read/write IR assume-uniform implementations
// Assume-uniform implementation is a W/A for vectorizer:
// Vectorizer fails to analyze the uniformity of the parameter, so it expects a
// variant with vector parameter.

// The implementation assumes the first widened vector-of-pointers parameter as
// uniform:
// e.g. (simply extract the 0-th element and call the uniform version builtin)
// define <4 x i32> @_Z29intel_sub_group_block_read1_4Dv4_PU3AS1KjDv4_j(<4 x i32 addrspace(1)*> %data, <4 x i32> %vec_mask) #1 {
//   %vecext = extractelement <4 x i32 addrspace(1)*> %data, i32 0
//   %1 = call <4 x i32> @_Z29intel_sub_group_block_read1_4PU3AS1KjDv4_j(i32 addrspace(1)* %vecext, <4 x i32> %vec_mask) #3
//   ret <4 x i32> %1
// }

include "GENERIC/ll_generation/definition.td"

let EmitMangledName = true in
foreach builtin = ["intel_sub_group_block_read"] in {
foreach suffix = ["", "_uc", "_us", "_ui", "_ul"] in {
  defvar base_data_type = !cond(!eq(suffix, "") : i32,
                                !eq(suffix, "_uc") : i8,
                                !eq(suffix, "_us") : i16,
                                !eq(suffix, "_ui") : i32,
                                !eq(suffix, "_ul") : i64);
  // length-16 implementation is char-specific.
  defvar lengths = !if(!eq(base_data_type, i8), [1, 2, 4, 8, 16], [1, 2, 4, 8]);
foreach block_length = lengths in {
foreach vf = [4, 8, 16, 32, 64] in {
  defvar func_name = builtin # suffix # block_length # "_" # vf;

  // Use base_data_type == i8 as example:
  // Return type: <vf*block_length x i8>
  defvar ret_type = !cast<VectorType>("v" # !mul(block_length, vf) # base_data_type);
  // The first arg: <vf x i8 addrspace(1)*> %data
  defvar uniform_data_type = !cast<PointerType>("pAS1" # base_data_type);
  defvar uniform_data_mangle = ManglePointerType<uniform_data_type, /*signed*/ false, /*mangle_suffix*/ "K">.ret; // mangle(const) == "K"
  defvar data_type = VectorType<uniform_data_type, vf>;
  defvar data_mangle = MangleVectorOfPointersType<data_type, /*signed*/ false, /*mangle_suffix*/ "K">.ret;
  // The second arg: <vf x i32> %vec_mask
  defvar mask_type = !cast<VectorType>("v" # vf # "i32"); // mask is always unsigned int
  defvar mask_mangle = MangleVectorType<mask_type, /*signed*/ false>.ret;

  defvar common_attrs = ["norecurse", "nounwind"];

  // Uniform version builtin declaration
  let FuncAttrs = common_attrs in
  def func_name # _uniform : LLDeclare<func_name, [uniform_data_type, mask_type], ret_type, uniform_data_mangle # mask_mangle>;

  // Assume-uniform version
  let FuncAttrs = common_attrs # ["readonly"] in
  defm func_name # _assume_uniform : LLDefine<func_name, [Value<data_type, "data">, Value<mask_type, "vec_mask">], ret_type, [{
  %vecext = extractelement {Args[0]}, i32 0
  %call = call {RetType} @{UniformFuncName}({UniformDataType} %vecext, {Args[1]})
  ret {RetType} %call
}], [Macro<"UniformFuncName", !cast<LLDeclare>(func_name # _uniform).MangledName>, Macro<"UniformDataType", uniform_data_type.Name>], data_mangle # mask_mangle>;

// example output
// define <32 x i8> @_Z33intel_sub_group_block_read_uc2_16Dv16_PU3AS1KhDv16_j(<16 x i8 addrspace(1)*> %data, <16 x i32> %vec_mask) norecurse nounwind readonly {
//   %vecext = extractelement <16 x i8 addrspace(1)*> %data, i32 0
//   %call = call <32 x i8> @_Z33intel_sub_group_block_read_uc2_16PU3AS1KhDv16_j(i8 addrspace(1)* %vecext, <16 x i32> %vec_mask)
//   ret <32 x i8> %call
// }
} // foreach vf
} // foreach block_length
} // foreach suffix
} // foreach builtin

let EmitMangledName = true in
foreach builtin = ["intel_sub_group_block_write"] in {
foreach suffix = ["", "_uc", "_us", "_ui", "_ul"] in {
  defvar base_data_type = !cond(!eq(suffix, "") : i32,
                                !eq(suffix, "_uc") : i8,
                                !eq(suffix, "_us") : i16,
                                !eq(suffix, "_ui") : i32,
                                !eq(suffix, "_ul") : i64);
  // length-16 implementation is char-specific.
  defvar lengths = !if(!eq(base_data_type, i8), [1, 2, 4, 8, 16], [1, 2, 4, 8]);
foreach block_length = lengths in {
foreach vf = [4, 8, 16, 32, 64] in {
  defvar func_name = builtin # suffix # block_length # "_" # vf;

  // Use base_data_type == i8 as example:
  // Return type: void
  defvar ret_type = void;
  // The first arg: <vf x i8 addrspace(1)*> %data
  defvar uniform_data_type = !cast<PointerType>("pAS1" # base_data_type);
  defvar uniform_data_mangle = ManglePointerType<uniform_data_type, /*signed*/ false>.ret;
  defvar data_type = VectorType<uniform_data_type, vf>;
  defvar data_mangle = MangleVectorOfPointersType<data_type, /*signed*/ false>.ret;
  // The second arg: <vf*block_length x i8> %src
  defvar src_type = !cast<VectorType>("v" # !mul(block_length, vf) # base_data_type);
  defvar src_mangle = MangleVectorType<src_type, /*signed*/ false>.ret;
  // The third arg: <vf x i32> %vec_mask
  defvar mask_type = !cast<VectorType>("v" # vf # "i32"); // mask is always unsigned
  // If _mask_mangle == src_mangle, then substitute with "S1_".
  // For uniform version,
  // S_ := uniform_data_mangle
  // S1_ := src_mangle
  defvar _mask_mangle = MangleVectorType<mask_type, /*signed*/ false>.ret;
  defvar uniform_mask_mangle = !if(!eq(_mask_mangle, src_mangle), "S1_", _mask_mangle);
  // If _mask_mangle == src_mangle, then substitute with "S2_".
  // For assume-uniform version,
  // S_ := mangle(data_type.PointeeType)
  // S1_ := data_mangle
  // S2_ := src_mangle
  defvar mask_mangle = !if(!eq(_mask_mangle, src_mangle), "S2_", _mask_mangle);

  defvar common_attrs = ["norecurse", "nounwind"];

  // Uniform version builtin declaration
  let FuncAttrs = common_attrs in
  def func_name # _uniform : LLDeclare<func_name, [uniform_data_type, src_type, mask_type], ret_type, uniform_data_mangle # src_mangle # uniform_mask_mangle>;

  // Assume-uniform version
  let FuncAttrs = common_attrs # ["writeonly"] in
  defm func_name # _assume_uniform : LLDefine<func_name, [Value<data_type, "data">, Value<src_type, "src">, Value<mask_type, "vec_mask">], ret_type, [{
  %vecext = extractelement {Args[0]}, i32 0
  call {RetType} @{UniformFuncName}({UniformDataType} %vecext, {Args[1]}, {Args[2]})
  ret {RetType}
}], [Macro<"UniformFuncName", !cast<LLDeclare>(func_name # _uniform).MangledName>, Macro<"UniformDataType", uniform_data_type.Name>], data_mangle # src_mangle # mask_mangle>;

// example output
// define void @_Z34intel_sub_group_block_write_uc2_16Dv16_PU3AS1hDv32_hDv16_j(<16 x i8 addrspace(1)*> %data, <32 x i8> %src, <16 x i32> %vec_mask) norecurse nounwind writeonly {
//   %vecext = extractelement <16 x i8 addrspace(1)*> %data, i32 0
//   call void @_Z34intel_sub_group_block_write_uc2_16PU3AS1hDv32_hDv16_j(i8 addrspace(1)* %vecext, <32 x i8> %src, <16 x i32> %vec_mask)
//   ret void
// }
} // foreach vf
} // foreach block_length
} // foreach suffix
} // foreach builtin


// ocl_image variants
// Define the ocl_image types first:
defset list<OpaqueStructType> ocl_image2d_types = {
  def ocl_image2d_ro : OpaqueStructType<"opencl.image2d_ro_t">;
  def ocl_image2d_wo : OpaqueStructType<"opencl.image2d_wo_t">;
  def ocl_image2d_rw : OpaqueStructType<"opencl.image2d_rw_t">;
}
// And their pointer types (global addrspace only):
defset list<PointerType> ocl_image2d_pointer_types = {
  foreach type = ocl_image2d_types in {
    def pAS1 # type : PointerType<type, GlobalAS>;
  }
}
// And vector of pointer types:
defset list<VectorType> ocl_image2d_vector_of_pointer_types = {
  foreach len = [4, 8, 16, 32, 64] in {
    foreach type = ocl_image2d_pointer_types in {
      def v # len # type : VectorType<type, len>;
    }
  }
}
// And the mangle helpers:
// e.g. mangle(%opencl.image2d_ro_t addrspace(1)*) = 14ocl_image2d_ro
class MangleOCLImagePointer<PointerType type> {
  string replace = !subst("opencl.image", "ocl_image",
                   !subst("_t", "", !cast<OpaqueStructType>(type.PointeeType).RawName));
  string ret = !size(replace) # replace;
}
// e.g. mangle(<16 x %opencl.image2d_ro_t addrspace(1)*>) = Dv16_14ocl_image2d_ro
class MangleOCLImageVectorOfPointers<VectorType type> {
  PointerType ocl_image_pointer_type = !cast<PointerType>(type.ElementType);
  string ret = "Dv" # type.Len # "_" # MangleOCLImagePointer<ocl_image_pointer_type>.ret;
}

let EmitMangledName = true in
foreach builtin = ["intel_sub_group_block_read"] in {
foreach base_img_type = [ocl_image2d_ro, ocl_image2d_rw] in {
foreach suffix = ["", "_uc", "_us", "_ui", "_ul"] in {
  defvar base_data_type = !cond(!eq(suffix, "") : i32,
                                !eq(suffix, "_uc") : i8,
                                !eq(suffix, "_us") : i16,
                                !eq(suffix, "_ui") : i32,
                                !eq(suffix, "_ul") : i64);
  // length-16 implementation is char-specific.
  defvar lengths = !if(!eq(base_data_type, i8), [1, 2, 4, 8, 16], [1, 2, 4, 8]);
foreach block_length = lengths in {
foreach vf = [4, 8, 16, 32, 64] in {
  defvar func_name = builtin # suffix # block_length # "_" # vf;

  // Use base_data_type == i8 as example:
  // Return type: <vf*block_length x i8>
  defvar ret_type = !cast<VectorType>("v" # !mul(block_length, vf) # base_data_type);
  // The first arg: <vf x %opencl.image2d_ro_t addrspace(1)*> %img
  defvar uniform_img_type = !cast<PointerType>("pAS1" # base_img_type);
  defvar uniform_img_mangle = MangleOCLImagePointer<uniform_img_type>.ret;
  defvar img_type = VectorType<uniform_img_type, vf>;
  defvar img_mangle = MangleOCLImageVectorOfPointers<img_type>.ret;
  // The second arg: <2*vf x i32> %coord
  defvar uniform_coord_type = v2i32;
  defvar uniform_coord_mangle = MangleVectorType<v2i32, /*signed*/ true>.ret;
  defvar coord_type = !cast<VectorType>("v" # !mul(2, vf) # "i32"); // coord is always signed int
  defvar coord_mangle = MangleVectorType<coord_type, /*signed*/ true>.ret;
  // The third arg: <vf x i32> %vec_mask
  defvar mask_type = !cast<VectorType>("v" # vf # "i32"); // mask is always unsigned int
  defvar mask_mangle = MangleVectorType<mask_type, /*signed*/ false>.ret;

  defvar common_attrs = ["norecurse", "nounwind"];

  // Uniform version builtin declaration
  let FuncAttrs = common_attrs in
  def func_name # base_img_type # _uniform : LLDeclare<func_name, [uniform_img_type, uniform_coord_type, mask_type], ret_type, uniform_img_mangle # uniform_coord_mangle # mask_mangle>;

  // Assume-uniform version
  let FuncAttrs = common_attrs # ["readonly"] in
  defm func_name # base_img_type # _assume_uniform : LLDefine<func_name, [Value<img_type, "img">, Value<coord_type, "coord">, Value<mask_type, "vec_mask">], ret_type, [{
  %img.vecext = extractelement {Args[0]}, i32 0
  %coord.vecext = shufflevector {Args[1]}, {CoordType} undef, <2 x i32> <i32 0, i32 1>
  %call = call {RetType} @{UniformFuncName}({UniformImgType} %img.vecext, <2 x i32> %coord.vecext, {Args[2]})
  ret {RetType} %call
}], [Macro<"CoordType", coord_type.Name>, Macro<"UniformFuncName", !cast<LLDeclare>(func_name # base_img_type # _uniform).MangledName>, Macro<"UniformImgType", uniform_img_type.Name>], img_mangle # coord_mangle # mask_mangle>;

// example output
// define <32 x i8> @_Z33intel_sub_group_block_read_uc2_16Dv16_14ocl_image2d_roDv32_iDv16_j(<16 x %opencl.image2d_ro_t addrspace(1)*> %img, <32 x i32> %coord, <16 x i32> %vec_mask) norecurse nounwind readonly {
//   %img.vecext = extractelement <16 x %opencl.image2d_ro_t addrspace(1)*> %img, i32 0
//   %coord.vecext = shufflevector <32 x i32> %coord, <32 x i32> undef, <2 x i32> <i32 0, i32 1>
//   %call = call <32 x i8> @_Z33intel_sub_group_block_read_uc2_1614ocl_image2d_roDv2_iDv16_j(%opencl.image2d_ro_t addrspace(1)* %img.vecext, <2 x i32> %coord.vecext, <16 x i32> %vec_mask)
//   ret <32 x i8> %call
// }
} // foreach vf
} // foreach block_length
} // foreach suffix
} // foreach img_type
} // foreach builtin

let EmitMangledName = true in
foreach builtin = ["intel_sub_group_block_write"] in {
foreach base_img_type = [ocl_image2d_wo, ocl_image2d_rw] in {
foreach suffix = ["", "_uc", "_us", "_ui", "_ul"] in {
  defvar base_data_type = !cond(!eq(suffix, "") : i32,
                                !eq(suffix, "_uc") : i8,
                                !eq(suffix, "_us") : i16,
                                !eq(suffix, "_ui") : i32,
                                !eq(suffix, "_ul") : i64);
  // length-16 implementation is char-specific.
  defvar lengths = !if(!eq(base_data_type, i8), [1, 2, 4, 8, 16], [1, 2, 4, 8]);
foreach block_length = lengths in {
foreach vf = [4, 8, 16, 32, 64] in {
  defvar func_name = builtin # suffix # block_length # "_" # vf;

  // Use base_data_type == i8 as example:
  // Return type: void
  defvar ret_type = void;
  // The first arg: <vf x %opencl.image2d_ro_t addrspace(1)*> %img
  defvar uniform_img_type = !cast<PointerType>("pAS1" # base_img_type);
  defvar uniform_img_mangle = MangleOCLImagePointer<uniform_img_type>.ret;
  defvar img_type = VectorType<uniform_img_type, vf>;
  defvar img_mangle = MangleOCLImageVectorOfPointers<img_type>.ret;
  // The second arg: <2*vf x i32> %coord
  defvar uniform_coord_type = v2i32;
  defvar uniform_coord_mangle = MangleVectorType<v2i32, /*signed*/ true>.ret;
  defvar coord_type = !cast<VectorType>("v" # !mul(2, vf) # "i32"); // coord is always signed int
  defvar coord_mangle = MangleVectorType<coord_type, /*signed*/ true>.ret;
  // The third type: <vf*block_length x i8>
  defvar data_type = !cast<VectorType>("v" # !mul(block_length, vf) # base_data_type);
  defvar data_mangle = MangleVectorType<data_type, /*signed*/ false>.ret;
  // The fourth arg: <vf x i32> %vec_mask
  defvar mask_type = !cast<VectorType>("v" # vf # "i32"); // mask is always unsigned int
  defvar _mask_mangle = MangleVectorType<mask_type, /*signed*/ false>.ret;
  // If _mask_mangle == data_mangle, then substitute with "S1_".
  // For uniform version,
  // S_ := uniform_coord_mangle
  // S1_ := data_mangle
  defvar uniform_mask_mangle = !if(!eq(_mask_mangle, data_mangle), "S1_", _mask_mangle);
  // If _mask_mangle == data_mangle, then substitute with "S2_".
  // For assume-uniform version,
  // S_ := img_mangle
  // S1_ := coord_mangle
  // S2_ := data_mangle
  defvar mask_mangle = !if(!eq(_mask_mangle, data_mangle), "S2_", _mask_mangle);

  defvar common_attrs = ["norecurse", "nounwind"];

  // Uniform version builtin declaration
  let FuncAttrs = common_attrs in
  def func_name # base_img_type # _uniform : LLDeclare<func_name, [uniform_img_type, uniform_coord_type, data_type, mask_type], ret_type, uniform_img_mangle # uniform_coord_mangle # data_mangle # uniform_mask_mangle>;

  // Assume-uniform version
  let FuncAttrs = common_attrs # ["writeonly"] in
  defm func_name # base_img_type # _assume_uniform : LLDefine<func_name, [Value<img_type, "img">, Value<coord_type, "coord">, Value<data_type, "data">, Value<mask_type, "vec_mask">], ret_type, [{
  %img.vecext = extractelement {Args[0]}, i32 0
  %coord.vecext = shufflevector {Args[1]}, {CoordType} undef, <2 x i32> <i32 0, i32 1>
  call {RetType} @{UniformFuncName}({UniformImgType} %img.vecext, <2 x i32> %coord.vecext, {Args[2]}, {Args[3]})
  ret {RetType}
}], [Macro<"CoordType", coord_type.Name>, Macro<"UniformFuncName", !cast<LLDeclare>(func_name # base_img_type # _uniform).MangledName>, Macro<"UniformImgType", uniform_img_type.Name>], img_mangle # coord_mangle # data_mangle # mask_mangle>;

// example output
// define void @_Z34intel_sub_group_block_write_uc2_16Dv16_14ocl_image2d_rwDv32_iDv32_hDv16_j(<16 x %opencl.image2d_rw_t addrspace(1)*> %img, <32 x i32> %coord, <32 x i8> %data, <16 x i32> %vec_mask) norecurse nounwind writeonly {
//   %img.vecext = extractelement <16 x %opencl.image2d_rw_t addrspace(1)*> %img, i32 0
//   %coord.vecext = shufflevector <32 x i32> %coord, <32 x i32> undef, <2 x i32> <i32 0, i32 1>
//   call void @_Z34intel_sub_group_block_write_uc2_1614ocl_image2d_rwDv2_iDv32_hDv16_j(%opencl.image2d_rw_t addrspace(1)* %img.vecext, <2 x i32> %coord.vecext, <32 x i8> %data, <16 x i32> %vec_mask)
//   ret void
// }
} // foreach vf
} // foreach block_length
} // foreach suffix
} // foreach img_type
} // foreach builtin

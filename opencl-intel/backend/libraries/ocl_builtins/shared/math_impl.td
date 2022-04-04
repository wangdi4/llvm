// sincos linear strided vector impl
foreach is_native = [true, false] in {
  defvar prefix = !if(is_native, "native_", "");
  foreach is_masked = [true, false] in {
    defvar suffix = "_strided" # !if(is_masked, "_mask", "");
    foreach name = ["sincos_pas", "sincos_las", "sincos_gas", "sincos_gen"] in {
      defvar builtin = !cast<OclBuiltin>(prefix # name # suffix);
      defvar impl_code = !if(is_masked, ret2ptr_strided_mask_code, ret2ptr_strided_code);

      def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
    } // foreach name

    // sincos_ret2ptr
    defvar builtin = !cast<OclBuiltin>(prefix # "sincos_pas2" # suffix);
    defvar impl_code = !if(is_masked, ret2ptr2_strided_mask_code, ret2ptr2_strided_code);

    def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/false, impl_code, [["$FUNC", "sincos"]]>;
  } // foreach is_masked
} // foreach is_native

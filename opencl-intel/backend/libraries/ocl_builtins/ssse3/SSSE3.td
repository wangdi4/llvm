// vim:ts=2:sw=2:et:

include "GENERIC/GENERIC_CPU.td"


// auxiliary functions used in atomic.td
let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE, INLINE],
    NeedForwardDecl = 1  in {
    def ocl20_add : OclBuiltin<"__ocl20_add", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_sub : OclBuiltin<"__ocl20_sub", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_or  : OclBuiltin<"__ocl20_or",  (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_xor : OclBuiltin<"__ocl20_xor", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_and : OclBuiltin<"__ocl20_and", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_min : OclBuiltin<"__ocl20_min", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
    def ocl20_max : OclBuiltin<"__ocl20_max", (outs gentype:$res), (ins gentype:$lhs, gentype:$rhs), [], [], 1, 0>;
}

class SSSE3 : Generic {
// Native types for SSSE3. Used in "Promote" calls
let NativeTypes = [v16i8, v16u8, v8i16, v8u16, v4i32, v4u32, v2i64, v2u64, v4f32, v2f64];

code Prolog = [{
    #define ALIGN16 __attribute__((aligned(16)))
    #define ALIGN32 __attribute__((aligned(32)))

    #define __OPENCL__
    #include <intrin.h>
    #include "generic-builtin-defines.h"
    #include "cpu-builtin-defines.h"
    #ifdef __x86_64__
      #include "svml_ia32e.h"
    #else
      #include "svml_ia32.h"
    #endif
    // new type for the implementation of the transpose functions
    #if defined(__AVX__)
        typedef __v32qi ocl_char32;
        ocl_char32 INLINE_ATTRIBUTE as_ocl_char32(int8 x) {
            return __builtin_astype(x, ocl_char32);
        }
    #endif
}];

include "ssse3/common.td"
include "ssse3/math.td"
include "ssse3/integers.td"
include "ssse3/geometric.td"
include "ssse3/atomic.td"
include "ssse3/misc.td"
include "ssse3/conversion.td"
include "ssse3/conversion_sat.td"
include "ssse3/vloadstore.td"
include "ssse3/relational.td"
include "ssse3/transpose.td"
include "ssse3/masked_load_store.td"
}

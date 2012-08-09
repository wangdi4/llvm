// vim:ts=2:sw=2:et:

include "GENERIC.td"

def m8 : OclType<"m8", "uchar", 1, 8, 0>;
def m16 : OclType<"m16", "ushort", 1, 16, 0>;

// masktype: 'v8xM' -> 'm8', 'v16xM' -> 'm16'
def masktype: OclGenType<"masktype"> {
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "", n));
  list<string> T5 = !foreach(n,  T4, !subst("u32", "", n));
  list<string> T6 = !foreach(n,  T5, !subst("i64", "", n));
  list<string> T7 = !foreach(n,  T6, !subst("u64", "", n));
  list<string> T8 = !foreach(n,  T7, !subst("f16", "", n));
  list<string> T9 = !foreach(n,  T8, !subst("f32", "", n));
  list<string> Ta = !foreach(n,  T9, !subst("f64", "", n));
  list<string> Tb = !foreach(n,  Ta, !subst("v16", "m16", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8",  "m8", n));
  list<string> Td = !foreach(n,  Tc, !subst( "v4",  "m4", n));
  list<string> Te = !foreach(n,  Td, !subst( "v3",  "m3", n));
  list<string> Tf = !foreach(n,  Te, !subst( "v2",  "m2", n));
  list<string> Tg = !foreach(n,  Tf, !subst( "v1",  "m1", n));
  list<string> Tout = Tg;
}

// math builtins
let Types = [v16f32, v8f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_fmax : OclBuiltin<"mask_fmax", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_fmin : OclBuiltin<"mask_fmin", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_mad  : OclBuiltin<"mask_mad",  (outs gentype:$ret), (ins masktype:$mask, gentype:$a, gentype:$b, gentype:$c), [], []>;
}

// lprefetch builtin
def lprefetch : OclBuiltin<"lprefetch", (outs), (ins pgentype:$ptr, size_t:$numElements, size_t:$elmSize), [v1i8], []>;

// Memory Fence Functions
def mem_fence : OclBuiltin<"mem_fence", (outs), (ins size_t:$flags), [v1i64], []>;
def read_mem_fence : OclBuiltin<"read_mem_fence", (outs), (ins size_t:$flags), [v1i64], []>;
def write_mem_fence : OclBuiltin<"write_mem_fence", (outs), (ins size_t:$flags), [v1i64], []>;


// native math builtins
let Types = [v16f32, v8f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_native_fdim    : OclBuiltin<"mask_native_fdim",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_native_fmax    : OclBuiltin<"mask_native_fmax",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_native_fmin    : OclBuiltin<"mask_native_fmin",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_native_fmod    : OclBuiltin<"mask_native_fmod",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_native_ilogb   : OclBuiltin<"mask_native_ilogb",   (outs i32gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
def mask_native_logb    : OclBuiltin<"mask_native_logb",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
}

// integer builtins
let Types = [v16u8, v16u16, v16u32, v8u64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_popcount : OclBuiltin<"mask_popcount", (outs gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
}

// common builtins
let Types = [v16f32, v8f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_max : OclBuiltin<"mask_max", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_min : OclBuiltin<"mask_min", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], []>;
def mask_mix : OclBuiltin<"mask_mix", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y, gentype:$a), [], []>;
}

// vload/vstore builtins
let Types = [ v16i8,  v16u8, v16i16, v16u16,
             v16i32, v16u32, v16f32,
              v8i64,  v8u64,  v8f64],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_vload_gas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def mask_vload_cas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def mask_vload_las  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def mask_vload_pas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
def mask_vstore_pas : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstore_las : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstore_gas : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
}

// vload_half/vloada_half
let Types = [v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vload_half
def mask_vload_half_gas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def mask_vload_half_cas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def mask_vload_half_las     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def mask_vload_half_pas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
// vloada_half
def mask_vloada_half_gas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def mask_vloada_half_cas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def mask_vloada_half_las    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def mask_vloada_half_pas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
}
// vstore_half/vstorea_half on float16
let Types = [v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vstore_half
def mask_vstoref_half_pas       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoref_half_las       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoref_half_gas       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rte
def mask_vstoref_half_rte_pas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoref_half_rte_las   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoref_half_rte_gas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtz
def mask_vstoref_half_rtz_pas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoref_half_rtz_las   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoref_half_rtz_gas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtp
def mask_vstoref_half_rtp_pas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoref_half_rtp_las   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoref_half_rtp_gas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtn
def mask_vstoref_half_rtn_pas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoref_half_rtn_las   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoref_half_rtn_gas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half
def mask_vstoreaf_half_pas      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoreaf_half_las      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoreaf_half_gas      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rte
def mask_vstoreaf_half_rte_pas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoreaf_half_rte_las  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoreaf_half_rte_gas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtz
def mask_vstoreaf_half_rtz_pas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoreaf_half_rtz_las  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoreaf_half_rtz_gas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtp
def mask_vstoreaf_half_rtp_pas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoreaf_half_rtp_las  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoreaf_half_rtp_gas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtn
def mask_vstoreaf_half_rtn_pas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoreaf_half_rtn_las  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoreaf_half_rtn_gas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
}
// vstore_half/vstorea_half on double8
let Types = [v8f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vstore_half
def mask_vstored_half_pas   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstored_half_las   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstored_half_gas   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half
def mask_vstoread_half_pas  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoread_half_las  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoread_half_gas  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rte
def mask_vstored_half_rte_pas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstored_half_rte_las   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstored_half_rte_gas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtz
def mask_vstored_half_rtz_pas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstored_half_rtz_las   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstored_half_rtz_gas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtp
def mask_vstored_half_rtp_pas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstored_half_rtp_las   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstored_half_rtp_gas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstore_half_rtn
def mask_vstored_half_rtn_pas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstored_half_rtn_las   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstored_half_rtn_gas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rte
def mask_vstoread_half_rte_pas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoread_half_rte_las  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoread_half_rte_gas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtz
def mask_vstoread_half_rtz_pas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoread_half_rtz_las  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoread_half_rtz_gas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtp
def mask_vstoread_half_rtp_pas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoread_half_rtp_las  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoread_half_rtp_gas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
// vstorea_half_rtn
def mask_vstoread_half_rtn_pas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def mask_vstoread_half_rtn_las  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def mask_vstoread_half_rtn_gas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
}

// conversions
let Types = [v8f32],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_convert_f64 : OclBuiltin<"mask_convert_$ReturnType", (outs f64gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
}

// any/all
let Types = [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_any : OclBuiltin<"mask_any", (outs si32gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
def mask_all : OclBuiltin<"mask_all", (outs si32gentype:$ret), (ins masktype:$mask, gentype:$x), [], []>;
}

class KNC : Generic {

  list<OclType> NativeTypes = [v16f32, v8f64, v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v8i64, v8u64];


  code Prolog = [{
  #if defined (__MIC__) || defined(__MIC2__)

  #ifdef __cplusplus
  extern "C" {
  #endif

  #pragma OPENCL EXTENSION cl_khr_fp64 : enable

  #include <intrin.h>

  #include "generic-builtin-defines.h"
  #include "knc-builtin-defines.h"
  #include "mic_dev_limits.h"
  }];

  code Epilog = [{
  #pragma OPENCL EXTENSION cl_khr_fp64 : disable

  #ifdef __cplusplus
  }
  #endif
  #endif // defined (__MIC__) || defined(__MIC2__)
  }];

  code NonMaskPromote1 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }];

  code NonMaskPromote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];

  code NonMaskPromote3 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName);
    return $ReturnVarName$Pattern;
  }];

  code MaskPromote1 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeReturnType $ReturnVarName = mask_$NativeFunc($Mask, n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }];

  code MaskPromote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = mask_$NativeFunc($Mask, n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];

  code MaskPromote3 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeReturnType $ReturnVarName = mask_$NativeFunc($Mask, n$Arg0VarName, n$Arg1VarName, n$Arg2VarName);
    return $ReturnVarName$Pattern;
  }];

  code ScalarReturnPromote1 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName);
    return $ReturnVarName;
  }];

  code ScalarReturnPromote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $ReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName;
  }];

  code ScalarReturnPromote3 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg1VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $ReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName);
    return $ReturnVarName;
  }];

include "MIC/math.td"
include "MIC/integer.td"
include "MIC/common.td"
include "MIC/geometry.td"
include "MIC/relational.td"
include "MIC/vloadstore.td"
include "MIC/misc.td"
include "MIC/conversion.td"
include "MIC/conversion_sat.td"
}

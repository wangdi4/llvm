// vim:ts=2:sw=2:et:

include "GENERIC/GENERIC_MIC.td"

def m8 : OclType<"m8", "uchar", 1, 8, 0>;
def m16 : OclType<"m16", "ushort", 1, 16, 0>;

// masktype: 'v8xM' -> 'm8', 'v16xM' -> 'm16'
def masktype: OclGenType<"masktype"> {
  list<string> Tin = Torig;
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

// replace all vectors' length with v16
class SubSt_v16<list<string> IN>
{
  string n; // used in foreach
  list<string> R1  = !foreach(n,  IN, !subst( "v8",   "v16", n));
  list<string> R2  = !foreach(n,  R1, !subst( "v4",   "v16", n));
  list<string> R3  = !foreach(n,  R2, !subst( "v3",   "v16", n));
  list<string> R4  = !foreach(n,  R3, !subst( "v2",   "v16", n));
  list<string> R5  = !foreach(n,  R4, !subst( "v1",   "v16", n));
  list<string> res = !foreach(n,  R5, !subst( "v166", "v16", n));
}

// replace 8 and 16-bit integers with 32-bit
class SubSt_ui32<list<string> IN>
{
  string n; // used in foreach
  list<string> R1  = !foreach(n,  IN, !subst( "u8",   "u32", n));
  list<string> R2  = !foreach(n,  R1, !subst( "u16",  "u32", n));
  list<string> R3  = !foreach(n,  R2, !subst( "i8",   "i32", n));
  list<string> res = !foreach(n,  R3, !subst( "i16",  "i32", n));
}

// remove vector length
class SubSt_delVecLength<list<string> IN>
{
  string n; // used in foreach
  list<string> T1   = !foreach(n, IN, !subst("v16", "", n));
  list<string> T2   = !foreach(n, T1, !subst("v1", "", n));
  list<string> T3   = !foreach(n, T2, !subst("v2", "", n));
  list<string> T4   = !foreach(n, T3, !subst("v3", "", n));
  list<string> T5   = !foreach(n, T4, !subst("v4", "", n));
  list<string> res  = !foreach(n, T5, !subst("v8", "", n));
}

// remove integer width
class SubSt_delUIWidth<list<string> IN>
{
  string n; // used in foreach
  list<string> T1  = !foreach(n, IN, !subst("u8",  "u", n));
  list<string> T2  = !foreach(n, T1, !subst("u16", "u", n));
  list<string> T3  = !foreach(n, T2, !subst("u32", "u", n));
  list<string> T4  = !foreach(n, T3, !subst("u64", "u", n));
  list<string> T5  = !foreach(n, T4, !subst("i8",  "i", n));
  list<string> T6  = !foreach(n, T5, !subst("i16", "i", n));
  list<string> T7  = !foreach(n, T6, !subst("i32", "i", n));
  list<string> res = !foreach(n, T7, !subst("i64", "i", n));
}

// expand integer type to 32-bit values
def ui32gentype : OclGenType<"ui32gentype"> {
  list<string> Tin  = Torig;
  list<string> Tout = SubSt_ui32<ocl_types.Names>.res;
}

// generate v16<u|i>32  type
def v16ui32gentype : OclGenType<"v16ui32gentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_ui32<ocl_types.Names>.res;
  list<string> Tout = SubSt_v16<T1>.res;
}

// generate "epu32", "epi32", "epu64","epi64" suffixes
// Can only be used with $ArgXSym macro
def epuigentype : OclGenType<"epuigentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_delVecLength<Tin>.res;
  list<string> T2   = !foreach(n, T1, !subst("u",  "epu", n));
  list<string> T3   = !foreach(n, T2, !subst("i",  "epi", n));
  list<string> T4   = !foreach(n, T3, !subst("8",  "32", n));
  list<string> Tout = !foreach(n, T4, !subst("16", "32", n));
}

// generate short or char mask 
// Can only be used with $ArgXSym macro
def maskgentype: OclGenType<"maskgentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_delVecLength<Tin>.res;
  list<string> T2   = !foreach(n, T1,  !subst("i32", "short", n));
  list<string> T3   = !foreach(n, T2,  !subst("u32", "short", n));
  list<string> T4   = !foreach(n, T3,  !subst("f32", "short", n));
  list<string> T5   = !foreach(n, T4,  !subst("i64", "char", n));
  list<string> T6   = !foreach(n, T5,  !subst("u64", "char", n));
  list<string> Tout = !foreach(n, T6,  !subst("f64", "char", n));
}

// generate "ps" and "pd" suffixes
// Can only be used with $ArgXSym macro
def pspdgentype : OclGenType<"pspdgentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_delVecLength<Tin>.res;
  list<string> T2   = !foreach(n, T1,  !subst("f32", "ps", n));
  list<string> Tout = !foreach(n, T2,  !subst("f64", "pd", n));
}
 
// generate "__m512", "__m512d", and "__m512i" types
// Can only be used with $ArgXSym macro
def m512gentype : OclGenType<"m512gentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_delVecLength<Tin>.res;
  list<string> T2   = SubSt_delUIWidth<T1>.res;
  list<string> T3   = !foreach(n, T2,  !subst("p",   "", n));
  list<string> T4   = !foreach(n, T3,  !subst("f32", "__m512", n));
  list<string> T5   = !foreach(n, T4,  !subst("f64", "__m512d", n));
  list<string> T6   = !foreach(n, T5,  !subst("u", "__m512i", n));
  list<string> Tout = !foreach(n, T6,  !subst("i", "__m512i", n));
}

// generate MIC native types from any type
// Can only be used with $ArgXSym macro
def micnativegentype : OclGenType<"micnativegentype"> {
  list<string> Tin  = Torig;
  list<string> T1   = SubSt_delVecLength<Tin>.res;
  list<string> T2   = !foreach(n, T1,  !subst("p",   "", n));
  list<string> T3   = !foreach(n, T2,  !subst("f32", "float16", n));
  list<string> T4   = !foreach(n, T3,  !subst("f64", "double8", n));
  list<string> T5   = !foreach(n, T4,  !subst("u8",  "uchar16", n));
  list<string> T6   = !foreach(n, T5,  !subst("i8",  "char16", n));
  list<string> T7   = !foreach(n, T6,  !subst("u16", "ushort16", n));
  list<string> T8   = !foreach(n, T7,  !subst("i16", "short16", n));
  list<string> T9   = !foreach(n, T8,  !subst("u32", "uint16", n));
  list<string> T10  = !foreach(n, T9,  !subst("i32", "int16", n));
  list<string> T11  = !foreach(n, T10, !subst("u64", "ulong8", n));
  list<string> Tout = !foreach(n, T11, !subst("i64", "long8", n));
}

// lprefetch builtin
def lprefetch : OclBuiltin<"lprefetch", (outs), (ins pgentype:$ptr, size_t:$numElements, size_t:$elmSize), [v1i8], [], 0, 0>;


// native math builtins
let Types = [v16f32, v8f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_native_fdim    : OclBuiltin<"mask_native_fdim",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_native_fmax    : OclBuiltin<"mask_native_fmax",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_native_fmin    : OclBuiltin<"mask_native_fmin",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_native_fmod    : OclBuiltin<"mask_native_fmod",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_native_ilogb   : OclBuiltin<"mask_native_ilogb",   (outs i32gentype:$ret), (ins masktype:$mask, gentype:$x), [], [], 0, 0>;
def mask_native_logb    : OclBuiltin<"mask_native_logb",    (outs gentype:$ret), (ins masktype:$mask, gentype:$x), [], [], 0, 0>;
}

// integer builtins
let Types = [v16u8, v16u16, v16u32, v8u64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_popcount : OclBuiltin<"mask_popcount", (outs gentype:$ret), (ins masktype:$mask, gentype:$x), [], [], 0, 0>;
}

// common builtins
let Types = [v16f32, v8f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_max : OclBuiltin<"mask_max", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_min : OclBuiltin<"mask_min", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y), [], [], 0, 0>;
def mask_mix : OclBuiltin<"mask_mix", (outs gentype:$ret), (ins masktype:$mask, gentype:$x, gentype:$y, gentype:$a), [], [], 0, 0>;
}

// vload/vstore builtins
let Types = [ v16i8,  v16u8, v16i16, v16u16,
             v16i32, v16u32, v16f32,
              v8i64,  v8u64,  v8f64],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_vload_gas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def mask_vload_cas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def mask_vload_las  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def mask_vload_pas  : OclBuiltin<"mask_vload",  (outs gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
def mask_vstore_pas : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstore_las : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstore_gas : OclBuiltin<"mask_vstore", (outs), (ins masktype:$mask, gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}

// vload_half/vloada_half
let Types = [v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vload_half
def mask_vload_half_gas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def mask_vload_half_cas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def mask_vload_half_las     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def mask_vload_half_pas     : OclBuiltin<"mask_vload_half",  (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
// vloada_half
def mask_vloada_half_gas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def mask_vloada_half_cas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def mask_vloada_half_las    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def mask_vloada_half_pas    : OclBuiltin<"mask_vloada_half", (outs up_gentype:$ret), (ins masktype:$mask, size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
}
// vstore_half/vstorea_half on float16
let Types = [v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vstore_half
def mask_vstoref_half_pas       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_las       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_gas       : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rte
def mask_vstoref_half_rte_pas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_rte_las   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_rte_gas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtz
def mask_vstoref_half_rtz_pas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_rtz_las   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_rtz_gas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtp
def mask_vstoref_half_rtp_pas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_rtp_las   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_rtp_gas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtn
def mask_vstoref_half_rtn_pas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_rtn_las   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_rtn_gas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_final
def mask_vstoref_half_final_pas : OclBuiltin<"mask_vstore_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoref_half_final_las : OclBuiltin<"mask_vstore_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoref_half_final_gas : OclBuiltin<"mask_vstore_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half
def mask_vstoreaf_half_pas      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_las      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_gas      : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rte
def mask_vstoreaf_half_rte_pas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_rte_las  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_rte_gas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtz
def mask_vstoreaf_half_rtz_pas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_rtz_las  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_rtz_gas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtp
def mask_vstoreaf_half_rtp_pas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_rtp_las  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_rtp_gas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtn
def mask_vstoreaf_half_rtn_pas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_rtn_las  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_rtn_gas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_final
def mask_vstoreaf_half_final_pas : OclBuiltin<"mask_vstorea_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoreaf_half_final_las : OclBuiltin<"mask_vstorea_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoreaf_half_final_gas : OclBuiltin<"mask_vstorea_half_final", (outs), (ins masktype:$mask, up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}
// vstore_half/vstorea_half on double8
let Types = [v8f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
// vstore_half
def mask_vstored_half_pas   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstored_half_las   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstored_half_gas   : OclBuiltin<"mask_vstore_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half
def mask_vstoread_half_pas  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoread_half_las  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoread_half_gas  : OclBuiltin<"mask_vstorea_half", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rte
def mask_vstored_half_rte_pas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstored_half_rte_las   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstored_half_rte_gas   : OclBuiltin<"mask_vstore_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtz
def mask_vstored_half_rtz_pas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstored_half_rtz_las   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstored_half_rtz_gas   : OclBuiltin<"mask_vstore_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtp
def mask_vstored_half_rtp_pas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstored_half_rtp_las   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstored_half_rtp_gas   : OclBuiltin<"mask_vstore_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstore_half_rtn
def mask_vstored_half_rtn_pas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstored_half_rtn_las   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstored_half_rtn_gas   : OclBuiltin<"mask_vstore_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rte
def mask_vstoread_half_rte_pas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoread_half_rte_las  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoread_half_rte_gas  : OclBuiltin<"mask_vstorea_half_rte", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtz
def mask_vstoread_half_rtz_pas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoread_half_rtz_las  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoread_half_rtz_gas  : OclBuiltin<"mask_vstorea_half_rtz", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtp
def mask_vstoread_half_rtp_pas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoread_half_rtp_las  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoread_half_rtp_gas  : OclBuiltin<"mask_vstorea_half_rtp", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
// vstorea_half_rtn
def mask_vstoread_half_rtn_pas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def mask_vstoread_half_rtn_las  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def mask_vstoread_half_rtn_gas  : OclBuiltin<"mask_vstorea_half_rtn", (outs), (ins masktype:$mask, up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}

// any/all
let Types = [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def mask_any : OclBuiltin<"mask_any", (outs si32gentype:$ret), (ins masktype:$mask, gentype:$x), [], [], 0, 0>;
def mask_all : OclBuiltin<"mask_all", (outs si32gentype:$ret), (ins masktype:$mask, gentype:$x), [], [], 0, 0>;
}

class KNC : Generic {

  let NativeTypes = [v16f32, v8f64, v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v8i64, v8u64];


  code Prolog = [{
    #if defined (__MIC__) || defined(__MIC2__)

    #pragma OPENCL EXTENSION cl_khr_fp64 : enable

    #define __OPENCL__
    #include <intrin.h>

    #include "generic-builtin-defines.h"
    #include "knc-builtin-defines.h"
    #include "mic_dev_limits.h"
  }];

  code Epilog = [{
    #endif // defined (__MIC__) || defined(__MIC2__)
  }];

  code NonMaskPromote1 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $ReturnBaseType$NativeArg0VecLength $ReturnVarName = $NativeFunc(n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }];

  code NonMaskPromote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $Arg1BaseType$NativeArg0VecLength n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $ReturnBaseType$NativeArg0VecLength $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];

  code NonMaskPromote3 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $Arg1BaseType$NativeArg0VecLength n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $Arg2BaseType$NativeArg0VecLength n$Arg2VarName; n$Arg2VarName$Pattern = $Arg2VarName;
    $ReturnBaseType$NativeArg0VecLength $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName, n$Arg2VarName);
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

list<OclType> float_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

include "KNCni/atomic.td"
include "KNCni/common.td"
include "KNCni/conversion.td"
include "KNCni/conversion_sat.td"
include "KNCni/geometry.td"
include "KNCni/integer.td"
include "KNCni/math.td"
include "KNCni/misc.td"
include "KNCni/relational.td"
include "KNCni/vloadstore.td"

}

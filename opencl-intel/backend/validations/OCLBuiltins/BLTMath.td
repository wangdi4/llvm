// vim:ts=2:sw=2:et:

include "ref-opencl-builtin.td"
include "BLTRef.td"

// OpenCL 1.2 rev 15, Table 6.8

let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def acos          : REF_OclBuiltin<"acos",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def acosh         : REF_OclBuiltin<"acosh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def acospi        : REF_OclBuiltin<"acospi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def asin          : REF_OclBuiltin<"asin",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def asinh         : REF_OclBuiltin<"asinh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def asinpi        : REF_OclBuiltin<"asinpi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def atan          : REF_OclBuiltin<"atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], [], MathTemplate>;
def atan2         : REF_OclBuiltin<"atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], MathTemplate>;
def atanh         : REF_OclBuiltin<"atanh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def atanpi        : REF_OclBuiltin<"atanpi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def atan2pi       : REF_OclBuiltin<"atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], MathTemplate>;
def cbrt          : REF_OclBuiltin<"cbrt",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def ceil          : REF_OclBuiltin<"ceil",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def copysign      : REF_OclBuiltin<"copysign",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def cos           : REF_OclBuiltin<"cos",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def cosh          : REF_OclBuiltin<"cosh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def cospi         : REF_OclBuiltin<"cospi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
// functions erf and erfc are not supported by RefALU and NEAT so far, turned off
// def erfc          : REF_OclBuiltin<"erfc",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
// def erf           : REF_OclBuiltin<"erf",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def exp           : REF_OclBuiltin<"exp",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def exp2          : REF_OclBuiltin<"exp2",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def exp10         : REF_OclBuiltin<"exp10",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def expm1         : REF_OclBuiltin<"expm1",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def fabs          : REF_OclBuiltin<"fabs",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def fdim          : REF_OclBuiltin<"fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def floor         : REF_OclBuiltin<"floor",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def fma           : REF_OclBuiltin<"fma",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], MathTemplate>;
def fmax          : REF_OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def fmax_s        : REF_OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def fmin          : REF_OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def fmin_s        : REF_OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def fmod          : REF_OclBuiltin<"fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def fract_pas     : REF_OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def fract_las     : REF_OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def fract_gas     : REF_OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def fract_gnas    : REF_OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def frexp_pas     : REF_OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def frexp_las     : REF_OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def frexp_gas     : REF_OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def frexp_gnas    : REF_OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def hypot         : REF_OclBuiltin<"hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def ilogb         : REF_OclBuiltin<"ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def ldexp         : REF_OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$k), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def ldexp_s       : REF_OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, si32gentype:$k), [], [], TemplateAttr< [type0, vec0, vec1]> >, Except<[v1f32, v1f64]>;
def lgamma        : REF_OclBuiltin<"lgamma",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def lgamma_r_pas  : REF_OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def lgamma_r_las  : REF_OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def lgamma_r_gas  : REF_OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def lgamma_r_gnas : REF_OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def log           : REF_OclBuiltin<"log",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def log2          : REF_OclBuiltin<"log2",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def log10         : REF_OclBuiltin<"log10",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def log1p         : REF_OclBuiltin<"log1p",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def logb          : REF_OclBuiltin<"logb",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def mad           : REF_OclBuiltin<"mad",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], MathTemplate>;
def maxmag        : REF_OclBuiltin<"maxmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def minmag        : REF_OclBuiltin<"minmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def modf_pas      : REF_OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def modf_las      : REF_OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def modf_gas      : REF_OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def modf_gnas     : REF_OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def nan           : REF_OclBuiltin<"nan",       (outs gentype:$ret), (ins ugentype:$x), [], [], TemplateAttr< [OclBuiltinAttr<"u_float">, type0, vec0]> >, Except<[v1f64, v2f64, v3f64, v4f64, v8f64, v16f64]>;
def nan_s         : REF_OclBuiltin<"nan",       (outs gentype:$ret), (ins ugentype:$x), [], [], TemplateAttr< [OclBuiltinAttr<"u_double">, type0, vec0]> >, Except<[v1f32, v2f32, v3f32, v4f32, v8f32, v16f32]>;
def nextafter     : REF_OclBuiltin<"nextafter", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def pow           : REF_OclBuiltin<"pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def pown          : REF_OclBuiltin<"pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], MathTemplate>;
def powr          : REF_OclBuiltin<"powr",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def remainder     : REF_OclBuiltin<"remainder", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def remquo_pas    : REF_OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def remquo_las    : REF_OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def remquo_gas    : REF_OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def remquo_gnas   : REF_OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def rint          : REF_OclBuiltin<"rint",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def rootn         : REF_OclBuiltin<"rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], MathTemplate>;
def round         : REF_OclBuiltin<"round",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def rsqrt         : REF_OclBuiltin<"rsqrt",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def sin           : REF_OclBuiltin<"sin",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def sincos_pas    : REF_OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def sincos_las    : REF_OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def sincos_gas    : REF_OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def sincos_gnas   : REF_OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def sinh          : REF_OclBuiltin<"sinh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def sinpi         : REF_OclBuiltin<"sinpi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def sqrt          : REF_OclBuiltin<"sqrt",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def tan           : REF_OclBuiltin<"tan",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def tanh          : REF_OclBuiltin<"tanh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def tanpi         : REF_OclBuiltin<"tanpi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
// function tgamma is not supported by RefALU and NEAT so far, turned off
// def tgamma        : REF_OclBuiltin<"tgamma",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def trunc         : REF_OclBuiltin<"trunc",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}

// OpenCL 1.2 rev 15, Table 6.9
// half
// Spec requires Data type to be ONLY float(f32) (not-double)
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def half_cos      : REF_OclBuiltin<"half_cos",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_divide   : REF_OclBuiltin<"half_divide",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def half_exp      : REF_OclBuiltin<"half_exp",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_exp2     : REF_OclBuiltin<"half_exp2",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_exp10    : REF_OclBuiltin<"half_exp10",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_log      : REF_OclBuiltin<"half_log",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_log2     : REF_OclBuiltin<"half_log2",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_log10    : REF_OclBuiltin<"half_log10",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_powr     : REF_OclBuiltin<"half_powr",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def half_recip    : REF_OclBuiltin<"half_recip",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_rsqrt    : REF_OclBuiltin<"half_rsqrt",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_sin      : REF_OclBuiltin<"half_sin",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_sqrt     : REF_OclBuiltin<"half_sqrt",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def half_tan      : REF_OclBuiltin<"half_tan",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}

// OpenCL 1.2 rev 15, Table 6.9
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
// native
// Spec requires Data type to be ONLY float(f32) (not-double)
// Double support is not in OpenCL 1.2 spec. It is Intel defined extension of spec
def native_cos    : REF_OclBuiltin<"native_cos",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_divide : REF_OclBuiltin<"native_divide", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_exp    : REF_OclBuiltin<"native_exp",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_exp2   : REF_OclBuiltin<"native_exp2",   (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_exp10  : REF_OclBuiltin<"native_exp10",  (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_log    : REF_OclBuiltin<"native_log",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_log2   : REF_OclBuiltin<"native_log2",   (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_log10  : REF_OclBuiltin<"native_log10",  (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_powr   : REF_OclBuiltin<"native_powr",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_recip  : REF_OclBuiltin<"native_recip",  (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_rsqrt  : REF_OclBuiltin<"native_rsqrt",  (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_sin    : REF_OclBuiltin<"native_sin",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_sqrt   : REF_OclBuiltin<"native_sqrt",   (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_tan    : REF_OclBuiltin<"native_tan",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
let NeedForwardDecl = 1 in { /* work around the missing prototype in opencl.h */
// extened native
def native_acos          : REF_OclBuiltin<"native_acos",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_acosh         : REF_OclBuiltin<"native_acosh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_acospi        : REF_OclBuiltin<"native_acospi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_asin          : REF_OclBuiltin<"native_asin",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_asinh         : REF_OclBuiltin<"native_asinh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_asinpi        : REF_OclBuiltin<"native_asinpi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_atan          : REF_OclBuiltin<"native_atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], [], MathTemplate>;
def native_atan2         : REF_OclBuiltin<"native_atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], MathTemplate>;
def native_atanh         : REF_OclBuiltin<"native_atanh",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_atanpi        : REF_OclBuiltin<"native_atanpi",    (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_atan2pi       : REF_OclBuiltin<"native_atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], MathTemplate>;
def native_cbrt          : REF_OclBuiltin<"native_cbrt",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_cosh          : REF_OclBuiltin<"native_cosh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_cospi         : REF_OclBuiltin<"native_cospi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
// functions erf and erfc are not supported by RefALU and NEAT so far, native version turned off as well
// def native_erfc          : REF_OclBuiltin<"native_erfc",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
// def native_erf           : REF_OclBuiltin<"native_erf",       (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_expm1         : REF_OclBuiltin<"native_expm1",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_fdim          : REF_OclBuiltin<"native_fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_fmax          : REF_OclBuiltin<"native_fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def native_fmin          : REF_OclBuiltin<"native_fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], TemplateAttr< [type0, vec0, vec1]> >;
def native_fmod          : REF_OclBuiltin<"native_fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_fract_pas     : REF_OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE; // MIC
def native_fract_las     : REF_OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL; // MIC
def native_fract_gas     : REF_OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL; // MIC
def native_fract_gnas    : REF_OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC; // MIC
def native_hypot         : REF_OclBuiltin<"native_hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_ilogb         : REF_OclBuiltin<"native_ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_log1p         : REF_OclBuiltin<"native_log1p",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_logb          : REF_OclBuiltin<"native_logb",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_pow           : REF_OclBuiltin<"native_pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], MathTemplate>;
def native_pown          : REF_OclBuiltin<"native_pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], MathTemplate>;
// function rcbrt is not supported by RefALU and NEAT so far, turned off
// def native_rcbrt         : REF_OclBuiltin<"native_rcbrt",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_rootn         : REF_OclBuiltin<"native_rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], MathTemplate>;
def native_sincos_pas    : REF_OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_sincos_las    : REF_OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def native_sincos_gas    : REF_OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def native_sincos_gnas   : REF_OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], MathTemplate>, ExceptAttr<[CONSTFUNC]>, GENERIC;
def native_sinh          : REF_OclBuiltin<"native_sinh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_sinpi         : REF_OclBuiltin<"native_sinpi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_tanh          : REF_OclBuiltin<"native_tanh",      (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
def native_tanpi         : REF_OclBuiltin<"native_tanpi",     (outs gentype:$ret), (ins gentype:$x), [], [], MathTemplate>;
}
}

class Generic {
  code Prolog = [{
/*****************************************************************************\

Copyright (C) 2013 Intel Corporation

This software and the related documents are Intel copyrighted materials, and
your use of them is governed by the express license under which they were
provided to you ("License"). Unless the License provides otherwise, you may
not use, modify, copy, publish, distribute, disclose or transmit this
software or the related documents without Intel's prior written permission.

This software and the related documents are provided as is, with no express
or implied warranties, other than those that are expressly stated in the
License.

File Name:  BLTMath.cpp

\*****************************************************************************/

#include "BLTMath.h"
#include "llvm/Support/DynamicLibrary.h"

using namespace llvm;
using std::string;
using std::vector;
using namespace Validation::OCLBuiltins;

#ifndef BUILTINS_API
   #if defined(_WIN32)
      #define BUILTINS_API __declspec(dllexport)
   #else
      #define BUILTINS_API
   #endif
#endif

extern "C" {
  }];

  code EpilogInit = [{
    BUILTINS_API void initOCLBuiltinsMath() {
  }];

  code Epilog = [{
    }

} // extern "C"
  }];

}
  def inc : Generic;

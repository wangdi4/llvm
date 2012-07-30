// vim:ts=2:sw=2:et:

// OpenCL 1.2 rev 15, Table 6.8
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def acos          : OclBuiltin<"acos",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def acosh         : OclBuiltin<"acosh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def acospi        : OclBuiltin<"acospi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def asin          : OclBuiltin<"asin",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def asinh         : OclBuiltin<"asinh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def asinpi        : OclBuiltin<"asinpi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def atan          : OclBuiltin<"atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], []>;
def atan2         : OclBuiltin<"atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], []>;
def atanh         : OclBuiltin<"atanh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def atanpi        : OclBuiltin<"atanpi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def atan2pi       : OclBuiltin<"atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], []>;
def cbrt          : OclBuiltin<"cbrt",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def ceil          : OclBuiltin<"ceil",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def copysign      : OclBuiltin<"copysign",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def cos           : OclBuiltin<"cos",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def cosh          : OclBuiltin<"cosh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def cospi         : OclBuiltin<"cospi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def erfc          : OclBuiltin<"erfc",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def erf           : OclBuiltin<"erf",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def exp           : OclBuiltin<"exp",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def exp2          : OclBuiltin<"exp2",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def exp10         : OclBuiltin<"exp10",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def expm1         : OclBuiltin<"expm1",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def fabs          : OclBuiltin<"fabs",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def fdim          : OclBuiltin<"fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def floor         : OclBuiltin<"floor",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def fma           : OclBuiltin<"fma",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def fmax          : OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def fmax_s        : OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], []>, Except<[v1f32, v1f64]>;
def fmin          : OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def fmin_s        : OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], []>, Except<[v1f32, v1f64]>;
def fmod          : OclBuiltin<"fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def fract_pas     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def fract_las     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def fract_gas     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def frexp_pas     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def frexp_las     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def frexp_gas     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def hypot         : OclBuiltin<"hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def ilogb         : OclBuiltin<"ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def ldexp         : OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$k), [], []>;
def ldexp_s       : OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, si32gentype:$k), [], []>, Except<[v1f32, v1f64]>;
def lgamma        : OclBuiltin<"lgamma",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def lgamma_r_pas  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def lgamma_r_las  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def lgamma_r_gas  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def log           : OclBuiltin<"log",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def log2          : OclBuiltin<"log2",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def log10         : OclBuiltin<"log10",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def log1p         : OclBuiltin<"log1p",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def logb          : OclBuiltin<"logb",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def mad           : OclBuiltin<"mad",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def maxmag        : OclBuiltin<"maxmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def minmag        : OclBuiltin<"minmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def modf_pas      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def modf_las      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def modf_gas      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def nan           : OclBuiltin<"nan",       (outs gentype:$ret), (ins ugentype:$x), [], []>;
def nextafter     : OclBuiltin<"nextafter", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def pow           : OclBuiltin<"pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def pown          : OclBuiltin<"pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def powr          : OclBuiltin<"powr",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def remainder     : OclBuiltin<"remainder", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def remquo_pas    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def remquo_las    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def remquo_gas    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def rint          : OclBuiltin<"rint",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def rootn         : OclBuiltin<"rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def round         : OclBuiltin<"round",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def rsqrt         : OclBuiltin<"rsqrt",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def sin           : OclBuiltin<"sin",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def sincos_pas    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def sincos_las    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def sincos_gas    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def sinh          : OclBuiltin<"sinh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def sinpi         : OclBuiltin<"sinpi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def sqrt          : OclBuiltin<"sqrt",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def tan           : OclBuiltin<"tan",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def tanh          : OclBuiltin<"tanh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def tanpi         : OclBuiltin<"tanpi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def tgamma        : OclBuiltin<"tgamma",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def trunc         : OclBuiltin<"trunc",     (outs gentype:$ret), (ins gentype:$x), [], []>;
}


// OpenCL 1.2 rev 15, Table 6.9
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
// half
def half_cos      : OclBuiltin<"half_cos",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_divide   : OclBuiltin<"half_divide",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def half_exp      : OclBuiltin<"half_exp",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_exp2     : OclBuiltin<"half_exp2",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_exp10    : OclBuiltin<"half_exp10",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_log      : OclBuiltin<"half_log",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_log2     : OclBuiltin<"half_log2",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_log10    : OclBuiltin<"half_log10",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_powr     : OclBuiltin<"half_powr",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def half_recip    : OclBuiltin<"half_recip",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_rsqrt    : OclBuiltin<"half_rsqrt",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_sin      : OclBuiltin<"half_sin",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_sqrt     : OclBuiltin<"half_sqrt",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def half_tan      : OclBuiltin<"half_tan",      (outs gentype:$ret), (ins gentype:$x), [], []>;
}

// OpenCL 1.2 rev 15, Table 6.9
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
// native
def native_cos    : OclBuiltin<"native_cos",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_divide : OclBuiltin<"native_divide", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_exp    : OclBuiltin<"native_exp",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_exp2   : OclBuiltin<"native_exp2",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_exp10  : OclBuiltin<"native_exp10",  (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_log    : OclBuiltin<"native_log",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_log2   : OclBuiltin<"native_log2",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_log10  : OclBuiltin<"native_log10",  (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_powr   : OclBuiltin<"native_powr",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_recip  : OclBuiltin<"native_recip",  (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_rsqrt  : OclBuiltin<"native_rsqrt",  (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_sin    : OclBuiltin<"native_sin",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_sqrt   : OclBuiltin<"native_sqrt",   (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_tan    : OclBuiltin<"native_tan",    (outs gentype:$ret), (ins gentype:$x), [], []>;
let NeedForwardDecl = 1 in { /* work around the missing prototype in opencl.h */
// extened native
def native_acos          : OclBuiltin<"native_acos",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_acosh         : OclBuiltin<"native_acosh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_acospi        : OclBuiltin<"native_acospi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_asin          : OclBuiltin<"native_asin",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_asinh         : OclBuiltin<"native_asinh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_asinpi        : OclBuiltin<"native_asinpi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_atan          : OclBuiltin<"native_atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], []>;
def native_atan2         : OclBuiltin<"native_atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], []>;
def native_atanh         : OclBuiltin<"native_atanh",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_atanpi        : OclBuiltin<"native_atanpi",    (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_atan2pi       : OclBuiltin<"native_atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], []>;
def native_cbrt          : OclBuiltin<"native_cbrt",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_cosh          : OclBuiltin<"native_cosh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_cospi         : OclBuiltin<"native_cospi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_erfc          : OclBuiltin<"native_erfc",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_erf           : OclBuiltin<"native_erf",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_expm1         : OclBuiltin<"native_expm1",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_fdim          : OclBuiltin<"native_fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_fmax          : OclBuiltin<"native_fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_fmin          : OclBuiltin<"native_fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_fmod          : OclBuiltin<"native_fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_fract_pas     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE; // MIC
def native_fract_las     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL; // MIC
def native_fract_gas     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL; // MIC
def native_hypot         : OclBuiltin<"native_hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_ilogb         : OclBuiltin<"native_ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def native_log1p         : OclBuiltin<"native_log1p",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_logb          : OclBuiltin<"native_logb",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_pow           : OclBuiltin<"native_pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def native_pown          : OclBuiltin<"native_pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def native_rcbrt         : OclBuiltin<"native_rcbrt",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_rootn         : OclBuiltin<"native_rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], []>;
def native_sincos_pas    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_sincos_las    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def native_sincos_gas    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], []>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def native_sinh          : OclBuiltin<"native_sinh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_sinpi         : OclBuiltin<"native_sinpi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_tanh          : OclBuiltin<"native_tanh",      (outs gentype:$ret), (ins gentype:$x), [], []>;
def native_tanpi         : OclBuiltin<"native_tanpi",     (outs gentype:$ret), (ins gentype:$x), [], []>;
}
}

// OpenCL 1.2 rev 15, Table 6.10
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def abs       : OclBuiltin<"abs",       (outs ugentype:$ret), (ins gentype:$x), [], []>;
def abs_diff  : OclBuiltin<"abs_diff",  (outs ugentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def add_sat   : OclBuiltin<"add_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def hadd      : OclBuiltin<"hadd",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def rhadd     : OclBuiltin<"rhadd",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def clz       : OclBuiltin<"clz",       (outs gentype:$ret), (ins gentype:$x), [], []>;
def mad_hi    : OclBuiltin<"mad_hi",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def mad_sat   : OclBuiltin<"mad_sat",   (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def mul_hi    : OclBuiltin<"mul_hi",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def rotate    : OclBuiltin<"rotate",    (outs gentype:$ret), (ins gentype:$v, gentype:$i), [], []>;
def sub_sat   : OclBuiltin<"sub_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def upsample  : OclBuiltin<"upsample",  (outs up_gentype:$ret), (ins gentype:$hi, ugentype:$lo), [], []>,
                Except<[v1i64, v2i64, v3i64, v4i64, v8i64, v16i64, v1u64, v2u64, v3u64, v4u64, v8u64, v16u64]>;
let NeedForwardDecl = 1 in /* work around the missing prototype in opencl.h */
def popcount  : OclBuiltin<"popcount",  (outs gentype:$ret), (ins gentype:$x), [], []>;
}
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
             v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def clamp   : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, gentype:$minval, gentype:$maxval), [], []>;
def clamp_s : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, sgentype:$minval, sgentype:$maxval), [], []>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def max     : OclBuiltin<"max",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def max_s   : OclBuiltin<"max",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], []>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def min     : OclBuiltin<"min",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def min_s   : OclBuiltin<"min",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], []>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
}


// OpenCL 1.2 rev 15, Table 6.11
let Types = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def mad24   : OclBuiltin<"mad24", (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$z), [], []>;
def mul24   : OclBuiltin<"mul24", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
}

// OpenCL 1.2 rev 15, Table 6.12
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def degrees       : OclBuiltin<"degrees",     (outs gentype:$ret), (ins gentype:$radians), [], []>;
def mix           : OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$a), [], []>;
def mix_s         : OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, sgentype:$a), [], []>, Except<[v1f32, v1f64]>;
def radians       : OclBuiltin<"radians",     (outs gentype:$ret), (ins gentype:$degrees), [], []>;
def step          : OclBuiltin<"step",        (outs gentype:$ret), (ins gentype:$edge, gentype:$x), [], []>;
def step_s        : OclBuiltin<"step",        (outs gentype:$ret), (ins sgentype:$edge, gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def smoothstep    : OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins gentype:$edge0, gentype:$edge1, gentype:$x), [], []>;
def smoothstep_s  : OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins sgentype:$edge0, sgentype:$edge1, gentype:$x), [], []>, Except<[v1f32, v1f64]>;
def sign          : OclBuiltin<"sign",        (outs gentype:$ret), (ins gentype:$x), [], []>;
}

// OpenCL 1.2 rev 15, Table 6.13
let Types = [v1f32, v2f32, v3f32, v4f32,
             v1f64, v2f64, v3f64, v4f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def cross     : OclBuiltin<"cross",     (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>,
                Except<[v1f32, v2f32, v1f64, v2f64]>;
def dot       : OclBuiltin<"dot",       (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def distance  : OclBuiltin<"distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def length    : OclBuiltin<"length",    (outs sgentype:$ret), (ins gentype:$p), [], []>;
def normalize : OclBuiltin<"normalize", (outs gentype:$ret), (ins gentype:$p), [], []>;
}
let Types = [v1f32, v2f32, v3f32, v4f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def fast_distance  : OclBuiltin<"fast_distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def fast_length    : OclBuiltin<"fast_length",    (outs sgentype:$ret), (ins gentype:$p), [], []>;
def fast_normalize : OclBuiltin<"fast_normalize", (outs gentype:$ret), (ins gentype:$p), [], []>;
}

// OpenCL 1.2 rev 15, Table 6.14
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def isequal         : OclBuiltin<"isequal",         (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isnotequal      : OclBuiltin<"isnotequal",      (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isgreater       : OclBuiltin<"isgreater",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isgreaterequal  : OclBuiltin<"isgreaterequal",  (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isless          : OclBuiltin<"isless",          (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def islessequal     : OclBuiltin<"islessequal",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def islessgreater   : OclBuiltin<"islessgreater",   (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isfinite        : OclBuiltin<"isfinite",        (outs rf2igentype:$ret), (ins gentype:$x), [], []>;
def isinf           : OclBuiltin<"isinf",           (outs rf2igentype:$ret), (ins gentype:$x), [], []>;
def isnan           : OclBuiltin<"isnan",           (outs rf2igentype:$ret), (ins gentype:$x), [], []>;
def isnormal        : OclBuiltin<"isnormal",        (outs rf2igentype:$ret), (ins gentype:$x), [], []>;
def isordered       : OclBuiltin<"isordered",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isunordered     : OclBuiltin<"isunordered",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def signbit         : OclBuiltin<"signbit",         (outs rf2igentype:$ret), (ins gentype:$x), [], []>;
}
let Types = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def any : OclBuiltin<"any", (outs si32gentype:$ret), (ins gentype:$x), [], []>;
def all : OclBuiltin<"all", (outs si32gentype:$ret), (ins gentype:$x), [], []>;
}
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
             v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def bitselect   : OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], []>;
def iselect     : OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c), [], []>;
def uselect     : OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c), [], []>;
}

// OpenCL 1.2 rev 15, Table 6.15
let Types = [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v2i16, v3i16, v4i16, v8i16, v16i16,
             v2u16, v3u16, v4u16, v8u16, v16u16,
             v2i32, v3i32, v4i32, v8i32, v16i32,
             v2u32, v3u32, v4u32, v8u32, v16u32,
             v2i64, v3i64, v4i64, v8i64, v16i64,
             v2u64, v3u64, v4u64, v8u64, v16u64,
             v2f32, v3f32, v4f32, v8f32, v16f32,
             v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [OVERLOADABLE] in {
def vload_gas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def vload_cas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def vload_las   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def vload_pas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
def vstore_pas  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstore_las  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstore_gas  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
}
let Types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE] in {
def vload_half_gas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def vload_half_cas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def vload_half_las        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def vload_half_pas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
def vstoref_half_pas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoref_half_las      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoref_half_gas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoref_half_rte_pas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoref_half_rte_las  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoref_half_rte_gas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoref_half_rtz_pas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoref_half_rtz_las  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoref_half_rtz_gas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoref_half_rtp_pas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoref_half_rtp_las  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoref_half_rtp_gas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoref_half_rtn_pas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoref_half_rtn_las  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoref_half_rtn_gas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstored_half_pas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstored_half_las      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstored_half_gas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstored_half_rte_pas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstored_half_rte_las  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstored_half_rte_gas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstored_half_rtz_pas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstored_half_rtz_las  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstored_half_rtz_gas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstored_half_rtp_pas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstored_half_rtp_las  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstored_half_rtp_gas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstored_half_rtn_pas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstored_half_rtn_las  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstored_half_rtn_gas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vloada_half_gas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_GLOBAL;
def vloada_half_cas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONSTANT;
def vloada_half_las       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_LOCAL;
def vloada_half_pas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], []>, CONST_PRIVATE;
def vstoreaf_half_pas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoreaf_half_las     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoreaf_half_gas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoreaf_half_rte_pas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoreaf_half_rte_las : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoreaf_half_rte_gas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoreaf_half_rtz_pas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoreaf_half_rtz_las : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoreaf_half_rtz_gas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoreaf_half_rtp_pas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoreaf_half_rtp_las : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoreaf_half_rtp_gas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoreaf_half_rtn_pas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoreaf_half_rtn_las : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoreaf_half_rtn_gas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoread_half_pas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoread_half_las     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoread_half_gas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoread_half_rte_pas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoread_half_rte_las : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoread_half_rte_gas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoread_half_rtz_pas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoread_half_rtz_las : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoread_half_rtz_gas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoread_half_rtp_pas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoread_half_rtp_las : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoread_half_rtp_gas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
def vstoread_half_rtn_pas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, PRIVATE;
def vstoread_half_rtn_las : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, LOCAL;
def vstoread_half_rtn_gas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], []>, GLOBAL;
}

// OpenCL 1.2 rev 15, Table 6.20
let Types = [ v2i8,  v4i8,  v8i8,  v16i8,
              v2u8,  v4u8,  v8u8,  v16u8,
             v2i16, v4i16, v8i16, v16i16,
             v2u16, v4u16, v8u16, v16u16,
             v2i32, v4i32, v8i32, v16i32,
             v2u32, v4u32, v8u32, v16u32,
             v2i64, v4i64, v8i64, v16i64,
             v2u64, v4u64, v8u64, v16u64,
             v2f32, v4f32, v8f32, v16f32,
             v2f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def shuffle_2   : OclBuiltin<"shuffle",   (outs gentype2:$ret), (ins gentype:$x, ugentype2:$mask), [], []>;
def shuffle_4   : OclBuiltin<"shuffle",   (outs gentype4:$ret), (ins gentype:$x, ugentype4:$mask), [], []>;
def shuffle_8   : OclBuiltin<"shuffle",   (outs gentype8:$ret), (ins gentype:$x, ugentype8:$mask), [], []>;
def shuffle_16  : OclBuiltin<"shuffle",   (outs gentype16:$ret), (ins gentype:$x, ugentype16:$mask), [], []>;
def shuffle2_2  : OclBuiltin<"shuffle2",  (outs gentype2:$ret), (ins gentype:$x, gentype:$y, ugentype2:$mask), [], []>;
def shuffle2_4  : OclBuiltin<"shuffle2",  (outs gentype4:$ret), (ins gentype:$x, gentype:$y, ugentype4:$mask), [], []>;
def shuffle2_8  : OclBuiltin<"shuffle2",  (outs gentype8:$ret), (ins gentype:$x, gentype:$y, ugentype8:$mask), [], []>;
def shuffle2_16 : OclBuiltin<"shuffle2",  (outs gentype16:$ret), (ins gentype:$x, gentype:$y, ugentype16:$mask), [], []>;
}

// OpenCL 1.2 rev 15, Section 6.2.3 Explicit Conversions
//
// destType convert_destType<_sat><_roundingMode>(sourceType)
//
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64,
             v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
// conversion to integer has satured mode.
def convert_u8          : OclBuiltin<"convert_$ReturnType",         (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8          : OclBuiltin<"convert_$ReturnType",         (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16         : OclBuiltin<"convert_$ReturnType",         (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16         : OclBuiltin<"convert_$ReturnType",         (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32         : OclBuiltin<"convert_$ReturnType",         (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32         : OclBuiltin<"convert_$ReturnType",         (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64         : OclBuiltin<"convert_$ReturnType",         (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64         : OclBuiltin<"convert_$ReturnType",         (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_sat      : OclBuiltin<"convert_$ReturnType_sat",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_sat      : OclBuiltin<"convert_$ReturnType_sat",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
// conversions from floating to integer also honor rounding mode as well as saturated mode.
let ExceptionTypes = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                       v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                      v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                      v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                      v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                      v1u64, v2u64, v3u64, v4u64, v8u64, v16u64] in {
def convert_u8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_i8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_i16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_i32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_u64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def convert_i64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
}
// conversion to floating may not has saturated mode.
def convert_f32         : OclBuiltin<"convert_$ReturnType",         (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def convert_f32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def convert_f32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def convert_f32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def convert_f32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def convert_f64         : OclBuiltin<"convert_$ReturnType",         (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def convert_f64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def convert_f64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def convert_f64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def convert_f64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs f64gentype:$ret), (ins gentype:$x), [], []>;
// the following conversions make no sense but are added for completeness.
//
// conversions from int to integer with rounding mode and/or saturated mode.
let ExceptionTypes = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                      v1f64, v2f64, v3f64, v4f64, v8f64, v16f64] in {
def _convert_u8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_u64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_i64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
}
// conversion to floating make nosense with saturated mode.
def _convert_f32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f64gentype:$ret), (ins gentype:$x), [], []>;
def _convert_f64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f64gentype:$ret), (ins gentype:$x), [], []>;
}

// Extended OpenCL built-in for SOA Geometry functions
let Types = [v16f32, v8f64, v4f32, v8f32, v4f64, v16f64],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def soa_dot1           : OclBuiltin<"soa_dot1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def soa_dot2           : OclBuiltin<"soa_dot2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], []>;
def soa_dot3           : OclBuiltin<"soa_dot3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], []>;
def soa_dot4           : OclBuiltin<"soa_dot4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], []>;

def soa_length1        : OclBuiltin<"soa_length1",   (outs gentype:$ret), (ins gentype:$p0), [], []>;
def soa_length2        : OclBuiltin<"soa_length2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y), [], []>;
def soa_length3        : OclBuiltin<"soa_length3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z), [], []>;
def soa_length4        : OclBuiltin<"soa_length4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w), [], []>;

def soa_distance1     : OclBuiltin<"soa_distance1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def soa_distance2     : OclBuiltin<"soa_distance2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], []>;
def soa_distance3     : OclBuiltin<"soa_distance3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], []>;
def soa_distance4     : OclBuiltin<"soa_distance4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], []>;

def soa_cross3           : OclBuiltin<"soa_cross3", (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], []>;
def soa_cross4           : OclBuiltin<"soa_cross4", (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], []>;

def soa_normalize1       : OclBuiltin<"soa_normalize1",   (outs gentype:$ret), (ins gentype:$p0), [], []>;
def soa_normalize2       : OclBuiltin<"soa_normalize2",   (outs), (ins gentype:$p0_x, gentype:$p0_y, pgentype:$res_x, pgentype:$res_y), [], []>;
def soa_normalize3       : OclBuiltin<"soa_normalize3",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], []>;
def soa_normalize4       : OclBuiltin<"soa_normalize4",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], []>;
}
let Types = [v16f32, v4f32, v8f32],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def soa_fast_length1   : OclBuiltin<"soa_fast_length1",   (outs gentype:$ret), (ins gentype:$p0), [], []>;
def soa_fast_length2   : OclBuiltin<"soa_fast_length2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y), [], []>;
def soa_fast_length3   : OclBuiltin<"soa_fast_length3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z), [], []>;
def soa_fast_length4   : OclBuiltin<"soa_fast_length4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w), [], []>;

def soa_fast_distance1 : OclBuiltin<"soa_fast_distance1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], []>;
def soa_fast_distance2 : OclBuiltin<"soa_fast_distance2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], []>;
def soa_fast_distance3 : OclBuiltin<"soa_fast_distance3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], []>;
def soa_fast_distance4 : OclBuiltin<"soa_fast_distance4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], []>;

def soa_fast_normalize1       : OclBuiltin<"soa_fast_normalize1",   (outs gentype:$ret), (ins gentype:$p0), [], []>;
def soa_fast_normalize2       : OclBuiltin<"soa_fast_normalize2",   (outs), (ins gentype:$p0_x, gentype:$p0_y, pgentype:$res_x, pgentype:$res_y), [], []>;
def soa_fast_normalize3       : OclBuiltin<"soa_fast_normalize3",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], []>;
def soa_fast_normalize4       : OclBuiltin<"soa_fast_normalize4",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], []>;
}

// OpenCL 1.2 rev 15, Section 6.12.11 Explicit Conversions
//
// Atomic Functions 
let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def atomic_add_global : OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_add_local  : OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_sub_global : OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_sub_local  : OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_inc_global : OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_GLOBAL;
def atomic_inc_local  : OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_LOCAL;
def atomic_dec_global : OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_GLOBAL;
def atomic_dec_local  : OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_LOCAL;
def atomic_cmpxchg_global : OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_cmpxchg_local  : OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_min_global : OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_min_local  : OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_max_global : OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_max_local  : OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_and_global : OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_and_local  : OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_or_global : OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_or_local  : OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atomic_xor_global : OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_xor_local  : OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def atomic_xchg_global : OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atomic_xchg_local  : OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def atom_add_vol_global     : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_add_global         : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_sub_vol_global     : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_sub_global         : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_inc_vol_global     : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_GLOBAL;
def atom_inc_global         : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, GLOBAL;
def atom_dec_vol_global     : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_GLOBAL;
def atom_dec_global         : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, GLOBAL;
def atom_cmpxchg_vol_global : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_cmpxchg_global     : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, GLOBAL;
def atom_min_vol_global     : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_min_global         : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_max_vol_global     : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_max_global         : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_and_vol_global     : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_and_global         : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_or_vol_global      : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_or_global          : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_xor_vol_global     : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_xor_global         : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;

def atom_add_vol_local     : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_add_local         : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_sub_vol_local     : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_sub_local         : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_cmpxchg_vol_local : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_cmpxchg_local     : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], []>, LOCAL;
def atom_inc_vol_local     : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_LOCAL;
def atom_inc_local         : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], []>, LOCAL;
def atom_dec_vol_local     : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, VOLATILE_LOCAL;
def atom_dec_local         : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], []>, LOCAL;
def atom_min_vol_local     : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_min_local         : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_max_vol_local     : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_max_local         : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_and_vol_local     : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_and_local         : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_or_vol_local      : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_or_local          : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
def atom_xor_vol_local     : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_xor_local         : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE], 
    NeedForwardDecl = 1 in {
def atom_xchg_vol_global    : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_GLOBAL;
def atom_xchg_global        : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, GLOBAL;
def atom_xchg_vol_local    : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, VOLATILE_LOCAL;
def atom_xchg_local        : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], []>, LOCAL;
}

// SOA versions of the any/all relational builtins
// 6 versions are required for each of the two builtins, since
// the number of parameters depends on the original vector width
// which can be any one of 1,2,3,4,8,16.
// There doesn't seem to be a way to generate this without
// a change to the tablegen backend.
let Types = [v4i8,  v8i8,  v16i8,
             v4i16, v8i16, v16i16,
             v4i32, v8i32, v16i32,
             v4i64, v8i64, v16i64],
    Attrs = [OVERLOADABLE],
	NeedForwardDecl = 1	in {
def soa_any1 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0), [], []>;
def soa_any2 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1), [], []>;
def soa_any3 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, gentype:$x2), [], []>;
def soa_any4 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, 
	gentype:$x2, gentype:$x3), [], []>;
def soa_any8 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, 
	 gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5, 
	 gentype:$x6, gentype:$x7), [], []>;
def soa_any16 : OclBuiltin<"soa_any", (outs i32gentype:$ret), (ins gentype:$x0,  gentype:$x1, 
	 gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5, 
	 gentype:$x6, gentype:$x7, gentype:$x8, gentype:$x9, 
	 gentype:$x10, gentype:$x11, gentype:$x12, gentype:$x13, 
	 gentype:$x14, gentype:$x15), [], []>;
def soa_all1 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0), [], []>;
def soa_all2 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1), [], []>;
def soa_all3 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, gentype:$x2), [], []>;
def soa_all4 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, 
	gentype:$x2, gentype:$x3), [], []>;
def soa_all8 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, 
	 gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5, 
	 gentype:$x6, gentype:$x7), [], []>;
def soa_all16 : OclBuiltin<"soa_all", (outs i32gentype:$ret), (ins gentype:$x0,  gentype:$x1, 
	 gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5, 
	 gentype:$x6, gentype:$x7, gentype:$x8, gentype:$x9, 
	 gentype:$x10, gentype:$x11, gentype:$x12, gentype:$x13, 
	 gentype:$x14, gentype:$x15), [], []>;
}

// Helper built-ins
// This function is incroduced to overcome issues with division operation on MIC target.
// WARNING! Do not use it! Your performance could be harmed.
// This built-in is expected to be removed at WW35'12.
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def division      : OclBuiltin<"division",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
}

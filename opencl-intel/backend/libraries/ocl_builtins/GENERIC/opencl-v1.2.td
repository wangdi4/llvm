// vim:ts=2:sw=2:et:

// OpenCL 1.2 rev 15, Table 6.8
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def acos          : OclBuiltin<"acos",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def acosh         : OclBuiltin<"acosh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def acospi        : OclBuiltin<"acospi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def asin          : OclBuiltin<"asin",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def asinh         : OclBuiltin<"asinh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def asinpi        : OclBuiltin<"asinpi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def atan          : OclBuiltin<"atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], [], 0, 1>;
def atan2         : OclBuiltin<"atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], 0, 1>;
def atanh         : OclBuiltin<"atanh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def atanpi        : OclBuiltin<"atanpi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def atan2pi       : OclBuiltin<"atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], 0, 0>;
def cbrt          : OclBuiltin<"cbrt",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def ceil          : OclBuiltin<"ceil",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def copysign      : OclBuiltin<"copysign",  (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def cos           : OclBuiltin<"cos",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def cosh          : OclBuiltin<"cosh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def cospi         : OclBuiltin<"cospi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def erfc          : OclBuiltin<"erfc",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def erf           : OclBuiltin<"erf",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def exp           : OclBuiltin<"exp",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def exp2          : OclBuiltin<"exp2",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def exp10         : OclBuiltin<"exp10",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def expm1         : OclBuiltin<"expm1",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def fabs          : OclBuiltin<"fabs",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def fdim          : OclBuiltin<"fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def floor         : OclBuiltin<"floor",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def fma           : OclBuiltin<"fma",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], 0, 1>;
def fmax          : OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def fmax_common   : OclBuiltin<"fmax_common", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def fmax_s        : OclBuiltin<"fmax",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 1>, Except<[v1f32, v1f64]>;
def fmin          : OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def fmin_common   : OclBuiltin<"fmin_common", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def fmin_s        : OclBuiltin<"fmin",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 1>, Except<[v1f32, v1f64]>;
def fmod          : OclBuiltin<"fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def fract_pas     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def fract_las     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def fract_gas     : OclBuiltin<"fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def fract_pas2    : OclBuiltin<"fract_ret2ptr",    (outs), (ins gentype:$x, pgentype:$ret0 ,pgentype:$ret1), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def frexp_pas     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def frexp_las     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def frexp_gas     : OclBuiltin<"frexp",     (outs gentype:$ret), (ins gentype:$x, pi32gentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def hypot         : OclBuiltin<"hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def ilogb         : OclBuiltin<"ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def ldexp         : OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$k), [], [], 0, 1>;
def ldexp_s       : OclBuiltin<"ldexp",     (outs gentype:$ret), (ins gentype:$x, si32gentype:$k), [], [], 0, 1>, Except<[v1f32, v1f64]>;
def lgamma        : OclBuiltin<"lgamma",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def lgamma_r_pas  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def lgamma_r_las  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def lgamma_r_gas  : OclBuiltin<"lgamma_r",  (outs gentype:$ret), (ins gentype:$x, pi32gentype:$signp), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def log           : OclBuiltin<"log",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def log2          : OclBuiltin<"log2",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def log10         : OclBuiltin<"log10",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def log1p         : OclBuiltin<"log1p",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def logb          : OclBuiltin<"logb",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def mad           : OclBuiltin<"mad",       (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], 0, 0>;
def maxmag        : OclBuiltin<"maxmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def minmag        : OclBuiltin<"minmag",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def modf_pas      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def modf_las      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def modf_gas      : OclBuiltin<"modf",      (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def modf_pas2    : OclBuiltin<"modf_ret2ptr",    (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def nan           : OclBuiltin<"nan",       (outs gentype:$ret), (ins ugentype:$x), [], [], 0, 1>;
def nextafter     : OclBuiltin<"nextafter", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def pow           : OclBuiltin<"pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def pown          : OclBuiltin<"pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], 0, 1>;
def powr          : OclBuiltin<"powr",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def remainder     : OclBuiltin<"remainder", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
def remquo_pas    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def remquo_las    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def remquo_gas    : OclBuiltin<"remquo",    (outs gentype:$ret), (ins gentype:$x, gentype:$y, pi32gentype:$quo), [], [], 0, 1>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def rint          : OclBuiltin<"rint",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def rootn         : OclBuiltin<"rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], 0, 0>;
def round         : OclBuiltin<"round",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def rsqrt         : OclBuiltin<"rsqrt",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def sin           : OclBuiltin<"sin",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def sincos_pas    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def sincos_las    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def sincos_gas    : OclBuiltin<"sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def sincos_pas2    : OclBuiltin<"sincos_ret2ptr",    (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def sinh          : OclBuiltin<"sinh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def sinpi         : OclBuiltin<"sinpi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def sqrt          : OclBuiltin<"sqrt",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def tan           : OclBuiltin<"tan",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def tanh          : OclBuiltin<"tanh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def tanpi         : OclBuiltin<"tanpi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def tgamma        : OclBuiltin<"tgamma",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def trunc         : OclBuiltin<"trunc",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 1>;
}

// OpenCL 1.2 rev 15, Table 6.9
// half
// Spec requires Data type to be ONLY float(f32) (not-double)
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def half_cos      : OclBuiltin<"half_cos",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_divide   : OclBuiltin<"half_divide",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def half_exp      : OclBuiltin<"half_exp",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_exp2     : OclBuiltin<"half_exp2",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_exp10    : OclBuiltin<"half_exp10",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_log      : OclBuiltin<"half_log",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_log2     : OclBuiltin<"half_log2",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_log10    : OclBuiltin<"half_log10",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_powr     : OclBuiltin<"half_powr",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def half_recip    : OclBuiltin<"half_recip",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_rsqrt    : OclBuiltin<"half_rsqrt",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_sin      : OclBuiltin<"half_sin",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_sqrt     : OclBuiltin<"half_sqrt",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def half_tan      : OclBuiltin<"half_tan",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// OpenCL 1.2 rev 15, Table 6.9
// native
// Spec requires Data type to be ONLY float(f32) (not-double)
// Double support is not in OpenCL 1.2 spec. It is Intel defined extension of spec
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
let NeedForwardDecl = 1 in { /* work around the missing prototype in opencl.h */
def native_cos    : OclBuiltin<"native_cos",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_divide : OclBuiltin<"native_divide", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_exp    : OclBuiltin<"native_exp",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_exp2   : OclBuiltin<"native_exp2",   (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_exp10  : OclBuiltin<"native_exp10",  (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_log    : OclBuiltin<"native_log",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_log2   : OclBuiltin<"native_log2",   (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_log10  : OclBuiltin<"native_log10",  (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_powr   : OclBuiltin<"native_powr",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_recip  : OclBuiltin<"native_recip",  (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_rsqrt  : OclBuiltin<"native_rsqrt",  (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_sin    : OclBuiltin<"native_sin",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_sqrt   : OclBuiltin<"native_sqrt",   (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_tan    : OclBuiltin<"native_tan",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
// extened native
def native_acos          : OclBuiltin<"native_acos",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_acosh         : OclBuiltin<"native_acosh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_acospi        : OclBuiltin<"native_acospi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_asin          : OclBuiltin<"native_asin",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_asinh         : OclBuiltin<"native_asinh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_asinpi        : OclBuiltin<"native_asinpi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_atan          : OclBuiltin<"native_atan",      (outs gentype:$ret), (ins gentype:$y_over_x), [], [], 0, 0>;
def native_atan2         : OclBuiltin<"native_atan2",     (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], 0, 0>;
def native_atanh         : OclBuiltin<"native_atanh",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_atanpi        : OclBuiltin<"native_atanpi",    (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_atan2pi       : OclBuiltin<"native_atan2pi",   (outs gentype:$ret), (ins gentype:$y, gentype:$x), [], [], 0, 0>;
def native_cbrt          : OclBuiltin<"native_cbrt",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_cosh          : OclBuiltin<"native_cosh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_cospi         : OclBuiltin<"native_cospi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_erfc          : OclBuiltin<"native_erfc",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_erf           : OclBuiltin<"native_erf",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_expm1         : OclBuiltin<"native_expm1",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_fdim          : OclBuiltin<"native_fdim",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_fmax          : OclBuiltin<"native_fmax",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_fmax_s        : OclBuiltin<"native_fmax",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 0>, Except<[v1f32, v1f64]>;
def native_fmin          : OclBuiltin<"native_fmin",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_fmin_s        : OclBuiltin<"native_fmin",      (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 0>, Except<[v1f32, v1f64]>;
def native_fmod          : OclBuiltin<"native_fmod",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_fract_pas     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE; // MIC
def native_fract_las     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, LOCAL; // MIC
def native_fract_gas     : OclBuiltin<"native_fract",     (outs gentype:$ret), (ins gentype:$x, pgentype:$iptr), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, GLOBAL; // MIC
def native_fract_pas2    : OclBuiltin<"native_fract_ret2ptr",    (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_hypot         : OclBuiltin<"native_hypot",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_ilogb         : OclBuiltin<"native_ilogb",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_log1p         : OclBuiltin<"native_log1p",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_logb          : OclBuiltin<"native_logb",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_pow           : OclBuiltin<"native_pow",       (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def native_pown          : OclBuiltin<"native_pown",      (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], 0, 0>;
def native_rcbrt         : OclBuiltin<"native_rcbrt",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_rootn         : OclBuiltin<"native_rootn",     (outs gentype:$ret), (ins gentype:$x, i32gentype:$y), [], [], 0, 0>;
def native_sincos_pas    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_sincos_las    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def native_sincos_gas    : OclBuiltin<"native_sincos",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
def native_sincos_pas2   : OclBuiltin<"native_sincos_ret2ptr",    (outs), (ins gentype:$x, pgentype:$sinval ,pgentype:$cosval), [], [], 0, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def native_sinh          : OclBuiltin<"native_sinh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_sinpi         : OclBuiltin<"native_sinpi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_tanh          : OclBuiltin<"native_tanh",      (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def native_tanpi         : OclBuiltin<"native_tanpi",     (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
}
//Vectorizer generated table
//{{"_Z20__retbyvector_sincosd" , "_Z19__retbyarray_sincosDv2_d" , "_Z19__retbyarray_sincosDv4_d" , "_Z19__retbyarray_sincosDv8_d" , "_Z19__retbyarray_sincosDv16_d" , "_Z19__retbyarray_sincosDv3_d" },1, 0}
foreach bi = ["sincos", "native_sincos", "modf", "fract", "native_fract"] in {
  let Types = [v1f32, v1f64], Attrs = [CONSTFUNC, OVERLOADABLE] in {
        def bi#_scalarized :  OclBuiltin<"__retbyvector_"#bi, (outs v2gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
  }
  let Types = [v2f32, v3f32, v4f32, v8f32, v16f32,
      v2f64, v3f64, v4f64, v8f64, v16f64],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
        def retbyarray_#bi : OclBuiltin<"__retbyarray_"#bi, (outs), (ins gentype:$x), [], [], 0, 0>;
  }
 //float:
  //Cells
  def bi#cellf            : MapCell<!cast<OclBuiltin>(bi#"_scalarized"), v1f32>;
  def retbyarray_#bi#cellv2f32 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v2f32>;
  def retbyarray_#bi#cellv4f32 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v4f32>;
  def retbyarray_#bi#cellv8f32 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v8f32>;
  def retbyarray_#bi#cellv16f32 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v16f32>;
  def retbyarray_#bi#cellv3f32 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v3f32>;
  //Rows
  def bi#row_scalarizef            : MapRow<[
    !cast<MapCell>(bi#"cellf"), //scalar
    !cast<MapCell>("retbyarray_"#bi#"cellv2f32"), //v2
    !cast<MapCell>("retbyarray_"#bi#"cellv4f32"), //v4
    !cast<MapCell>("retbyarray_"#bi#"cellv8f32"), //v8
    !cast<MapCell>("retbyarray_"#bi#"cellv16f32"), //16
    !cast<MapCell>("retbyarray_"#bi#"cellv3f32")], //3,
    1,0>;
//double:
  //Cells
  def bi#celld            : MapCell<!cast<OclBuiltin>(bi#"_scalarized"), v1f64>;
  def retbyarray_#bi#cellv2f64 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v2f64>;
  def retbyarray_#bi#cellv4f64 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v4f64>;
  def retbyarray_#bi#cellv8f64 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v8f64>;
  def retbyarray_#bi#cellv16f64 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v16f64>;
  def retbyarray_#bi#cellv3f64 : MapCell<!cast<OclBuiltin>("retbyarray_"#bi), v3f64>;
  //Rows
  def bi#row_scalarized            : MapRow<[
    !cast<MapCell>(bi#"celld"), //scalar
    !cast<MapCell>("retbyarray_"#bi#"cellv2f64"), //v2
    !cast<MapCell>("retbyarray_"#bi#"cellv4f64"), //v4
    !cast<MapCell>("retbyarray_"#bi#"cellv8f64"), //v8
    !cast<MapCell>("retbyarray_"#bi#"cellv16f64"), //16
    !cast<MapCell>("retbyarray_"#bi#"cellv3f64")], //3,
    1,0>;
//float:
  def bi#_ret2ptrv4f32 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v4f32>;
  def bi#_ret2ptrv8f32 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v8f32>;
  def bi#_ret2ptrv16f32 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v16f32>;
  def bi#row_packetizef : MapRow<[
    !cast<MapCell>(bi#"cellf"), //scalar
    nullCell,
    !cast<MapCell>(bi#"_ret2ptrv4f32"),
    !cast<MapCell>(bi#"_ret2ptrv8f32"),
    !cast<MapCell>(bi#"_ret2ptrv16f32"),
    nullCell],
    0,1>;
//double:
  def bi#_ret2ptrv4f64 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v4f64>;
  def bi#_ret2ptrv8f64 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v8f64>;
  def bi#_ret2ptrv16f64 : MapCell<!cast<OclBuiltin>(bi#"_pas2"), v16f64>;
  def bi#row_packetized : MapRow<[
    !cast<MapCell>(bi#"celld"), //scalar
    nullCell,
    !cast<MapCell>(bi#"_ret2ptrv4f64"),
    !cast<MapCell>(bi#"_ret2ptrv8f64"),
    !cast<MapCell>(bi#"_ret2ptrv16f64"),
    nullCell],
    0,1>;
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
def abs       : OclBuiltin<"abs",       (outs ugentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def abs_diff  : OclBuiltin<"abs_diff",  (outs ugentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def add_sat   : OclBuiltin<"add_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def hadd      : OclBuiltin<"hadd",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def rhadd     : OclBuiltin<"rhadd",     (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def clz       : OclBuiltin<"clz",       (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def mad_hi    : OclBuiltin<"mad_hi",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], 0, 0>;
def mad_sat   : OclBuiltin<"mad_sat",   (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], 0, 0>;
def mul_hi    : OclBuiltin<"mul_hi",    (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def rotate    : OclBuiltin<"rotate",    (outs gentype:$ret), (ins gentype:$v, gentype:$i), [], [], 0, 0>;
def sub_sat   : OclBuiltin<"sub_sat",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def upsample  : OclBuiltin<"upsample",  (outs up_gentype:$ret), (ins gentype:$hi, ugentype:$lo), [], [], 0, 0>,
                Except<[v1i64, v2i64, v3i64, v4i64, v8i64, v16i64, v1u64, v2u64, v3u64, v4u64, v8u64, v16u64]>;
let NeedForwardDecl = 1 in /* work around the missing prototype in opencl.h */
def popcount  : OclBuiltin<"popcount",  (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
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
def clamp   : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, gentype:$minval, gentype:$maxval), [], [], 0, 0>;
def clamp_s : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, sgentype:$minval, sgentype:$maxval), [], [], 0, 0>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def max     : OclBuiltin<"max",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def max_s   : OclBuiltin<"max",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 0>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
def min     : OclBuiltin<"min",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def min_s   : OclBuiltin<"min",   (outs gentype:$ret), (ins gentype:$x, sgentype:$y), [], [], 0, 0>,
              Except<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64]>;
}


// OpenCL 1.2 rev 15, Table 6.11
let Types = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def mad24   : OclBuiltin<"mad24", (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$z), [], [], 0, 0>;
def mul24   : OclBuiltin<"mul24", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
}

// OpenCL 1.2 rev 15, Table 6.12
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def degrees       : OclBuiltin<"degrees",     (outs gentype:$ret), (ins gentype:$radians), [], [], 0, 0>;
def mix           : OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$a), [], [], 0, 0>;
def mix_s         : OclBuiltin<"mix",         (outs gentype:$ret), (ins gentype:$x, gentype:$y, sgentype:$a), [], [], 0, 0>, Except<[v1f32, v1f64]>;
def radians       : OclBuiltin<"radians",     (outs gentype:$ret), (ins gentype:$degrees), [], [], 0, 0>;
def step          : OclBuiltin<"step",        (outs gentype:$ret), (ins gentype:$edge, gentype:$x), [], [], 0, 0>;
def step_s        : OclBuiltin<"step",        (outs gentype:$ret), (ins sgentype:$edge, gentype:$x), [], [], 0, 0>, Except<[v1f32, v1f64]>;
def smoothstep    : OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins gentype:$edge0, gentype:$edge1, gentype:$x), [], [], 0, 0>;
def smoothstep_s  : OclBuiltin<"smoothstep",  (outs gentype:$ret), (ins sgentype:$edge0, sgentype:$edge1, gentype:$x), [], [], 0, 0>, Except<[v1f32, v1f64]>;
def sign          : OclBuiltin<"sign",        (outs gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// OpenCL 1.2 rev 15, Table 6.13
let Types = [v1f32, v2f32, v3f32, v4f32,
             v1f64, v2f64, v3f64, v4f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def cross     : OclBuiltin<"cross",     (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>,
                Except<[v1f32, v2f32, v1f64, v2f64]>;
def dot       : OclBuiltin<"dot",       (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def distance  : OclBuiltin<"distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def length    : OclBuiltin<"length",    (outs sgentype:$ret), (ins gentype:$p), [], [], 0, 0>;
def normalize : OclBuiltin<"normalize", (outs gentype:$ret), (ins gentype:$p), [], [], 0, 0>;
}
let Types = [v1f32, v2f32, v3f32, v4f32],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def fast_distance  : OclBuiltin<"fast_distance",  (outs sgentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def fast_length    : OclBuiltin<"fast_length",    (outs sgentype:$ret), (ins gentype:$p), [], [], 0, 0>;
def fast_normalize : OclBuiltin<"fast_normalize", (outs gentype:$ret), (ins gentype:$p), [], [], 0, 0>;
}

// OpenCL 1.2 rev 15, Table 6.14
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
//def isequal         : OclBuiltin<"isequal",         (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
//def isnotequal      : OclBuiltin<"isnotequal",      (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
//def isgreater       : OclBuiltin<"isgreater",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
//def isgreaterequal  : OclBuiltin<"isgreaterequal",  (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
//def isless          : OclBuiltin<"isless",          (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
//def islessequal     : OclBuiltin<"islessequal",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def islessgreater   : OclBuiltin<"islessgreater",   (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
let NeedForwardDecl = 1 in {
def isfinite        : OclBuiltin<"isfinite",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def isinf           : OclBuiltin<"isinf",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def isnan           : OclBuiltin<"isnan",           (outs rf2igentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def isnormal        : OclBuiltin<"isnormal",        (outs rf2igentype:$ret), (ins gentype:$x), [], [], 0, 1>;
def isordered       : OclBuiltin<"isordered",       (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def isunordered     : OclBuiltin<"isunordered",     (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 0>;
def signbit         : OclBuiltin<"signbit",         (outs rf2igentype:$ret), (ins gentype:$x), [], [], 0, 1>;
}
}
let Types = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
def any : OclBuiltin<"any", (outs si32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def all : OclBuiltin<"all", (outs si32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
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
def bitselect   : OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c), [], [], 0, 0>;
def iselect     : OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c), [], [], 0, 0>;
def uselect     : OclBuiltin<"select",    (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c), [], [], 0, 0>;
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
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def vload_gas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def vload_cas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def vload_las   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def vload_pas   : OclBuiltin<"vload$Suffix",  (outs gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
def vstore_pas  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstore_las  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstore_gas  : OclBuiltin<"vstore$Suffix", (outs), (ins gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}
let Types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def vload_half_gas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def vload_half_cas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def vload_half_las        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def vload_half_pas        : OclBuiltin<"vload_half$Suffix",       (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
def vstoref_half_pas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoref_half_las      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoref_half_gas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoref_half_rte_pas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoref_half_rte_las  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoref_half_rte_gas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoref_half_rtz_pas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoref_half_rtz_las  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoref_half_rtz_gas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoref_half_rtp_pas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoref_half_rtp_las  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoref_half_rtp_gas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoref_half_rtn_pas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoref_half_rtn_las  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoref_half_rtn_gas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstored_half_pas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstored_half_las      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstored_half_gas      : OclBuiltin<"vstore_half$Suffix",      (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstored_half_rte_pas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstored_half_rte_las  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstored_half_rte_gas  : OclBuiltin<"vstore_half$Suffix_rte",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstored_half_rtz_pas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstored_half_rtz_las  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstored_half_rtz_gas  : OclBuiltin<"vstore_half$Suffix_rtz",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstored_half_rtp_pas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstored_half_rtp_las  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstored_half_rtp_gas  : OclBuiltin<"vstore_half$Suffix_rtp",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstored_half_rtn_pas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstored_half_rtn_las  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstored_half_rtn_gas  : OclBuiltin<"vstore_half$Suffix_rtn",  (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vloada_half_gas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_GLOBAL;
def vloada_half_cas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONSTANT;
def vloada_half_las       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_LOCAL;
def vloada_half_pas       : OclBuiltin<"vloada_half$Suffix",      (outs up_gentype:$ret), (ins size_t:$offset, psgentype:$p), [], [], 0, 0>, CONST_PRIVATE;
def vstoreaf_half_pas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoreaf_half_las     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoreaf_half_gas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}
let Types = [v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def vstoreaf_half_rte_pas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoreaf_half_rte_las : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoreaf_half_rte_gas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoreaf_half_rtz_pas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoreaf_half_rtz_las : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoreaf_half_rtz_gas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoreaf_half_rtp_pas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoreaf_half_rtp_las : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoreaf_half_rtp_gas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoreaf_half_rtn_pas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoreaf_half_rtn_las : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoreaf_half_rtn_gas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}
let Types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def vstoread_half_pas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoread_half_las     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoread_half_gas     : OclBuiltin<"vstorea_half$Suffix",     (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
}
let Types = [v2f16, v3f16, v4f16, v8f16, v16f16],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def vstoread_half_rte_pas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoread_half_rte_las : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoread_half_rte_gas : OclBuiltin<"vstorea_half$Suffix_rte", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoread_half_rtz_pas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoread_half_rtz_las : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoread_half_rtz_gas : OclBuiltin<"vstorea_half$Suffix_rtz", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoread_half_rtp_pas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoread_half_rtp_las : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoread_half_rtp_gas : OclBuiltin<"vstorea_half$Suffix_rtp", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
def vstoread_half_rtn_pas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, PRIVATE;
def vstoread_half_rtn_las : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, LOCAL;
def vstoread_half_rtn_gas : OclBuiltin<"vstorea_half$Suffix_rtn", (outs), (ins up2_gentype:$data, size_t:$offset, psgentype:$p), [], [], 0, 0>, GLOBAL;
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
def shuffle_2   : OclBuiltin<"shuffle",   (outs gentype2:$ret), (ins gentype:$x, ugentype2:$mask), [], [], 0, 0>;
def shuffle_4   : OclBuiltin<"shuffle",   (outs gentype4:$ret), (ins gentype:$x, ugentype4:$mask), [], [], 0, 0>;
def shuffle_8   : OclBuiltin<"shuffle",   (outs gentype8:$ret), (ins gentype:$x, ugentype8:$mask), [], [], 0, 0>;
def shuffle_16  : OclBuiltin<"shuffle",   (outs gentype16:$ret), (ins gentype:$x, ugentype16:$mask), [], [], 0, 0>;
def shuffle2_2  : OclBuiltin<"shuffle2",  (outs gentype2:$ret), (ins gentype:$x, gentype:$y, ugentype2:$mask), [], [], 0, 0>;
def shuffle2_4  : OclBuiltin<"shuffle2",  (outs gentype4:$ret), (ins gentype:$x, gentype:$y, ugentype4:$mask), [], [], 0, 0>;
def shuffle2_8  : OclBuiltin<"shuffle2",  (outs gentype8:$ret), (ins gentype:$x, gentype:$y, ugentype8:$mask), [], [], 0, 0>;
def shuffle2_16 : OclBuiltin<"shuffle2",  (outs gentype16:$ret), (ins gentype:$x, gentype:$y, ugentype16:$mask), [], [], 0, 0>;
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
def convert_u8          : OclBuiltin<"convert_$ReturnType",         (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8          : OclBuiltin<"convert_$ReturnType",         (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16         : OclBuiltin<"convert_$ReturnType",         (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16         : OclBuiltin<"convert_$ReturnType",         (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32         : OclBuiltin<"convert_$ReturnType",         (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32         : OclBuiltin<"convert_$ReturnType",         (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64         : OclBuiltin<"convert_$ReturnType",         (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64         : OclBuiltin<"convert_$ReturnType",         (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_sat      : OclBuiltin<"convert_$ReturnType_sat",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_sat      : OclBuiltin<"convert_$ReturnType_sat",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
// conversions from floating to integer also honor rounding mode as well as saturated mode.
let ExceptionTypes = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                       v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                      v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                      v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                      v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                      v1u64, v2u64, v3u64, v4u64, v8u64, v16u64] in {
def convert_u8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_u64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_i64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
// conversion to floating may not has saturated mode.
def convert_f32         : OclBuiltin<"convert_$ReturnType",         (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f64         : OclBuiltin<"convert_$ReturnType",         (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def convert_f64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
// the following conversions make no sense but are added for completeness.
//
// conversions from int to integer with rounding mode and/or saturated mode.
let ExceptionTypes = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                      v1f64, v2f64, v3f64, v4f64, v8f64, v16f64] in {
def _convert_u8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_rte      : OclBuiltin<"convert_$ReturnType_rte",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_rtz      : OclBuiltin<"convert_$ReturnType_rtz",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_rtp      : OclBuiltin<"convert_$ReturnType_rtp",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_rtn      : OclBuiltin<"convert_$ReturnType_rtn",     (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  i8gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i16_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i16gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_u64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs u64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_rte     : OclBuiltin<"convert_$ReturnType_rte",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_rtz     : OclBuiltin<"convert_$ReturnType_rtz",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_rtp     : OclBuiltin<"convert_$ReturnType_rtp",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_rtn     : OclBuiltin<"convert_$ReturnType_rtn",     (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_i64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs i64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}
// conversion to floating make nosense with saturated mode.
def _convert_f32_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f32_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f32_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f32_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f32_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f64_sat     : OclBuiltin<"convert_$ReturnType_sat",     (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f64_sat_rte : OclBuiltin<"convert_$ReturnType_sat_rte", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f64_sat_rtz : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f64_sat_rtp : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
def _convert_f64_sat_rtn : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs f64gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// Extended OpenCL built-in for SOA Geometry functions
let Types = [v16f32, v8f64, v4f32, v8f32, v4f64, v16f64],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def soa_dot1           : OclBuiltin<"soa_dot1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def soa_dot2           : OclBuiltin<"soa_dot2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], [], 0, 0>;
def soa_dot3           : OclBuiltin<"soa_dot3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], [], 0, 0>;
def soa_dot4           : OclBuiltin<"soa_dot4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], [], 0, 0>;

def soa_length1        : OclBuiltin<"soa_length1",   (outs gentype:$ret), (ins gentype:$p0), [], [], 0, 0>;
def soa_length2        : OclBuiltin<"soa_length2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y), [], [], 0, 0>;
def soa_length3        : OclBuiltin<"soa_length3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z), [], [], 0, 0>;
def soa_length4        : OclBuiltin<"soa_length4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w), [], [], 0, 0>;

def soa_distance1     : OclBuiltin<"soa_distance1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def soa_distance2     : OclBuiltin<"soa_distance2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], [], 0, 0>;
def soa_distance3     : OclBuiltin<"soa_distance3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], [], 0, 0>;
def soa_distance4     : OclBuiltin<"soa_distance4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], [], 0, 0>;

def soa_cross3           : OclBuiltin<"soa_cross3", (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], [], 0, 0>, PRIVATE;
def soa_cross4           : OclBuiltin<"soa_cross4", (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], [], 0, 0>, PRIVATE;

def soa_normalize1       : OclBuiltin<"soa_normalize1",   (outs gentype:$ret), (ins gentype:$p0), [], [], 0, 0>;
def soa_normalize2       : OclBuiltin<"soa_normalize2",   (outs), (ins gentype:$p0_x, gentype:$p0_y, pgentype:$res_x, pgentype:$res_y), [], [], 0, 0>, PRIVATE;
def soa_normalize3       : OclBuiltin<"soa_normalize3",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], [], 0, 0>, PRIVATE;
def soa_normalize4       : OclBuiltin<"soa_normalize4",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], [], 0, 0>, PRIVATE;
}
let Types = [v16f32, v4f32, v8f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def soa_fast_length1   : OclBuiltin<"soa_fast_length1",   (outs gentype:$ret), (ins gentype:$p0), [], [], 0, 0>;
def soa_fast_length2   : OclBuiltin<"soa_fast_length2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y), [], [], 0, 0>;
def soa_fast_length3   : OclBuiltin<"soa_fast_length3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z), [], [], 0, 0>;
def soa_fast_length4   : OclBuiltin<"soa_fast_length4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w), [], [], 0, 0>;

def soa_fast_distance1 : OclBuiltin<"soa_fast_distance1",   (outs gentype:$ret), (ins gentype:$p0, gentype:$p1), [], [], 0, 0>;
def soa_fast_distance2 : OclBuiltin<"soa_fast_distance2",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p1_x, gentype:$p1_y), [], [], 0, 0>;
def soa_fast_distance3 : OclBuiltin<"soa_fast_distance3",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z), [], [], 0, 0>;
def soa_fast_distance4 : OclBuiltin<"soa_fast_distance4",   (outs gentype:$ret), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, gentype:$p1_x, gentype:$p1_y, gentype:$p1_z, gentype:$p1_w), [], [], 0, 0>;

def soa_fast_normalize1       : OclBuiltin<"soa_fast_normalize1",   (outs gentype:$ret), (ins gentype:$p0), [], [], 0, 0>;
def soa_fast_normalize2       : OclBuiltin<"soa_fast_normalize2",   (outs), (ins gentype:$p0_x, gentype:$p0_y, pgentype:$res_x, pgentype:$res_y), [], [], 0, 0>, PRIVATE;
def soa_fast_normalize3       : OclBuiltin<"soa_fast_normalize3",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z), [], [], 0, 0>, PRIVATE;
def soa_fast_normalize4       : OclBuiltin<"soa_fast_normalize4",   (outs), (ins gentype:$p0_x, gentype:$p0_y, gentype:$p0_z, gentype:$p0_w, pgentype:$res_x, pgentype:$res_y, pgentype:$res_z, pgentype:$res_w), [], [], 0, 0>, PRIVATE;
}

// OpenCL 1.2 rev 15, Section 6.12.11 Explicit Conversions
//
// Atomic Functions
let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atomic_add_global : OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_add_local  : OclBuiltin<"atomic_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_sub_global : OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_sub_local  : OclBuiltin<"atomic_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_inc_global : OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_inc_local  : OclBuiltin<"atomic_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_dec_global : OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_dec_local  : OclBuiltin<"atomic_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_cmpxchg_global : OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_cmpxchg_local  : OclBuiltin<"atomic_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_min_global : OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_min_local  : OclBuiltin<"atomic_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_max_global : OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_max_local  : OclBuiltin<"atomic_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_and_global : OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_and_local  : OclBuiltin<"atomic_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_or_global : OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_or_local  : OclBuiltin<"atomic_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_xor_global : OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_xor_local  : OclBuiltin<"atomic_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atomic_xchg_global : OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_xchg_local  : OclBuiltin<"atomic_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 1, 0>, VOLATILE_LOCAL;
}

let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atom_add_vol_global     : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_add_global         : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_sub_vol_global     : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_sub_global         : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_inc_vol_global     : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_inc_global         : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, GLOBAL;
def atom_dec_vol_global     : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_dec_global         : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, GLOBAL;
def atom_cmpxchg_vol_global : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_cmpxchg_global     : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_min_vol_global     : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_min_global         : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_max_vol_global     : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_max_global         : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_and_vol_global     : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_and_global         : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_or_vol_global      : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_or_global          : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_xor_vol_global     : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_xor_global         : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;

def atom_add_vol_local     : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_add_local         : OclBuiltin<"atom_add", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_sub_vol_local     : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_sub_local         : OclBuiltin<"atom_sub", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_cmpxchg_vol_local : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_cmpxchg_local     : OclBuiltin<"atom_cmpxchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$cmp, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_inc_vol_local     : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_inc_local         : OclBuiltin<"atom_inc", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, LOCAL;
def atom_dec_vol_local     : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_dec_local         : OclBuiltin<"atom_dec", (outs gentype:$ret), (ins pgentype:$p), [], [], 0, 0>, LOCAL;
def atom_min_vol_local     : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_min_local         : OclBuiltin<"atom_min", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_max_vol_local     : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_max_local         : OclBuiltin<"atom_max", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_and_vol_local     : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_and_local         : OclBuiltin<"atom_and", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_or_vol_local      : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_or_local          : OclBuiltin<"atom_or", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
def atom_xor_vol_local     : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_xor_local         : OclBuiltin<"atom_xor", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
}

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE],
    NeedForwardDecl = 1 in {
def atom_xchg_vol_global    : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_GLOBAL;
def atom_xchg_global        : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, GLOBAL;
def atom_xchg_vol_local    : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, VOLATILE_LOCAL;
def atom_xchg_local        : OclBuiltin<"atom_xchg", (outs gentype:$ret), (ins pgentype:$p, gentype:$val), [], [], 0, 0>, LOCAL;
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
  NeedForwardDecl = 1  in {
def soa_any1 : OclBuiltin<"soa_any1", (outs i32gentype:$ret), (ins gentype:$x0), [], [], 0, 0>;
def soa_any2 : OclBuiltin<"soa_any2", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1), [], [], 0, 0>;
def soa_any3 : OclBuiltin<"soa_any3", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, gentype:$x2), [], [], 0, 0>;
def soa_any4 : OclBuiltin<"soa_any4", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1,
  gentype:$x2, gentype:$x3), [], [], 0, 0>;
def soa_any8 : OclBuiltin<"soa_any8", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1,
  gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5,
  gentype:$x6, gentype:$x7), [], [], 0, 0>;
def soa_any16 : OclBuiltin<"soa_any16", (outs i32gentype:$ret), (ins gentype:$x0,  gentype:$x1,
  gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5,
  gentype:$x6, gentype:$x7, gentype:$x8, gentype:$x9,
  gentype:$x10, gentype:$x11, gentype:$x12, gentype:$x13,
  gentype:$x14, gentype:$x15), [], [], 0, 0>;
def soa_all1 : OclBuiltin<"soa_all1", (outs i32gentype:$ret), (ins gentype:$x0), [], [], 0, 0>;
def soa_all2 : OclBuiltin<"soa_all2", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1), [], [], 0, 0>;
def soa_all3 : OclBuiltin<"soa_all3", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1, gentype:$x2), [], [], 0, 0>;
def soa_all4 : OclBuiltin<"soa_all4", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1,
  gentype:$x2, gentype:$x3), [], [], 0, 0>;
def soa_all8 : OclBuiltin<"soa_all8", (outs i32gentype:$ret), (ins gentype:$x0, gentype:$x1,
  gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5,
  gentype:$x6, gentype:$x7), [], [], 0, 0>;
def soa_all16 : OclBuiltin<"soa_all16", (outs i32gentype:$ret), (ins gentype:$x0,  gentype:$x1,
  gentype:$x2, gentype:$x3, gentype:$x4, gentype:$x5,
  gentype:$x6, gentype:$x7, gentype:$x8, gentype:$x9,
  gentype:$x10, gentype:$x11, gentype:$x12, gentype:$x13,
  gentype:$x14, gentype:$x15), [], [], 0, 0>;
}

///////////////////////////////////////////////////////////////////////////////
//Soa versions for relationals
///////////////////////////////////////////////////////////////////////////////

def downConvert : OclBuiltin<"downConvert", (outs downgentype:$ret), (ins gentype:$p),
  [v4i64, v8i64, v16i64], [OVERLOADABLE], 0, 0>;

//
//vectorized versions declaraions for ralational functions
//
foreach relationf = [/*"isless", "isgreater", "isequal", "isnotequal", "isgreaterequal", "islessequal",*/
  "islessgreater"/*, "isfinite"*/] in {
  //scalar
  def scalar_#relationf : OclBuiltin<"scalar_"#relationf, (outs igentype:$res),
    (ins gentype:$l, gentype:$r), [v1f32, v1f64], [OVERLOADABLE], 0, 0>;
  //
  //generating an vecroization map entries, for the auto-code gen for the reflection module
  //
  //
  //entries for scalarizing packed versions
  //
  //floats:
  def map_cell_scalar_#relationf#1 : MapCell<!cast<OclBuiltin>("scalar_"#relationf), v1f32>;
  def map_cell#relationf#2 : MapCell<!cast<OclBuiltin>(relationf), v2f32>;
  def map_cell#relationf#3 : MapCell<!cast<OclBuiltin>(relationf), v3f32>;
  def map_cell#relationf#4 : MapCell<!cast<OclBuiltin>(relationf), v4f32>;
  def map_cell#relationf#8 : MapCell<!cast<OclBuiltin>(relationf), v8f32>;
  def map_cell#relationf#16 :MapCell<!cast<OclBuiltin>(relationf), v16f32>;
  def relationf#"_scalar": MapRow<[
    !cast<MapCell>("map_cell_scalar_"#relationf#1), //scalar version
    !cast<MapCell>("map_cell"#relationf#2), //v2
    !cast<MapCell>("map_cell"#relationf#4), //v4
    !cast<MapCell>("map_cell"#relationf#8), //v8
    !cast<MapCell>("map_cell"#relationf#16),//v16
    !cast<MapCell>("map_cell"#relationf#3)],//v3
    1, 1>; //scalarizable, packetizable
  //
  //doubles:
  //
  def map_cell64_scalar_#relationf : MapCell<!cast<OclBuiltin>("scalar_"#relationf), v1f64>;
  def map_cell64#relationf#2 : MapCell<!cast<OclBuiltin>(relationf), v2f64>;
  def map_cell64#relationf#3 : MapCell<!cast<OclBuiltin>(relationf), v3f64>;
  def map_cell64#relationf#4 : MapCell<!cast<OclBuiltin>(relationf), v4f64>;
  def map_cell64#relationf#8 : MapCell<!cast<OclBuiltin>(relationf), v8f64>;
  def map_cell64#relationf#16 :MapCell<!cast<OclBuiltin>(relationf), v16f64>;
  def relationf#"64_scalar": MapRow<[
    !cast<MapCell>("map_cell64_scalar_"#relationf), //scalar version
    !cast<MapCell>("map_cell64"#relationf#2), //v2
    !cast<MapCell>("map_cell64"#relationf#4), //v4
    !cast<MapCell>("map_cell64"#relationf#8), //v8
    !cast<MapCell>("map_cell64"#relationf#16),//v16
    !cast<MapCell>("map_cell64"#relationf#3)],//v3
    1, 1>; //scalarizable, packetizable
  //
  //entries for packetizing scalar versions
  //
  def map_cell_#relationf#1 : MapCell<!cast<OclBuiltin>(relationf), v1f32>;
  def v_#relationf : OclBuiltin<"v_"#relationf, (outs i32gentype:$res),
    (ins gentype:$l, gentype:$r), [v4f32, v4f64, v8f32, v8f64, v16f32, v16f64], [OVERLOADABLE], 0, 0>;
  //
  //floats
  //
  def map_cell_v#relationf#4 : MapCell<!cast<OclBuiltin>("v_"#relationf), v4f32>;
  def map_cell_v#relationf#8 : MapCell<!cast<OclBuiltin>("v_"#relationf), v8f32>;
  def map_cell_v#relationf#16: MapCell<!cast<OclBuiltin>("v_"#relationf),v16f32>;
  def relationf#_nullentry: MapRow<[
    !cast<MapCell>("map_cell_"#relationf#1),
    nullCell, //v2
    !cast<MapCell>("map_cell_v"#relationf#4), //v4
    !cast<MapCell>("map_cell_v"#relationf#8), //v8
    !cast<MapCell>("map_cell_v"#relationf#16),//v16
    nullCell],//v3
    0,1>; //not scalarizable, packetizable
  //
  //doubles
  //
  def map_cell_64#relationf#1 : MapCell<!cast<OclBuiltin>(relationf), v1f64>;
  def map_cell_64v#relationf#4 : MapCell<!cast<OclBuiltin>("v_"#relationf), v4f64>;
  def map_cell_64v#relationf#8 : MapCell<!cast<OclBuiltin>("v_"#relationf), v8f64>;
  def map_cell_64v#relationf#16: MapCell<!cast<OclBuiltin>("v_"#relationf),v16f64>;
  def relationf#64_nullentry: MapRow<[
    !cast<MapCell>("map_cell_64"#relationf#1),
    nullCell, //v2
    !cast<MapCell>("map_cell_64v"#relationf#4), //v4
    !cast<MapCell>("map_cell_64v"#relationf#8), //v8
    !cast<MapCell>("map_cell_64v"#relationf#16),//v16
    nullCell],//v3
    0,1>; //not scalarizable, packetizable
}


let Types = [v1i64],
    Attrs = [OVERLOADABLE] in {
  // Memory Fence Functions
  def mem_fence : OclBuiltin<"mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [], [], 0, 0>;
  def read_mem_fence : OclBuiltin<"read_mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [], [], 0, 0>;
  def write_mem_fence : OclBuiltin<"write_mem_fence", (outs), (ins cl_mem_fence_flags:$flags), [], [], 0, 0>;
}

let Types = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = 1 in {
  def idiv          : OclBuiltin<"idiv",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
  def irem          : OclBuiltin<"irem",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;

}
let Types = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32],
    Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = 1 in {
  def udiv          : OclBuiltin<"udiv",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
  def urem          : OclBuiltin<"urem",      (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 0, 1>;
}

///////////////////////////////////////////////////////////////////////////////
// OpenCL 1.2 rev 15, Table 6.18
// Async copy built-ins
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
    Attrs = [OVERLOADABLE] in {
  def async_work_group_copy_l2g : OclBuiltin<"async_work_group_copy", (outs event_t:$ret), (ins gpgentype:$dst, clpgentype:$src, size_t:$num_elements, event_t:$event), [], [], 0, 0>;
  def async_work_group_copy_g2l : OclBuiltin<"async_work_group_copy", (outs event_t:$ret), (ins lpgentype:$dst, cgpgentype:$src, size_t:$num_elements, event_t:$event), [], [], 0, 0>;
  def async_work_group_strided_copy_l2g : OclBuiltin<"async_work_group_strided_copy", (outs event_t:$ret), (ins gpgentype:$dst, clpgentype:$src, size_t:$num_elements, size_t:$dst_stride, event_t:$event), [], [], 0, 0>;
  def async_work_group_strided_copy_g2l : OclBuiltin<"async_work_group_strided_copy", (outs event_t:$ret), (ins lpgentype:$dst, cgpgentype:$src, size_t:$num_elements, size_t:$src_stride, event_t:$event), [], [], 0, 0>;
}

let Types = [ void ],
    Attrs = [OVERLOADABLE] in {
  def wait_group_events : OclBuiltin<"wait_group_events", (outs), (ins v1i32:$num_events, pevent_t:$event_list), [], [], 0, 0>, PRIVATE;
}

///////////////////////////////////////////////////////////////////////////////
// Helper built-ins
// intel_movemask
// custom BI used in GENERIC and CPU for fast detecting analyzing mask in if()
// for scalar types it returns true if input is NOT equal to zero otherwise returns zero
// for vector types it returns most significant bit in each vector element compressed to output mask,
// behaviour is equal to _mm_movemask() intrinsic from x86 SSE
// example of use: fract
let Types = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def intel_movemask: OclBuiltin<"intel_movemask", (outs si32gentype:$ret), (ins gentype:$x), [], [], 0, 0>;
}

// OpenCL 1.2 Section 6.12 Image Functions (those which have SOA versions)
//

foreach qual = [ "ro", "rw" ] in {
  let Types = [ v4f32, v4i32 ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def read_imagef_2d_#qual  : OclBuiltin<"read_imagef",  (outs f32gentype:$ret), (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  def read_imagei_2d_#qual  : OclBuiltin<"read_imagei",  (outs i32gentype:$ret), (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  def read_imageui_2d_#qual : OclBuiltin<"read_imageui", (outs u32gentype:$ret), (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imagef_2d_#qual  : OclBuiltin<"mask_read_imagef",  (outs f32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imagei_2d_#qual  : OclBuiltin<"mask_read_imagei",  (outs i32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imageui_2d_#qual : OclBuiltin<"mask_read_imageui", (outs u32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, gentype2:$coord), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "rw" ] in {
  let Types = [ v4i32 ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def read_imageui_2d_#qual#_no_sampler : OclBuiltin<"read_imageui", (outs u32gentype:$ret), (ins !cast<OclType>("image2d_"#qual):$image, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imagef_2d_#qual#_no_sampler  : OclBuiltin<"mask_read_imagef",  (outs f32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imagei_2d_#qual#_no_sampler  : OclBuiltin<"mask_read_imagei",  (outs i32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord), [], [], 0, 0>;
  def mask_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"mask_read_imageui", (outs u32gentype:$ret), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord), [], [], 0, 0>;
  }
}

foreach qual = [ "wo", "rw" ] in {
  let Types = [ v4i32 ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def write_imagef_2d_#qual  : OclBuiltin<"write_imagef",  (outs), (ins !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, f32gentype:$color), [], [], 0, 0>;
  def write_imagei_2d_#qual  : OclBuiltin<"write_imagei",  (outs), (ins !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, i32gentype:$color), [], [], 0, 0>;
  def write_imageui_2d_#qual : OclBuiltin<"write_imageui", (outs), (ins !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, u32gentype:$color), [], [], 0, 0>;
  def mask_write_imagef_2d_#qual  : OclBuiltin<"mask_write_imagef",  (outs), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, f32gentype:$color), [], [], 0, 0>;
  def mask_write_imagei_2d_#qual  : OclBuiltin<"mask_write_imagei",  (outs), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, i32gentype:$color), [], [], 0, 0>;
  def mask_write_imageui_2d_#qual : OclBuiltin<"mask_write_imageui", (outs), (ins v1i32:$mask, !cast<OclType>("image2d_"#qual):$image, gentype2:$coord, u32gentype:$color), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image2d_"#qual),
                !cast<OclType>("image3d_"#qual),
                !cast<OclType>("image2d_array_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def get_image_height_#qual : OclBuiltin<"get_image_height", (outs v1i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image1d_"#qual),
                !cast<OclType>("image2d_"#qual),
                !cast<OclType>("image3d_"#qual),
                !cast<OclType>("image1d_array_"#qual),
                !cast<OclType>("image1d_buffer_"#qual),
                !cast<OclType>("image2d_array_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def get_image_width_#qual  : OclBuiltin<"get_image_width",  (outs v1i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def get_image_channel_order_#qual : OclBuiltin<"get_image_channel_order",  (outs v1i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def get_image_channel_data_type_#qual : OclBuiltin<"get_image_channel_data_type", (outs v1i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image3d_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def get_image_depth_#qual  : OclBuiltin<"get_image_depth",  (outs v1i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def get_image_dim_2d_#qual : OclBuiltin<"get_image_dim",  (outs v2i32:$ret), (ins !cast<OclType>("image2d_"#qual):$image), [], [], 0, 0>;
  def get_image_dim_3d_#qual : OclBuiltin<"get_image_dim",  (outs v4i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  }
}

// SOA versions of image functions above
//
foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image2d_"#qual),
                !cast<OclType>("image3d_"#qual),
                !cast<OclType>("image2d_array_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def soa4_get_image_height_#qual : OclBuiltin<"soa4_get_image_height", (outs v4i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa8_get_image_height_#qual : OclBuiltin<"soa8_get_image_height", (outs v8i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image1d_"#qual),
                !cast<OclType>("image2d_"#qual),
                !cast<OclType>("image3d_"#qual),
                !cast<OclType>("image1d_array_"#qual),
                !cast<OclType>("image1d_buffer_"#qual),
                !cast<OclType>("image2d_array_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def soa4_get_image_width_#qual : OclBuiltin<"soa4_get_image_width", (outs v4i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa8_get_image_width_#qual : OclBuiltin<"soa8_get_image_width", (outs v8i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa4_get_image_channel_order_#qual : OclBuiltin<"soa4_get_image_channel_order", (outs v4i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa8_get_image_channel_order_#qual : OclBuiltin<"soa8_get_image_channel_order", (outs v8i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa4_get_image_channel_data_type_#qual : OclBuiltin<"soa4_get_image_channel_data_type", (outs v4i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  def soa8_get_image_channel_data_type_#qual : OclBuiltin<"soa8_get_image_channel_data_type", (outs v8i32:$ret), (ins gentype:$image), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image3d_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def soa4_get_image_depth_#qual : OclBuiltin<"soa4_get_image_depth", (outs v4i32:$ret), (ins !cast<OclType>("image3d_"#qual):$image), [], [], 0, 0>;
  def soa8_get_image_depth_#qual : OclBuiltin<"soa8_get_image_depth", (outs v8i32:$ret), (ins !cast<OclType>("image3d_"#qual):$image), [], [], 0, 0>;
  }
}

foreach qual = [ "ro", "rw" ] in {
  let Types = [ !cast<OclType>("image2d_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def soa4_read_imageui_2d_#qual : OclBuiltin<"soa4_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, v4i32:$coord_x, v4i32:$coord_y,
                     pv4u32:$res_x, pv4u32:$res_y, pv4u32:$res_z, pv4u32:$res_w), [], [], 0, 0>, PRIVATE;
  def soa8_read_imageui_2d_#qual : OclBuiltin<"soa8_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, v8i32:$coord_x, v8i32:$coord_y,
                     pv8u32:$res_x, pv8u32:$res_y, pv8u32:$res_z, pv8u32:$res_w), [], [], 0, 0>, PRIVATE;
  def soa16_read_imageui_2d_#qual : OclBuiltin<"soa16_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, v16i32:$coord_x, v16i32:$coord_y,
                     pv16u32:$res_x, pv16u32:$res_y, pv16u32:$res_z, pv16u32:$res_w), [], [], 0, 0>, PRIVATE;
  def soa4_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"soa4_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v4i32:$coord_x, v4i32:$coord_y,
                     pv4u32:$res_x, pv4u32:$res_y, pv4u32:$res_z, pv4u32:$res_w), [], [], 0, 0>, PRIVATE;
  def soa8_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"soa8_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v8i32:$coord_x, v8i32:$coord_y,
                     pv8u32:$res_x, pv8u32:$res_y, pv8u32:$res_z, pv8u32:$res_w), [], [], 0, 0>, PRIVATE;
  def soa16_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"soa16_read_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v16i32:$coord_x, v16i32:$coord_y,
                     pv16u32:$res_x, pv16u32:$res_y, pv16u32:$res_z, pv16u32:$res_w), [], [], 0, 0>, PRIVATE;
  def mask_soa4_read_imageui_2d_#qual : OclBuiltin<"mask_soa4_read_imageui", (outs),
                (ins v4i32:$mask, !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, v4i32:$coord_x, v4i32:$coord_y,
                     pv4u32:$res_x, pv4u32:$res_y, pv4u32:$res_z, pv4u32:$res_w), [], [], 0, 0>, PRIVATE;
  def mask_soa8_read_imageui_2d_#qual : OclBuiltin<"mask_soa8_read_imageui", (outs),
                (ins v8i32:$mask, !cast<OclType>("image2d_"#qual):$image, sampler_t:$sampler, v8i32:$coord_x, v8i32:$coord_y,
                     pv8u32:$res_x, pv8u32:$res_y, pv8u32:$res_z, pv8u32:$res_w), [], [], 0, 0>, PRIVATE;
  def mask_soa4_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"mask_soa4_read_imageui", (outs),
                (ins v4i32:$mask, !cast<OclType>("image2d_"#qual):$image, v4i32:$coord_x, v4i32:$coord_y,
                     pv4u32:$res_x, pv4u32:$res_y, pv4u32:$res_z, pv4u32:$res_w), [], [], 0, 0>, PRIVATE;
  def mask_soa8_read_imageui_2d_#qual#_no_sampler : OclBuiltin<"mask_soa8_read_imageui", (outs),
                (ins v8i32:$mask, !cast<OclType>("image2d_"#qual):$image, v8i32:$coord_x, v8i32:$coord_y,
                     pv8u32:$res_x, pv8u32:$res_y, pv8u32:$res_z, pv8u32:$res_w), [], [], 0, 0>, PRIVATE;
  }
}

foreach qual = [ "wo", "rw" ] in {
  let Types = [ !cast<OclType>("image2d_"#qual) ],
      Attrs = [CONSTFUNC, OVERLOADABLE] in {
  def soa4_write_imageui_2d_#qual : OclBuiltin<"soa4_write_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v4i32:$coord_x, v4i32:$coord_y,
                     v4u32:$val_x, v4u32:$val_y, v4u32:$val_z, v4u32:$val_w), [], [], 0, 0>;
  def soa8_write_imageui_2d_#qual : OclBuiltin<"soa8_write_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v8i32:$coord_x, v8i32:$coord_y,
                     v8u32:$val_x, v8u32:$val_y, v8u32:$val_z, v8u32:$val_w), [], [], 0, 0>;
  def soa16_write_imageui_2d_#qual : OclBuiltin<"soa16_write_imageui", (outs),
                (ins !cast<OclType>("image2d_"#qual):$image, v16i32:$coord_x, v16i32:$coord_y,
                     v16u32:$val_x, v16u32:$val_y, v16u32:$val_z, v16u32:$val_w), [], [], 0, 0>;
  def mask_soa4_write_imageui_2d_#qual : OclBuiltin<"mask_soa4_write_imageui", (outs),
                (ins v4i32:$mask, !cast<OclType>("image2d_"#qual):$image, v4i32:$coord_x, v4i32:$coord_y,
                     v4u32:$val_x, v4u32:$val_y, v4u32:$val_z, v4u32:$val_w), [], [], 0, 0>;
  def mask_soa8_write_imageui_2d_#qual : OclBuiltin<"mask_soa8_write_imageui", (outs),
                (ins v8i32:$mask, !cast<OclType>("image2d_"#qual):$image, v8i32:$coord_x, v8i32:$coord_y,
                     v8u32:$val_x, v8u32:$val_y, v8u32:$val_z, v8u32:$val_w), [], [], 0, 0>;
  }
}

// Vectorizer generated table for masked image read/write functions
foreach imagebi = [ "mask_read_imagef_2d_ro", "mask_read_imagef_2d_rw",
                    "mask_read_imagei_2d_ro", "mask_read_imagei_2d_rw" ] in {
  def imagebi#_f_cell : MapCell<!cast<OclBuiltin>(imagebi), v4f32>;
  def imagebi#_f_vectorized : MapRow<[
    !cast<MapCell>(imagebi#_f_cell), //scalar
    nullCell,      //v2
    nullCell,      //v4
    nullCell,      //v8
    nullCell,      //16
    nullCell],     //3
    0,0>;          //not scalarizable, not packetizable
  def imagebi#_i_cell : MapCell<!cast<OclBuiltin>(imagebi), v4i32>;
  def imagebi#_i_vectorized : MapRow<[
    !cast<MapCell>(imagebi#_i_cell), //scalar
    nullCell,      //v2
    nullCell,      //v4
    nullCell,      //v8
    nullCell,      //16
    nullCell],     //3,
    0,0>;          //not scalarizable, not packetizable
}

foreach imagebi = [ "mask_read_imagef_2d_ro_no_sampler", "mask_read_imagef_2d_rw_no_sampler",
                    "mask_read_imagei_2d_ro_no_sampler", "mask_read_imagei_2d_rw_no_sampler" ] in {
  def imagebi#_f_cell : MapCell<!cast<OclBuiltin>(imagebi), v4i32>;
  def imagebi#_f_vectorized : MapRow<[
    !cast<MapCell>(imagebi#_f_cell), //scalar
    nullCell,      //v2
    nullCell,      //v4
    nullCell,      //v8
    nullCell,      //16
    nullCell],     //3
    0,0>;          //not scalarizable, not packetizable
}

foreach imagebi = [ "mask_write_imagef_2d_wo", "mask_write_imagef_2d_rw",
                    "mask_write_imagei_2d_wo", "mask_write_imagei_2d_rw" ] in {
  def imagebi#_i_cell : MapCell<!cast<OclBuiltin>(imagebi), v4i32>;
  def imagebi#_i_vectorized : MapRow<[
    !cast<MapCell>(imagebi#_i_cell), //scalar
    nullCell,      //v2
    nullCell,      //v4
    nullCell,      //v8
    nullCell,      //16
    nullCell],     //3
    0,0>;          //not scalarizable, not packetizable
}

foreach qual = [ "ro", "rw"] in {
  def mask_read_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_read_imageui_2d_"#qual), v4i32>;
  def mask_soa4_read_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_soa4_read_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def mask_soa8_read_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_soa8_read_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def mask_read_imageui_2d_#qual#_i_vectorized : MapRow<[
    !cast<MapCell>("mask_read_imageui_2d_"#qual#"_i_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("mask_soa4_read_imageui_2d_"#qual#"_i_cell"),      //v4
    !cast<MapCell>("mask_soa8_read_imageui_2d_"#qual#"_i_cell"),      //v8
    nullCell,      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable
}

foreach qual = [ "ro", "rw"] in {
  def mask_read_imageui_2d_#qual#_no_sampler_i_cell : MapCell<!cast<OclBuiltin>("mask_read_imageui_2d_"#qual#"_no_sampler"), v4i32>;
  def mask_soa4_read_imageui_2d_#qual#_no_sampler_i_cell : MapCell<!cast<OclBuiltin>("mask_soa4_read_imageui_2d_"#qual#"_no_sampler"), !cast<OclType>("image2d_"#qual)>;
  def mask_soa8_read_imageui_2d_#qual#_no_sampler_i_cell : MapCell<!cast<OclBuiltin>("mask_soa8_read_imageui_2d_"#qual#"_no_sampler"), !cast<OclType>("image2d_"#qual)>;
  def mask_read_imageui_2d_#qual#_no_sampler_i_vectorized : MapRow<[
    !cast<MapCell>("mask_read_imageui_2d_"#qual#"_no_sampler_i_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("mask_soa4_read_imageui_2d_"#qual#"_no_sampler_i_cell"),      //v4
    !cast<MapCell>("mask_soa8_read_imageui_2d_"#qual#"_no_sampler_i_cell"),      //v8
    nullCell,      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable
}

foreach qual = [ "wo", "rw"] in {
  def mask_write_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_write_imageui_2d_"#qual), v4i32>;
  def mask_soa4_write_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_soa4_write_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def mask_soa8_write_imageui_2d_#qual#_i_cell : MapCell<!cast<OclBuiltin>("mask_soa8_write_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def mask_write_imageui_2d_#qual#_i_vectorized : MapRow<[
    !cast<MapCell>("mask_write_imageui_2d_"#qual#"_i_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("mask_soa4_write_imageui_2d_"#qual#"_i_cell"),      //v4
    !cast<MapCell>("mask_soa8_write_imageui_2d_"#qual#"_i_cell"),      //v8
    nullCell,      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable
}

// Vectorizer generated table for image properties functions
foreach qual = [ "ro", "wo", "rw" ] in {
  foreach imagebi = [ "image2d_", "image3d_", "image2d_array_" ] in {
    def imagebi#qual#_get_image_height_cell : MapCell<!cast<OclBuiltin>("get_image_height_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa4_get_image_height_cell : MapCell<!cast<OclBuiltin>("soa4_get_image_height_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa8_get_image_height_cell : MapCell<!cast<OclBuiltin>("soa8_get_image_height_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_get_image_height_vectorized : MapRow<[
      !cast<MapCell>(imagebi#qual#"_get_image_height_cell"), //scalar
      nullCell,      //v2
      !cast<MapCell>(imagebi#qual#"_soa4_get_image_height_cell"),      //v4
      !cast<MapCell>(imagebi#qual#"_soa8_get_image_height_cell"),      //v8
      nullCell,      //16
      nullCell],     //3
      0,1>;          //not scalarizable, packetizable
  }

  foreach imagebi = [ "image1d_", "image2d_", "image3d_", "image1d_array_", "image1d_buffer_", "image2d_array_" ] in {
    def imagebi#qual#_get_image_width_cell : MapCell<!cast<OclBuiltin>("get_image_width_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa4_get_image_width_cell : MapCell<!cast<OclBuiltin>("soa4_get_image_width_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa8_get_image_width_cell : MapCell<!cast<OclBuiltin>("soa8_get_image_width_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_get_image_width_vectorized : MapRow<[
      !cast<MapCell>(imagebi#qual#_get_image_width_cell), //scalar
      nullCell,      //v2
      !cast<MapCell>(imagebi#qual#_soa4_get_image_width_cell),      //v4
      !cast<MapCell>(imagebi#qual#_soa8_get_image_width_cell),      //v8
      nullCell,      //16
      nullCell],     //3
      0,1>;          //not scalarizable, packetizable

    def imagebi#qual#_get_image_channel_order_cell : MapCell<!cast<OclBuiltin>("get_image_channel_order_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa4_get_image_channel_order_cell : MapCell<!cast<OclBuiltin>("soa4_get_image_channel_order_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa8_get_image_channel_order_cell : MapCell<!cast<OclBuiltin>("soa8_get_image_channel_order_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_get_image_channel_order_vectorized : MapRow<[
      !cast<MapCell>(imagebi#qual#_get_image_channel_order_cell), //scalar
      nullCell,      //v2
      !cast<MapCell>(imagebi#qual#_soa4_get_image_channel_order_cell),      //v4
      !cast<MapCell>(imagebi#qual#_soa8_get_image_channel_order_cell),      //v8
      nullCell,      //16
      nullCell],     //3
      0,1>;          //not scalarizable, packetizable

    def imagebi#qual#_get_image_channel_data_type_cell : MapCell<!cast<OclBuiltin>("get_image_channel_data_type_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa4_get_image_channel_data_type_cell : MapCell<!cast<OclBuiltin>("soa4_get_image_channel_data_type_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_soa8_get_image_channel_data_type_cell : MapCell<!cast<OclBuiltin>("soa8_get_image_channel_data_type_"#qual), !cast<OclType>(imagebi#qual)>;
    def imagebi#qual#_get_image_channel_data_type_vectorized : MapRow<[
      !cast<MapCell>(imagebi#qual#_get_image_channel_data_type_cell), //scalar
      nullCell,      //v2
      !cast<MapCell>(imagebi#qual#_soa4_get_image_channel_data_type_cell),      //v4
      !cast<MapCell>(imagebi#qual#_soa8_get_image_channel_data_type_cell),      //v8
      nullCell,      //16
      nullCell],     //3
      0,1>;          //not scalarizable, packetizable
  }

  def get_image_depth_cell_#qual : MapCell<!cast<OclBuiltin>("get_image_depth_"#qual), !cast<OclType>("image3d_"#qual)>;
  def soa4_get_image_depth_cell_#qual : MapCell<!cast<OclBuiltin>("soa4_get_image_depth_"#qual), !cast<OclType>("image3d_"#qual)>;
  def soa8_get_image_depth_cell_#qual : MapCell<!cast<OclBuiltin>("soa8_get_image_depth_"#qual), !cast<OclType>("image3d_"#qual)>;
  def get_image_depth_vectorized_#qual : MapRow<[
    !cast<MapCell>("get_image_depth_cell_"#qual), //scalar
    nullCell,      //v2
    !cast<MapCell>("soa4_get_image_depth_cell_"#qual),      //v4
    !cast<MapCell>("soa8_get_image_depth_cell_"#qual),      //v8
    nullCell,      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable
}

// Vectorizer generated table for image read/write functions
foreach qual = [ "ro", "rw" ] in {
  def read_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("read_imageui_2d_"#qual), v4i32>;
  def soa4_read_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa4_read_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def soa8_read_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa8_read_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def soa16_read_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa16_read_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def read_imageui_2d_#qual#_vectorized : MapRow<[
    !cast<MapCell>("read_imageui_2d_"#qual#"_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("soa4_read_imageui_2d_"#qual#"_cell"),      //v4
    !cast<MapCell>("soa8_read_imageui_2d_"#qual#"_cell"),      //v8
    !cast<MapCell>("soa16_read_imageui_2d_"#qual#"_cell"),     //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable

  def read_imageui_2d_#qual#_no_sampler_cell : MapCell<!cast<OclBuiltin>("read_imageui_2d_"#qual#"_no_sampler"), v4i32>;
  def soa4_read_imageui_2d_#qual#_no_sampler_cell : MapCell<!cast<OclBuiltin>("soa4_read_imageui_2d_"#qual#"_no_sampler"), !cast<OclType>("image2d_"#qual)>;
  def soa8_read_imageui_2d_#qual#_no_sampler_cell : MapCell<!cast<OclBuiltin>("soa8_read_imageui_2d_"#qual#"_no_sampler"), !cast<OclType>("image2d_"#qual)>;
  def soa16_read_imageui_2d_#qual#_no_sampler_cell : MapCell<!cast<OclBuiltin>("soa16_read_imageui_2d_"#qual#"_no_sampler"), !cast<OclType>("image2d_"#qual)>;
  def read_imageui_2d_#qual#_no_sampler__vectorized : MapRow<[
    !cast<MapCell>("read_imageui_2d_"#qual#"_no_sampler_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("soa4_read_imageui_2d_"#qual#"_no_sampler_cell"),      //v4
    !cast<MapCell>("soa8_read_imageui_2d_"#qual#"_no_sampler_cell"),      //v8
    !cast<MapCell>("soa16_read_imageui_2d_"#qual#"_no_sampler_cell"),      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable
}

foreach qual = [ "wo", "rw" ] in {
  def write_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("write_imageui_2d_"#qual), v4i32>;
  def soa4_write_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa4_write_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def soa8_write_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa8_write_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def soa16_write_imageui_2d_#qual#_cell : MapCell<!cast<OclBuiltin>("soa16_write_imageui_2d_"#qual), !cast<OclType>("image2d_"#qual)>;
  def write_imageui_2d_#qual#_vectorized : MapRow<[
    !cast<MapCell>("write_imageui_2d_"#qual#"_cell"), //scalar
    nullCell,      //v2
    !cast<MapCell>("soa4_write_imageui_2d_"#qual#"_cell"),      //v4
    !cast<MapCell>("soa8_write_imageui_2d_"#qual#"_cell"),      //v8
    !cast<MapCell>("soa16_write_imageui_2d_"#qual#"_cell"),      //16
    nullCell],     //3
    0,1>;          //not scalarizable, packetizable

}

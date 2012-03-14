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
def isfinite        : OclBuiltin<"isfinite",        (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isinf           : OclBuiltin<"isinf",           (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isnan           : OclBuiltin<"isnan",           (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
def isnormal        : OclBuiltin<"isnormal",        (outs rf2igentype:$ret), (ins gentype:$x, gentype:$y), [], []>;
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

// vim:ts=2:sw=2:et:

def outs;
def ins;

class VectorSuffixString<int len> {
  string Value = !if(!eq(len,  1),   "",
                 !if(!eq(len,  2),  "2",
                 !if(!eq(len,  3),  "3",
                 !if(!eq(len,  4),  "4",
                 !if(!eq(len,  8),  "8",
                 !if(!eq(len, 16), "16",
                 !if(!eq(len, 32), "32",
                 !if(!eq(len, 64), "64",
                 !if(!eq(len, 128), "128",
                 !if(!eq(len, 256), "256",
                 "__invalid__"))))))))));
}

class VectorVTypeSuffixString<string base, int len> {
  string Length = !if(!eq(len,  1),  "v1",
                  !if(!eq(len,  2),  "v2",
                  !if(!eq(len,  3),  "v3",
                  !if(!eq(len,  4),  "v4",
                  !if(!eq(len,  8),  "v8",
                  !if(!eq(len, 16), "v16",
                  !if(!eq(len, 32), "v32",
                  !if(!eq(len, 64), "v64",
                  !if(!eq(len, 128), "v128",
                  !if(!eq(len, 256), "v256",
                  "__invalid__"))))))))));
  string TypeName = !subst("double",  "f64",
                    !subst("long",    "i64",
                    !subst("ulong",   "u64",
                    !subst("float",   "f32",
                    !subst("half",    "f16",
                    !subst("int",     "i32",
                    !subst("uint",    "u32",
                    !subst("short",   "i16",
                    !subst("ushort",  "u16",
                    !subst("char",    "i8",
                    !subst("uchar",   "u8",
                      base)))))))))));
  string Value = !strconcat(Length, TypeName);
}

class VectorSVMLSuffixString<string base, int len> {
  string Length = !if(!eq(len,  1),  "1",
                  !if(!eq(len,  2),  "2",
                  !if(!eq(len,  3),  "3",
                  !if(!eq(len,  4),  "4",
                  !if(!eq(len,  8),  "8",
                  !if(!eq(len, 16), "16",
                  "__invalid__"))))));
  string TypeName = !subst("double",  "",
                    !subst("long",    "",
                    !subst("ulong",   "",
                    !subst("float",   "f",
                    !subst("int",     "f",
                    !subst("uint",    "f",
                    !subst("short",   "f",
                    !subst("ushort",  "f",
                    !subst("char",    "f",
                    !subst("uchar",   "f",
                      base))))))))));
  string Value = !strconcat(TypeName, Length);
}

// FIXME: SVMLDSuffix/SVMLFSuffix are added as hack without refactoring rewrite
// pattern seriously.
// Here are some initial thoughts on rewrite patterns:
// + rewrite pattern should be enclosed by '${' and '}' to enable it being
//   embedded even for the following case:
//
//    foo${PAT}bar.
//
//   the current design cannot handle this as it assume the delimiter is
//   non-alphanum. It also enable the put almost anything in the enclosed text.
//
// + rewrite patterns have two categories, i.e. with parameters or not, e.g.
//   'Ret' refers to the return variable and has no extra parameters, 'Arg'
//   refers to the Nth input argument and has one extra parameter to specify
//   the order of input arguments. One comprehensive example would be:
//
//    ${igentype(Arg(1))}
//
//   will be rewritten into the type of the 1st argument (counting from 0)
//   after type generator igentype.
//
//   All type generators specified in the *.td file could be used the rewrite
//   pattern with one extra parameter of the input type. In addition,
//   'nngentype' is used to get the next native type of the given input type.
//
// With such refactoring, lots of existing rewrite patterns will be replaced as
// the combination of these basic ones. E.g.,
//
// $ReturnType == ${type(Ret)}
// $ReturnName == ${name(Ret)}
// $Arg0VecType == ${vgentype(type(Arg(0)),len(Arg(0)))}
// $SVMLSuffix == ${svmlsuffix(type(Ret))} or ${svmlsuffix(type(Arg(0)))}
//
// It will be accurate to specify the type of either argument type or return
// type for some rewrite patterns, such as SVMLSuffix, where we have to play
// workaround in existing code.
//
class VectorSVMLDSuffixString<string base, int len> {
  string Length = !if(!eq(len,  1),  "1",
                  !if(!eq(len,  2),  "2",
                  !if(!eq(len,  3),  "3",
                  !if(!eq(len,  4),  "4",
                  !if(!eq(len,  8),  "8",
                  !if(!eq(len, 16), "16",
                  "__invalid__"))))));
  string TypeName = !subst("double",  "",
                    !subst("long",    "",
                    !subst("ulong",   "",
                    !subst("float",   "",
                    !subst("int",     "",
                    !subst("uint",    "",
                    !subst("short",   "",
                    !subst("ushort",  "",
                    !subst("char",    "",
                    !subst("uchar",   "",
                      base))))))))));
  string Value = !strconcat(TypeName, Length);
}

class VectorSVMLFSuffixString<string base, int len> {
  string Length = !if(!eq(len,  1),  "1",
                  !if(!eq(len,  2),  "2",
                  !if(!eq(len,  3),  "3",
                  !if(!eq(len,  4),  "4",
                  !if(!eq(len,  8),  "8",
                  !if(!eq(len, 16), "16",
                  "__invalid__"))))));
  string TypeName = !subst("double",  "f",
                    !subst("long",    "f",
                    !subst("ulong",   "f",
                    !subst("float",   "f",
                    !subst("int",     "f",
                    !subst("uint",    "f",
                    !subst("short",   "f",
                    !subst("ushort",  "f",
                    !subst("char",    "f",
                    !subst("uchar",   "f",
                      base))))))))));
  string Value = !strconcat(TypeName, Length);
}

class OclType<string name, string base, int length, int width, bit ptr> {
  string TypeName = name;
  string BaseCTypeName = base;
  int VecLength = length;
  int BitWidth = width;
  string Suffix = VectorSuffixString<VecLength>.Value;
  string SVMLSuffix = VectorSVMLSuffixString<BaseCTypeName, VecLength>.Value;
  string SVMLDSuffix = VectorSVMLDSuffixString<BaseCTypeName, VecLength>.Value;
  string SVMLFSuffix = VectorSVMLFSuffixString<BaseCTypeName, VecLength>.Value;
  string VTypeSuffix = VectorVTypeSuffixString<BaseCTypeName, VecLength>.Value;
  string CTypeName = !strconcat(BaseCTypeName, Suffix);
  bit IsPtr = ptr;
}

multiclass _OCLTYPE1_<string ss, int len, int width, string base> {
  def  #NAME# : OclType<ss, base, len, width, 0>;
  def p#NAME# : OclType<!strconcat("p", ss), base, len, width, 1>;
  def gp#NAME# : OclType<!strconcat("p", ss), !strconcat("__global ", base), len, width, 1>;
  def lp#NAME# : OclType<!strconcat("p", ss), !strconcat("__local ", base), len, width, 1>;
  def pp#NAME# : OclType<!strconcat("p", ss), !strconcat("__private ", base), len, width, 1>;
  def cgp#NAME# : OclType<!strconcat("p", ss), !strconcat("const __global ", base), len, width, 1>;
  def clp#NAME# : OclType<!strconcat("p", ss), !strconcat("const __local ", base), len, width, 1>;
  def cpp#NAME# : OclType<!strconcat("p", ss), !strconcat("const __private ", base), len, width, 1>;
  def vgp#NAME# : OclType<!strconcat("p", ss), !strconcat("volatile __global ", base), len, width, 1>;
  def vlp#NAME# : OclType<!strconcat("p", ss), !strconcat("volatile __local ", base), len, width, 1>;
  def vpp#NAME# : OclType<!strconcat("p", ss), !strconcat("volatile __private ", base), len, width, 1>;
}

multiclass _OCLTYPE2_<string ss, int width, string base> {
  defm  v1#NAME# : _OCLTYPE1_<!strconcat( "v1", ss),  1, width, base>;
  defm  v2#NAME# : _OCLTYPE1_<!strconcat( "v2", ss),  2, width, base>;
  defm  v3#NAME# : _OCLTYPE1_<!strconcat( "v3", ss),  3, width, base>;
  defm  v4#NAME# : _OCLTYPE1_<!strconcat( "v4", ss),  4, width, base>;
  defm  v8#NAME# : _OCLTYPE1_<!strconcat( "v8", ss),  8, width, base>;
  defm v16#NAME# : _OCLTYPE1_<!strconcat("v16", ss), 16, width, base>;
  defm v32#NAME# : _OCLTYPE1_<!strconcat("v32", ss), 32, width, base>;
  defm v64#NAME# : _OCLTYPE1_<!strconcat("v64", ss), 64, width, base>;
  defm v128#NAME# : _OCLTYPE1_<!strconcat("v128", ss), 128, width, base>;
  defm v256#NAME# : _OCLTYPE1_<!strconcat("v256", ss), 256, width, base>;
}
defm i8  : _OCLTYPE2_<"i8",  8, "char">;
defm u8  : _OCLTYPE2_<"u8",  8, "uchar">;
defm i16 : _OCLTYPE2_<"i16", 16, "short">;
defm u16 : _OCLTYPE2_<"u16", 16, "ushort">;
defm i32 : _OCLTYPE2_<"i32", 32, "int">;
defm u32 : _OCLTYPE2_<"u32", 32, "uint">;
defm i64 : _OCLTYPE2_<"i64", 64, "long">;
defm u64 : _OCLTYPE2_<"u64", 64, "ulong">;
defm f16 : _OCLTYPE2_<"f16", 16, "half">;
defm f32 : _OCLTYPE2_<"f32", 32, "float">;
defm f64 : _OCLTYPE2_<"f64", 64, "double">;
def size_t : OclType<"size_t", "size_t", 1, 0, 0>;
def uint  : OclType<"uint", "uint", 1, 0, 0>;
def int32   : OclType<"int", "int", 1, 0, 0>;
defm intptr_t : _OCLTYPE1_<"intptr_t", 1, 0, "intptr_t">;
defm uintptr_t : _OCLTYPE1_<"uintptr_t", 1, 0, "uintptr_t">;
def ptrdiff_t : OclType<"ptrdiff_t", "ptrdiff_t", 1, 0, 0>;
def cl_mem_fence_flags : OclType<"cl_mem_fence_flags", "cl_mem_fence_flags", 1, 0, 0>;
def image1d_ro: OclType<"image1d_ro", "__read_only image1d_t", 1, 0, 0>;
def image1d_wo: OclType<"image1d_wo", "__write_only image1d_t", 1, 0, 0>;
def image1d_rw: OclType<"image1d_rw", "__read_write image1d_t", 1, 0, 0>;
def image2d_ro: OclType<"image2d_ro", "__read_only image2d_t", 1, 0, 0>;
def image2d_wo: OclType<"image2d_wo", "__write_only image2d_t", 1, 0, 0>;
def image2d_rw: OclType<"image2d_rw", "__read_write image2d_t", 1, 0, 0>;
def image3d_ro: OclType<"image3d_ro", "__read_only image3d_t", 1, 0, 0>;
def image3d_wo: OclType<"image3d_wo", "__write_only image3d_t", 1, 0, 0>;
def image3d_rw: OclType<"image3d_rw", "__read_write image3d_t", 1, 0, 0>;
def image1d_buffer_ro: OclType<"image1d_buffer_ro", "__read_only image1d_buffer_t", 1, 0, 0>;
def image1d_buffer_wo: OclType<"image1d_buffer_wo", "__write_only image1d_buffer_t", 1, 0, 0>;
def image1d_buffer_rw: OclType<"image1d_buffer_rw", "__read_write image1d_buffer_t", 1, 0, 0>;
def image1d_array_ro: OclType<"image1d_array_ro", "__read_only image1d_array_t", 1, 0, 0>;
def image1d_array_wo: OclType<"image1d_array_wo", "__write_only image1d_array_t", 1, 0, 0>;
def image1d_array_rw: OclType<"image1d_array_rw", "__read_write image1d_array_t", 1, 0, 0>;
def image2d_array_ro: OclType<"image2d_array_ro", "__read_only image2d_array_t", 1, 0, 0>;
def image2d_array_wo: OclType<"image2d_array_wo", "__write_only image2d_array_t", 1, 0, 0>;
def image2d_array_rw: OclType<"image2d_array_rw", "__read_write image2d_array_t", 1, 0, 0>;
def sampler_t : OclType<"sampler_t", "sampler_t", 1, 0, 0>;
def event_t : OclType<"event_t", "event_t", 1, 0, 0>;
def pevent_t : OclType<"pevent_t", "event_t", 1, 0, 1>;
def void: OclType<"void", "void", 1, 0, 0>;
defm atom_v1i32 : _OCLTYPE1_<"atom_v1i32", 1, 32, "atomic_int">;
defm atom_v1u32 : _OCLTYPE1_<"atom_v1u32", 1, 32, "atomic_uint">;
defm atom_v1f32 : _OCLTYPE1_<"atom_v1f32", 1, 32, "atomic_float">;
defm atom_flag  : _OCLTYPE1_<"atom_flag", 1, 32, "atomic_flag">;
defm atom_v1i64 : _OCLTYPE1_<"atom_v1i64", 1, 64, "atomic_long">;
defm atom_v1u64 : _OCLTYPE1_<"atom_v1u64", 1, 64, "atomic_ulong">;
defm atom_v1f64 : _OCLTYPE1_<"atom_v1f64", 1, 64, "atomic_double">;
defm atom_intptr_t : _OCLTYPE1_<"atom_intptr_t", 1, 0, "atomic_intptr_t">;
defm atom_uintptr_t : _OCLTYPE1_<"atom_uintptr_t", 1, 0, "atomic_uintptr_t">;
def memory_order : OclType<"memory_order", "memory_order", 1, 32, 0>;
def memory_scope : OclType<"memory_scope", "memory_scope", 1, 32, 0>;
def bool : OclType<"bool", "bool", 1, 32, 0>;
def write_only_pipe : OclType<" write_only_pipe", "write_only pipe uchar", 1, 0, 0>;
def read_only_pipe : OclType<" read_only_pipe", "read_only pipe uchar", 1, 0, 0>;

// Custom types
def char32 : OclType<"char32", "__v32qi", 1, 0, 0>;
def pchar32 : OclType<"pchar32", "__v32qi", 1, 0, 1>;

class TypeNameList<list<string> names> {
  list<string> Names = names;
}
def ocl_types : TypeNameList<[
   v1i8.TypeName,  v2i8.TypeName,  v3i8.TypeName,  v4i8.TypeName,  v8i8.TypeName,  v16i8.TypeName,
   v1u8.TypeName,  v2u8.TypeName,  v3u8.TypeName,  v4u8.TypeName,  v8u8.TypeName,  v16u8.TypeName,
  v1i16.TypeName, v2i16.TypeName, v3i16.TypeName, v4i16.TypeName, v8i16.TypeName, v16i16.TypeName,
  v1u16.TypeName, v2u16.TypeName, v3u16.TypeName, v4u16.TypeName, v8u16.TypeName, v16u16.TypeName,
  v1i32.TypeName, v2i32.TypeName, v3i32.TypeName, v4i32.TypeName, v8i32.TypeName, v16i32.TypeName,
  v1u32.TypeName, v2u32.TypeName, v3u32.TypeName, v4u32.TypeName, v8u32.TypeName, v16u32.TypeName,
  v1i64.TypeName, v2i64.TypeName, v3i64.TypeName, v4i64.TypeName, v8i64.TypeName, v16i64.TypeName,
  v1u64.TypeName, v2u64.TypeName, v3u64.TypeName, v4u64.TypeName, v8u64.TypeName, v16u64.TypeName,
  v1f16.TypeName, v2f16.TypeName, v3f16.TypeName, v4f16.TypeName, v8f16.TypeName, v16f16.TypeName,
  v1f32.TypeName, v2f32.TypeName, v3f32.TypeName, v4f32.TypeName, v8f32.TypeName, v16f32.TypeName,
  v1f64.TypeName, v2f64.TypeName, v3f64.TypeName, v4f64.TypeName, v8f64.TypeName, v16f64.TypeName,

  intptr_t.TypeName, uintptr_t.TypeName,

  atom_v1i32.TypeName, atom_v1u32.TypeName, atom_v1f32.TypeName, atom_flag.TypeName,
  atom_v1i64.TypeName, atom_v1u64.TypeName, atom_v1f64.TypeName,
  atom_intptr_t.TypeName, atom_uintptr_t.TypeName,

  // Wide vectors for shuffles
  v32i64.TypeName, v64i64.TypeName, v128i64.TypeName, v256i64.TypeName,
  v32u64.TypeName, v64u64.TypeName, v128u64.TypeName, v256u64.TypeName,
  v32i32.TypeName, v64i32.TypeName, v128i32.TypeName, v256i32.TypeName,
  v32u32.TypeName, v64u32.TypeName, v128u32.TypeName, v256u32.TypeName,
  v32f32.TypeName, v64f32.TypeName, v128f32.TypeName, v256f32.TypeName,
  v32i16.TypeName, v64i16.TypeName, v128i16.TypeName, v256i16.TypeName,
  v32u16.TypeName, v64u16.TypeName, v128u16.TypeName, v256u16.TypeName,
  v32i8.TypeName,  v64i8.TypeName,  v128i8.TypeName, v256i8.TypeName,
  v32u8.TypeName,  v64u8.TypeName,  v128u8.TypeName, v256u8.TypeName,

  image1d_ro.TypeName, image1d_wo.TypeName, image1d_rw.TypeName,
  image2d_ro.TypeName, image2d_wo.TypeName, image2d_rw.TypeName,
  image3d_ro.TypeName, image3d_wo.TypeName, image3d_rw.TypeName,
  image1d_buffer_ro.TypeName, image1d_buffer_wo.TypeName, image1d_buffer_rw.TypeName,
  image1d_array_ro.TypeName, image1d_array_wo.TypeName, image1d_array_rw.TypeName,
  image2d_array_ro.TypeName, image2d_array_wo.TypeName, image2d_array_rw.TypeName,

  sampler_t.TypeName, event_t.TypeName,

  //The following is needed due to KNC conversions
   pv1i8.TypeName,  pv2i8.TypeName,  pv3i8.TypeName,  pv4i8.TypeName,  pv8i8.TypeName,  pv16i8.TypeName,
   pv1u8.TypeName,  pv2u8.TypeName,  pv3u8.TypeName,  pv4u8.TypeName,  pv8u8.TypeName,  pv16u8.TypeName,
  pv1i16.TypeName, pv2i16.TypeName, pv3i16.TypeName, pv4i16.TypeName, pv8i16.TypeName, pv16i16.TypeName,
  pv1u16.TypeName, pv2u16.TypeName, pv3u16.TypeName, pv4u16.TypeName, pv8u16.TypeName, pv16u16.TypeName,
  pv1i32.TypeName, pv2i32.TypeName, pv3i32.TypeName, pv4i32.TypeName, pv8i32.TypeName, pv16i32.TypeName,
  pv1u32.TypeName, pv2u32.TypeName, pv3u32.TypeName, pv4u32.TypeName, pv8u32.TypeName, pv16u32.TypeName,
  pv1i64.TypeName, pv2i64.TypeName, pv3i64.TypeName, pv4i64.TypeName, pv8i64.TypeName, pv16i64.TypeName,
  pv1u64.TypeName, pv2u64.TypeName, pv3u64.TypeName, pv4u64.TypeName, pv8u64.TypeName, pv16u64.TypeName,
  pv1f16.TypeName, pv2f16.TypeName, pv3f16.TypeName, pv4f16.TypeName, pv8f16.TypeName, pv16f16.TypeName,
  pv1f32.TypeName, pv2f32.TypeName, pv3f32.TypeName, pv4f32.TypeName, pv8f32.TypeName, pv16f32.TypeName,
  pv1f64.TypeName, pv2f64.TypeName, pv3f64.TypeName, pv4f64.TypeName, pv8f64.TypeName, pv16f64.TypeName,

  //The following are needed for atom2gentype
  patom_v1i32.TypeName, patom_v1u32.TypeName, patom_v1f32.TypeName,
  patom_v1i64.TypeName, patom_v1u64.TypeName, patom_v1f64.TypeName,
  patom_intptr_t.TypeName, patom_uintptr_t.TypeName
]>;

class OclGenType<string name>
  : OclType<name, name, 1, 0, 0> {
  list<string> Torig = ocl_types.Names;
}
// gentype: 'vNtM' -> 'vNtM'
def gentype : OclGenType<"gentype"> {
  list<string> Tin = Torig;
  list<string> Tout = Tin;
}
// converts a type to a pointer
def pgentype : OclGenType<"pgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Torig,  !strconcat("p", n));
}
// sgentype: 'vNtM' -> 'v1tM'
def sgentype : OclGenType<"sgentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "v2", "v1", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v3", "v1", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v4", "v1", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v8", "v1", n));
  list<string> T4 = !foreach(n,  T3, !subst("v16", "v1", n));
  list<string> Tout = T4;
}
// psgentype 'vNtM' -> 'pv1tM'
def psgentype : OclGenType<"psgentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "v2", "v1", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v3", "v1", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v4", "v1", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v8", "v1", n));
  list<string> T4 = !foreach(n,  T3, !subst("v16", "v1", n));
  list<string> T5 = !foreach(n,  T4, !strconcat("p", n));
  list<string> Tout = T5;
}
// gentype2: 'vNtM' -> 'v2tM'
def gentype2 : OclGenType<"gentype2"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v16", "vX", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v1", "v2", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v3", "v2", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v4", "v2", n));
  list<string> T4 = !foreach(n,  T3, !subst( "v8", "v2", n));
  list<string> T5 = !foreach(n,  T4, !subst( "vX", "v2", n));
  list<string> Tout = T5;
}
// gentype4: 'vNtM' -> 'v4tM'
def gentype4 : OclGenType<"gentype4"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v16", "vX", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v1", "v4", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v2", "v4", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v3", "v4", n));
  list<string> T4 = !foreach(n,  T3, !subst( "v8", "v4", n));
  list<string> T5 = !foreach(n,  T4, !subst( "vX", "v4", n));
  list<string> Tout = T5;
}
// gentype8: 'vNtM' -> 'v8tM'
def gentype8 : OclGenType<"gentype8"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v16", "vX", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v1", "v8", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v2", "v8", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v3", "v8", n));
  list<string> T4 = !foreach(n,  T3, !subst( "v4", "v8", n));
  list<string> T5 = !foreach(n,  T4, !subst( "vX", "v8", n));
  list<string> Tout = T5;
}
// gentype16: 'vNtM' -> 'v16tM'
def gentype16 : OclGenType<"gentype16"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v16", "vX", n));
  list<string> T1 = !foreach(n,  T0, !subst( "v1", "v16", n));
  list<string> T2 = !foreach(n,  T1, !subst( "v2", "v16", n));
  list<string> T3 = !foreach(n,  T2, !subst( "v3", "v16", n));
  list<string> T4 = !foreach(n,  T3, !subst( "v4", "v16", n));
  list<string> T5 = !foreach(n,  T4, !subst( "v8", "v16", n));
  list<string> T6 = !foreach(n,  T5, !subst( "vX", "v16", n));
  list<string> Tout = T6;
}
// ugentype2: 'vNtM' -> 'v2uM'
def ugentype2 : OclGenType<"ugentype2"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8",  "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "u64", n));
  list<string> T7 = !foreach(n,  T6, !subst("v16", "vX", n));
  list<string> T8 = !foreach(n,  T7, !subst( "v1", "v2", n));
  list<string> T9 = !foreach(n,  T8, !subst( "v2", "v2", n));
  list<string> Ta = !foreach(n,  T9, !subst( "v3", "v2", n));
  list<string> Tb = !foreach(n,  Ta, !subst( "v4", "v2", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8", "v2", n));
  list<string> Td = !foreach(n,  Tc, !subst( "vX", "v2", n));
  list<string> Tout = Td;
}
// ugentype4: 'vNtM' -> 'v4uM'
def ugentype4 : OclGenType<"ugentype4"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8",  "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "u64", n));
  list<string> T7 = !foreach(n,  T6, !subst("v16", "vX", n));
  list<string> T8 = !foreach(n,  T7, !subst( "v1", "v4", n));
  list<string> T9 = !foreach(n,  T8, !subst( "v2", "v4", n));
  list<string> Ta = !foreach(n,  T9, !subst( "v3", "v4", n));
  list<string> Tb = !foreach(n,  Ta, !subst( "v4", "v4", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8", "v4", n));
  list<string> Td = !foreach(n,  Tc, !subst( "vX", "v4", n));
  list<string> Tout = Td;
}
// ugentype8: 'vNtM' -> 'v8uM'
def ugentype8 : OclGenType<"ugentype8"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8",  "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "u64", n));
  list<string> T7 = !foreach(n,  T6, !subst("v16", "vX", n));
  list<string> T8 = !foreach(n,  T7, !subst( "v1", "v8", n));
  list<string> T9 = !foreach(n,  T8, !subst( "v2", "v8", n));
  list<string> Ta = !foreach(n,  T9, !subst( "v3", "v8", n));
  list<string> Tb = !foreach(n,  Ta, !subst( "v4", "v8", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8", "v8", n));
  list<string> Td = !foreach(n,  Tc, !subst( "vX", "v8", n));
  list<string> Tout = Td;
}
// ugentype16: 'vNtM' -> 'v16uM'
def ugentype16 : OclGenType<"ugentype16"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8",  "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "u64", n));
  list<string> T7 = !foreach(n,  T6, !subst("v16", "vX", n));
  list<string> T8 = !foreach(n,  T7, !subst( "v1", "v16", n));
  list<string> T9 = !foreach(n,  T8, !subst( "v2", "v16", n));
  list<string> Ta = !foreach(n,  T9, !subst( "v3", "v16", n));
  list<string> Tb = !foreach(n,  Ta, !subst( "v4", "v16", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8", "v16", n));
  list<string> Td = !foreach(n,  Tc, !subst( "vX", "v16", n));
  list<string> Tout = Td;
}
// ugentype: 'vNiM' -> 'vNuM'
def ugentype : OclGenType<"ugentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8",  "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "u64", n));
  list<string> Tout = T6;
}
// igentype: 'vNuM' -> 'vNiM'
def igentype : OclGenType<"igentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "u8",  "i8", n));
  list<string> T1 = !foreach(n,  T0, !subst("u16", "i16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "i16", n));
  list<string> T3 = !foreach(n,  T2, !subst("u32", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("u64", "i64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "i64", n));
  list<string> Tout = T6;
}
// i8gentype: 'vNtM' -> 'vNi8'
def i8gentype : OclGenType<"i8gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "u8", "i8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "i8", n));
  list<string> T2 = !foreach(n,  T1, !subst("u16", "i8", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "i8", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "i8", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "i8", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i8", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i8", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i8", n));
  list<string> Tout = T8;
}
// u8gentype: 'vNtM' -> 'vNu8'
def u8gentype : OclGenType<"u8gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "u8", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "u8", n));
  list<string> T2 = !foreach(n,  T1, !subst("u16", "u8", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u8", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "u8", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u8", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "u8", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "u8", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "u8", n));
  list<string> Tout = T8;
}
// i16gentype: 'vNtM' -> 'vNi16'
def i16gentype : OclGenType<"i16gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "i16", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "i16", n));
  list<string> T2 = !foreach(n,  T1, !subst("u16", "i16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "i16", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "i16", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "i16", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i16", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i16", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i16", n));
  list<string> Tout = T8;
}
// u16gentype: 'vNtM' -> 'vNu16'
def u16gentype : OclGenType<"u16gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "u16", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "u16", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "u16", n));
  list<string> T3 = !foreach(n,  T2, !subst("i32", "u16", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "u16", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u16", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "u16", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "u16", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "u16", n));
  list<string> Tout = T8;
}
// i32gentype: 'vNtM' -> 'vNi32'
def i32gentype : OclGenType<"i32gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "i32", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "i32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "i32", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "i32", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i32", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i32", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i32", n));
  list<string> Tout = T8;
}
// pi32gentype : 'vNtM' -> 'pvNi32'
def pi32gentype : OclGenType<"pi32gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "i32", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "i32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "i32", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "i32", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i32", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i32", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i32", n));
  list<string> T9 = !foreach(n,  T8, !strconcat("p", n));
  list<string> Tout = T9;
}
// si32gentype: 'vNtM' -> 'v1i32'
def si32gentype : OclGenType<"si32gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "i32", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "i32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "i32", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst("u32", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "i32", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i32", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i32", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i32", n));
  list<string> T9 = !foreach(n,  T8, !subst( "v2", "v1", n));
  list<string> Ta = !foreach(n,  T9, !subst( "v3", "v1", n));
  list<string> Tb = !foreach(n,  Ta, !subst( "v4", "v1", n));
  list<string> Tc = !foreach(n,  Tb, !subst( "v8", "v1", n));
  list<string> Td = !foreach(n,  Tc, !subst("v16", "v1", n));
  list<string> Tout = Td;
}
// u32gentype: 'vNtM' -> 'vNu32'
def u32gentype : OclGenType<"u32gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "u32", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "u32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "u32", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "u32", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "u32", n));
  list<string> T5 = !foreach(n,  T4, !subst("i64", "u32", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "u32", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "u32", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "u32", n));
  list<string> Tout = T8;
}
// i64gentype: 'vNtM' -> 'vNi64'
def i64gentype : OclGenType<"i64gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "i64", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "i64", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "i64", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "i64", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "i64", n));
  list<string> T5 = !foreach(n,  T4, !subst("u32", "i64", n));
  list<string> T6 = !foreach(n,  T5, !subst("u64", "i64", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "i64", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "i64", n));
  list<string> Tout = T8;
}
// u64gentype: 'vNtM' -> 'vNu64'
def u64gentype : OclGenType<"u64gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "u64", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "u64", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "u64", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "u64", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "u64", n));
  list<string> T5 = !foreach(n,  T4, !subst("u32", "u64", n));
  list<string> T6 = !foreach(n,  T5, !subst("i64", "u64", n));
  list<string> T7 = !foreach(n,  T6, !subst("f32", "u64", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "u64", n));
  list<string> Tout = T8;
}
// f32gentype: 'vNtM' -> 'vNf32'
def f32gentype : OclGenType<"f32gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "f32", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "f32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "f32", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "f32", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "f32", n));
  list<string> T5 = !foreach(n,  T4, !subst("u32", "f32", n));
  list<string> T6 = !foreach(n,  T5, !subst("i64", "f32", n));
  list<string> T7 = !foreach(n,  T6, !subst("u64", "f32", n));
  list<string> T8 = !foreach(n,  T7, !subst("f64", "f32", n));
  list<string> Tout = T8;
}
// f64gentype: 'vNtM' -> 'vNf64'
def f64gentype : OclGenType<"f64gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "i8", "f64", n));
  list<string> T1 = !foreach(n,  T0, !subst( "u8", "f64", n));
  list<string> T2 = !foreach(n,  T1, !subst("i16", "f64", n));
  list<string> T3 = !foreach(n,  T2, !subst("u16", "f64", n));
  list<string> T4 = !foreach(n,  T3, !subst("i32", "f64", n));
  list<string> T5 = !foreach(n,  T4, !subst("u32", "f64", n));
  list<string> T6 = !foreach(n,  T5, !subst("i64", "f64", n));
  list<string> T7 = !foreach(n,  T6, !subst("u64", "f64", n));
  list<string> T8 = !foreach(n,  T7, !subst("f32", "f64", n));
  list<string> Tout = T8;
}
// rf2igentype: 'vNf32' -> 'vNi32', 'vNf64' -> 'vNi64', 'v1f32' -> 'v1i32', 'v1f64' -> 'v1i32'
def rf2igentype : OclGenType<"rf2igentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("f32", "i32", n));
  list<string> T1 = !foreach(n,  T0, !subst("f64", "i64", n));
  list<string> T2 = !foreach(n,  T1, !subst("v1i64", "v1i32", n));
  list<string> Tout = T2;
}
// up_gentype: 'vNx8' -> 'vNx16', 'vNx16' -> 'vNx32', 'vNx32' -> 'vNx64'
def up_gentype : OclGenType<"up_gentype"> {
  list<string> Tin = Torig;
  // NOTE: The order is important to prevent a replaced string being replaced again!
  list<string> T0 = !foreach(n, Tin, !subst("f32", "f64", n));
  list<string> T1 = !foreach(n,  T0, !subst("f16", "f32", n));
  list<string> T2 = !foreach(n,  T1, !subst("i32", "i64", n));
  list<string> T3 = !foreach(n,  T2, !subst("u32", "u64", n));
  list<string> T4 = !foreach(n,  T3, !subst("i16", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("u16", "u32", n));
  list<string> T6 = !foreach(n,  T5, !subst( "i8", "i16", n));
  list<string> T7 = !foreach(n,  T6, !subst( "u8", "u16", n));
  list<string> Tout = T7;
}
// up2_gentype: 'vNx8' -> 'vNx32', 'vNx16' -> 'vNx64'
def up2_gentype : OclGenType<"up2_gentype"> {
  list<string> Tin = Torig;
  // NOTE: The order is important to prevent a replaced string being replaced again!
  list<string> T0 = !foreach(n, Tin, !subst("f16", "f64", n));
  list<string> T1 = !foreach(n,  T0, !subst("i16", "i64", n));
  list<string> T2 = !foreach(n,  T1, !subst("u16", "u64", n));
  list<string> T3 = !foreach(n,  T2, !subst( "i8", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst( "u8", "u32", n));
  list<string> Tout = T4;
}
// converts 'wTl' to 'wil*' e.g., v4f32 -> v4i32*, v4f64 -> v4i64*
def pigentype : OclGenType<"pigentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst( "u8",  "i8", n));
  list<string> T1 = !foreach(n,  T0, !subst("u16", "i16", n));
  list<string> T2 = !foreach(n,  T1, !subst("f16", "i16", n));
  list<string> T3 = !foreach(n,  T2, !subst("u32", "i32", n));
  list<string> T4 = !foreach(n,  T3, !subst("f32", "i32", n));
  list<string> T5 = !foreach(n,  T4, !subst("u64", "i64", n));
  list<string> T6 = !foreach(n,  T5, !subst("f64", "i64", n));
  list<string> T7 = !foreach(n, T6, !strconcat("p", n));
  list<string> Tout = T7;
}

def downgentype : OclGenType<"downgentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("f64", "f32", n));
  list<string> T1 = !foreach(n, T0, !subst("i16", "i8", n));
  list<string> T2 = !foreach(n, T1, !subst("i32", "i16", n));
  list<string> Tout = !foreach(n, T2, !subst("i64", "i32", n));
}

//converts the given typed to v2 vNxT -> v2XT e.g. v1f32 -> v2f32
def v2gentype : OclGenType<"v2gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v1", "v2", n));
  list<string> T1 = !foreach(n, T0, !subst("v3", "v2", n));
  list<string> T2 = !foreach(n, T1, !subst("v4", "v2", n));
  list<string> T3 = !foreach(n, T2, !subst("v8", "v2", n));
  list<string> Tout = !foreach(n, T3, !subst("v16", "v2", n));
}
//converts the given typed to v8 vNxT -> v8XT e.g. v1f32 -> v8f32
def v8gentype : OclGenType<"v8gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v1", "v8", n));
  list<string> T1 = !foreach(n, T0, !subst("v2", "v8", n));
  list<string> T2 = !foreach(n, T1, !subst("v3", "v8", n));
  list<string> T3 = !foreach(n, T2, !subst("v4", "v8", n));
  list<string> Tout = !foreach(n, T3, !subst("v16", "v8", n));
}
//converts the given typed to pv8 vNxT -> pv8XT e.g. v1f32 -> pv8f32 (pointer)
def pv8gentype : OclGenType<"pv8gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v1", "v8", n));
  list<string> T1 = !foreach(n, T0, !subst("v2", "v8", n));
  list<string> T2 = !foreach(n, T1, !subst("v3", "v8", n));
  list<string> T3 = !foreach(n, T2, !subst("v4", "v8", n));
  list<string> T4 = !foreach(n, T3, !subst("v16", "v8", n));
  list<string> T5 = !foreach(n, T4, !strconcat("p", n));
  list<string> Tout = T5;
}
//converts the given typed to v16 vNxT -> v16XT e.g. v1f32 -> v16f32
def v16gentype : OclGenType<"v16gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v1", "v16", n));
  list<string> T1 = !foreach(n, T0, !subst("v2", "v16", n));
  list<string> T2 = !foreach(n, T1, !subst("v3", "v16", n));
  list<string> T3 = !foreach(n, T2, !subst("v4", "v16", n));
  list<string> Tout = !foreach(n, T3, !subst("v8", "v16", n));
}
//converts the given typed to pv16 vNxT -> pv16XT e.g. v1f32 -> pv16f32 (pointer)
def pv16gentype : OclGenType<"pv16gentype"> {
  list<string> Tin = Torig;
  list<string> T0 = !foreach(n, Tin, !subst("v1", "v16", n));
  list<string> T1 = !foreach(n, T0, !subst("v2", "v16", n));
  list<string> T2 = !foreach(n, T1, !subst("v3", "v16", n));
  list<string> T3 = !foreach(n, T2, !subst("v4", "v16", n));
  list<string> T4 = !foreach(n, T3, !subst("v8", "v16", n));
  list<string> T5 = !foreach(n, T4, !strconcat("p", n));
  list<string> Tout = T5;
}

//TODO: Improve the below conversions once concat of list is possible.

// converts a type to a pointer atomic type
def patomgentype : OclGenType<"patomgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("patom_", n));
}
// converts a type to a volatile global pointer atomic type
def vgpatomgentype : OclGenType<"vgpatomgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vgpatom_", n));
}
// converts a type to a volatile local pointer atomic type
def vlpatomgentype : OclGenType<"vlpatomgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vlpatom_", n));
}
// converts a type to a volatile private pointer atomic type
def vppatomgentype : OclGenType<"vppatomgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vppatom_", n));
}
// converts an atomic type (or pointer) to its basic type
def atom2gentype : OclGenType<"atom2gentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !subst("atom_", "", n));
}

// the following two gentypes are needed for builtins with the second pointer
// different from the first pointer in the arguments list
// converts a type to a __global pointer
def gpgentype : OclGenType<"gpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("gp", n));
}
// converts a type to a __local pointer
def lpgentype : OclGenType<"lpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("lp", n));
}
// converts a type to a __private pointer
def ppgentype : OclGenType<"ppgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("pp", n));
}
// converts a type to a const __global pointer
def cgpgentype : OclGenType<"cgpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("cgp", n));
}
// converts a type to a const __local pointer
def clpgentype : OclGenType<"clpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("clp", n));
}
// converts a type to a const __private pointer
def cppgentype : OclGenType<"cppgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("cpp", n));
}
// converts a type to a volatile __global pointer
def vgpgentype : OclGenType<"vgpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vgp", n));
}
// converts a type to a volatile __local pointer
def vlpgentype : OclGenType<"vlpgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vlp", n));
}
// converts a type to a volatile __private pointer
def vppgentype : OclGenType<"vppgentype"> {
  list<string> Tin = Torig;
  list<string> Tout = !foreach(n, Tin, !strconcat("vpp", n));
}

// OclBuiltinAttr
class OclBuiltinAttr<string attr> {
  string Attr = attr;
}

def SVMLCC : OclBuiltinAttr<"__attribute__((svmlcc))">;
def CONSTFUNC : OclBuiltinAttr<"__attribute__((const))">;
def OVERLOADABLE : OclBuiltinAttr<"__attribute__((overloadable))">;
def INLINE : OclBuiltinAttr<"__attribute__((always_inline))">;
def INTERNAL_INLINE : OclBuiltinAttr<"inline __attribute__((always_inline))">;

// OclBuiltin
class OclBuiltin<string name, dag outs, dag ins, list<OclType> types, list<OclBuiltinAttr> attrs, bit exclude, bit prefix> {
  string Name = name;
  dag Outs = outs;
  dag Ins = ins;
  list<OclType> Types = types;
  list<OclType> ExceptionTypes = [];
  list<OclType> AdditionTypes = [];
  list<OclBuiltinAttr> Attrs = attrs;
  list<OclBuiltinAttr> ExceptionAttrs = [];
  list<OclBuiltinAttr> AdditionAttrs = [];
  bit IsDeclOnly = 0; // Just need declaration, e.g. SVML function prototype declaration.
  bit NeedForwardDecl = 0; // Non OCL builtin function need forwarding declaration.
  bit Exclude = exclude; // OCL builtin function that could be excluded from the build.
  bit Prefix = prefix; // OCL builtin function that could add prefix to its name.
}

// Helper to add exception type list
class Except<list<OclType> types> {
  list<OclType> ExceptionTypes = types;
}

// Helper to add addition type list
class Plus<list<OclType> types> {
  list<OclType> AdditionTypes = types;
}

// Helper to add exception type list
class ExceptAttr<list<OclBuiltinAttr> attrs> {
  list<OclBuiltinAttr> ExceptionAttrs = attrs;
}

// Helper to add addition type list
class PlusAttr<list<OclBuiltinAttr> attrs> {
  list<OclBuiltinAttr> AdditionAttrs = attrs;
}

// Special property for the pointer type in OCLBuiltin prototype.
class OclAddressSpace<string name, bit hasConst, bit hasVolatile> {
  string ASQualifier = name;
  bit HasConst = hasConst;
  bit HasVolatile = hasVolatile;
}

class PRIVATE       : OclAddressSpace<"__private",  0, 0>;
class LOCAL         : OclAddressSpace<"__local",    0, 0>;
class GLOBAL        : OclAddressSpace<"__global",   0, 0>;
class CONSTANT      : OclAddressSpace<"__constant", 1, 0>;
class CONST_PRIVATE : OclAddressSpace<"__private",  1, 0>;
class CONST_LOCAL   : OclAddressSpace<"__local",    1, 0>;
class CONST_GLOBAL  : OclAddressSpace<"__global",   1, 0>;
class VOLATILE_PRIVATE : OclAddressSpace<"__private",  0, 1>;
class VOLATILE_LOCAL   : OclAddressSpace<"__local",    0, 1>;
class VOLATILE_GLOBAL  : OclAddressSpace<"__global",   0, 1>;


// OclBuiltinImpl
class OclBuiltinImpl<OclBuiltin builtin, list<OclType> types, bit isDeclOnly, code impl, list<list<string>> customMacro = []> {
  OclBuiltin Builtin = builtin;
  list<OclType> Types = types;
  code Impl = impl;
  bit IsDeclOnly = isDeclOnly; // Just need declaration, for OCL builtin implementation defined somewhere else.
  list<list<string>> CustomMacro = customMacro;
}

//Reflection module map cell
class MapCell<OclBuiltin bi, OclType ty>{
  OclBuiltin builtin = bi;
  OclType type = ty;
}

def nullCell : MapCell<?, ?>;

class MapRow< list<MapCell> cl, bit is, bit ip>{
  list<MapCell> cellList = cl;
  bit isScalarizable = is;
  bit isPacketaizable= ip;
}

// TargetImpl
class TargetImpl {
  string Target = "";

  list<OclType> NativeTypes = [];
  // Rewrite patterns
  //
  // $Target
  //
  // $Suffix
  // $SVMLSuffix
  // $VTypeSuffix
  //
  // $Pattern
  // $BitWidth
  // $VecLength
  // $Mask
  // $Func
  // $ReturnType
  // $ReturnVarName
  // $Arg0Type
  // $Arg0VecType
  // $Arg0NoASType
  // $Arg0VarName
  // $Arg1Type
  // $Arg1VecType
  // $Arg1NoASType
  // $Arg1VarName
  // ...
  // $Arg9Type
  // $Arg9VecType
  // $Arg9NoASType
  // $Arg9VarName
  //
  // $NativeFunc
  // $NativeReturnType
  // $NativeArg0Type
  // $NativeArg1Type
  // ..
  // $NativeArg9Type
  // $PtrArg0Type
  // $PtrArg1Type
  // ..
  // $PtrArg9Type
  //
  // $ExpandLoPattern
  // $ExpandHiPattern
  // $ExpandLoFunc
  // $ExpandHiFunc
  //
  // FIXME: do we need them?
  // $ExpandLoReturnType
  // $ExpandHiReturnType
  // $ExpandLoArg0Type
  // $ExpandHiArg0Type
  // $ExpandLoArg1Type
  // $ExpandHiArg1Type
  // ..
  // $ExpandLoArg9Type
  // $ExpandHiArg9Type
  //
}

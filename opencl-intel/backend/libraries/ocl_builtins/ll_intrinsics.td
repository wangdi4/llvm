// vim:ts=2:sw=2:et:
// integer conversion intrinsics in LLVM IR

// sext
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def sext_i16 : OclBuiltin<"sext_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], []>,
               Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                       v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def sext_i32 : OclBuiltin<"sext_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], []>,
               Except<[v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def sext_i64 : OclBuiltin<"sext_$Arg0Symgentype_$ReturnSymgentype", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
}

// zext
let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def zext_i16 : OclBuiltin<"zext_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], []>,
               Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                       v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def zext_i32 : OclBuiltin<"zext_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], []>,
               Except<[v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
def zext_i64 : OclBuiltin<"zext_$Arg0Symgentype_$ReturnSymgentype", (outs i64gentype:$ret), (ins gentype:$x), [], []>;
}

// trunc
let Types = [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def trunc_i8  : OclBuiltin<"trunc_$Arg0Symgentype_$ReturnSymgentype", (outs  i8gentype:$ret), (ins gentype:$x), [], []>;
def trunc_i16 : OclBuiltin<"trunc_$Arg0Symgentype_$ReturnSymgentype", (outs i16gentype:$ret), (ins gentype:$x), [], []>,
                Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16]>;
def trunc_i32 : OclBuiltin<"trunc_$Arg0Symgentype_$ReturnSymgentype", (outs i32gentype:$ret), (ins gentype:$x), [], []>,
                Except<[v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                        v1i32, v2i32, v3i32, v4i32, v8i32, v16i32]>;
}

let Types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64],
    IsDeclOnly = 1, NeedForwardDecl = 1 in {
def trunc_f32 : OclBuiltin<"trunc_$Arg0Type_$ReturnType", (outs f32gentype:$ret), (ins gentype:$x), [], []>;
}

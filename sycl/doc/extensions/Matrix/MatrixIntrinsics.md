# Proposal for extended matrix type and operations representaion in LLVM IR

Currently we have 3 options to represent matrix type and operations in LLVM IR

## 1. LLVM intrinsics

The intrinsics below are based on the
[matrix instrinsics](https://llvm.org/docs/LangRef.html#matrix-intrinsics) from
the current LLVM language reference. The major difference is that matrix layout
is given as `<matrix layout>` argument, instead of being encoded into
intrinsics name. This argument must be one of the following strings:
```
"matrix.columnmajor"
"matrix.rownmajor"
"matrix.packed_a"
"matrix.packed_b"
```

Sync scope for matrix operations is defined by the last argument in the
intrinsics below and must be one of the following:
```
"scope.subgroup"
"scope.workgroup"
```

### 1.1 LLVM intrinsics with vector types

#### @llvm.experimental.matrix.load.* intrinsic
```
declare vectorty
@llvm.experimental.matrix.load.*(ptrty %Ptr, i64 %Stride, i1 <IsVolatile>,
                                 i32 <Rows>, i32 <Cols>,
                                 metadata <matrix layout>, metadata <scope>);
```

#### @llvm.experimental.matrix.store.* intrinsic
```
declare void
@llvm.experimental.matrix.store.*(vectorty %Matrix, ptrty %Ptr, i64 %Stride,
                                  i1 <IsVolatile>, i32 <Rows>, i32 <Cols>,
                                  metadata <matrix layout>, metadata <scope>);
```

#### @llvm.experimental.matrix.mad.* intrinsic
```
declare vectorty
@llvm.experimental.matrix.mad.*(vectorty %A, metadata <matrix layout>,
                                vectorty %B, metadata <matrix layout>,
                                vectorty %C, metadata <matrix layout>,
                                i32 %M, i32 %K, i32 %N, metadata <scope>)
```

#### Example:
```
define void @example(i32* %Ptr, i64 %Stride) {
    %A = call <12 x i32> @llvm.experimental.matrix.load.v12i32(i32* %Ptr, i64 %Stride, i1 false, i32 3, i32 4, metadata !"matrix.columnmajor", metadata !"scope.subgroup")
    %B = call <20 x i32> @llvm.experimental.matrix.load.v20i32(i32* %Ptr, i64 %Stride, i1 false, i32 4, i32 5, metadata !"matrix.columnmajor", metadata !"scope.subgroup")
    %C = call <15 x i32> @llvm.experimental.matrix.load.v15i32(i32* %Ptr, i64 %Stride, i1 false, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup")
    %D = call <15 x i32> @llvm.experimental.matrix.mad.v12i32.v20i32.v15i32(<12 x i32> %A, metadata !"matrix.columnmajor", <20 x i32> %B, metadata !"matrix.columnmajor", <15 x i32> %C, metadata !"matrix.rowmajor", i32 3, i32 4, i32 5, metadata !"scope.subgroup")
    call void @llvm.experimental.matrix.store.v15i32(<15 x i32> %D, i32* %Ptr, i64 %Stride, i1 0, i32 3, i32 5, metadata !"matrix.rowmajor", metadata !"scope.subgroup")
    ret void
}

declare <12 x i32> @llvm.experimental.matrix.load.v12i32(i32*, i64, i1, i32, i32, metadata, metadata);
declare <20 x i32> @llvm.experimental.matrix.load.v20i32(i32*, i64, i1, i32, i32, metadata, metadata);
declare <15 x i32> @llvm.experimental.matrix.load.v15i32(i32*, i64, i1, i32, i32, metadata, metadata);
declare <15 x i32> @llvm.experimental.matrix.mad.v12i32.v20i32.v15i32(<12 x i32>, metadata, <20 x i32>, metadata, <15 x i32>, metadata, i32, i32, i32, metadata)
declare void @llvm.experimental.matrix.store.v15i32(<15 x i32>, i32*, i64, i1, i32, i32, metadata, metadata);
```


### 1.2. LLVM intrinsics with opaque types

With this option we mix llvm intrinsics with (SPIR-V specific) opaque types,
which may look confusing from upstream to llvm.org perspective.

#### @llvm.experimental.matrix.load.* intrinsic
```
declare %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride>
@llvm.experimental.matrix.load.*(ptrty %Ptr, i64 %Stride, i1 <IsVolatile>,
                                 i32 <Rows>, i32 <Cols>,
                                 metadata <matrix layout>, metadata <scope>);
```

#### @llvm.experimental.matrix.store.* intrinsic
```
declare void
@llvm.experimental.matrix.store.*(%spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %Matrix,
                                  ptrty %Ptr, i64 %Stride,
                                  i1 <IsVolatile>, i32 <Rows>, i32 <Cols>,
                                  metadata <matrix layout>, metadata <scope>);
```

#### @llvm.experimental.matrix.mad.* intrinsic
```
declare %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride>
@llvm.experimental.matrix.mad.*(%spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %A,
                                %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %B,
                                %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %C,
                                i32 %M, i32 %K, i32 %N, metadata <scope>)
```


## 2. SPIR-V friendly LLVM IR

With this option we don't create new llvm intrinsics and therefore we don't need
to upstream it to llvm.org. Here types and function names follow convention
defined in
[SPIRVRepresentationInLLVM](https://github.com/KhronosGroup/SPIRV-LLVM-Translator/blob/master/docs/SPIRVRepresentationInLLVM.rst)
document. Values of `<layout>` and `<scope>` must match with the values defined
in **SPV_INTEL_matrix** extension. Note, the function names below are unmangled,
in LLVM IR they will be mangled.

#### OpMatrixLoadINTEL
```
declare %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride>
__spirv_MatrixLoadINTEL_R{ReturnType}(ptrty %Ptr, i64 %Stride, i1 <IsVolatile>,
                                      i32 <Rows>, i32 <Cols>,
                                      i32 <matrix layout>, i32 <scope>);
```

#### OpMatrixStoreINTEL
```
declare void
__spirv_MatrixStoreINTEL(%spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %Matrix,
                         ptrty %Ptr, i64 %Stride, i1 <IsVolatile>, i32 <Rows>,
                         i32 <Cols>, i32 <layout>, i32 <scope>);
```

#### OpMatrixMadINTEL
```
declare %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride>
__spirv_MatrixMadINTEL(%spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %A,
                       %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %B,
                       %spirv.MatrixINTEL._<type>_<rows>_<cols>_<layout>_<stride> %C,
                       i32 %M, i32 %K, i32 %N, i32 <scope>)
```


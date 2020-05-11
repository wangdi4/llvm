; RUN: opt -S -csa-memop-ordering <%s | FileCheck %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

define void @test_dgprefetch_i1(i1 %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a direct i1 input?
; CHECK-LABEL: test_dgprefetch_i1

  call void @llvm.csa.gated.prefetch.i1(i1 %gate, i8* %addr, i32 0, i32 0)
; CHECK: call void @llvm.csa.inord(i1 %gate)
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.i1(i1, i8*, i32, i32)

define void @test_dgprefetch_i32(i32 %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a i32 input?
; CHECK-LABEL: test_dgprefetch_i32

  call void @llvm.csa.gated.prefetch.i32(i32 %gate, i8* %addr, i32 0, i32 3)
  ; The i32 value needs to be truncated to i1 to make LLVM happy.
; CHECK: %[[TRUNC:[a-z0-9_.]+]] = trunc i32 %gate to i1
; CHECK: call void @llvm.csa.inord(i1 %[[TRUNC]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 3, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.i32(i32, i8*, i32, i32)

define void @test_dgprefetch_float(float %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a float input?
; CHECK-LABEL: test_dgprefetch_float

  call void @llvm.csa.gated.prefetch.f32(float %gate, i8* %addr, i32 1, i32 0)
  ; The float value needs to be bitcasted and truncated to i1 to make LLVM
  ; happy.
; CHECK: %[[CAST:[a-z0-9_.]+]] = bitcast float %gate to i32
; CHECK: %[[TRUNC:[a-z0-9_.]+]] = trunc i32 %[[CAST]] to i1
; CHECK: call void @llvm.csa.inord(i1 %[[TRUNC]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 1, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.f32(float, i8*, i32, i32)

define void @test_dgprefetch_double(double %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a double input?
; CHECK-LABEL: test_dgprefetch_double

  call void @llvm.csa.gated.prefetch.f64(double %gate, i8* %addr, i32 0, i32 0)
  ; The double value needs to be bitcasted and truncated to i1 to make LLVM
  ; happy.
; CHECK: %[[CAST:[a-z0-9_.]+]] = bitcast double %gate to i64
; CHECK: %[[TRUNC:[a-z0-9_.]+]] = trunc i64 %[[CAST]] to i1
; CHECK: call void @llvm.csa.inord(i1 %[[TRUNC]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.f64(double, i8*, i32, i32)

define void @test_dgprefetch_f32x2(<2 x float> %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a __m64f input?
; CHECK-LABEL: test_dgprefetch_f32x2

  call void @llvm.csa.gated.prefetch.v2f32(<2 x float> %gate, i8* %addr, i32 0, i32 0)
  ; The __m64f value needs to be bitcasted and truncated to i1 to make LLVM
  ; happy.
; CHECK: %[[CAST:[a-z0-9_.]+]] = bitcast <2 x float> %gate to i64
; CHECK: %[[TRUNC:[a-z0-9_.]+]] = trunc i64 %[[CAST]] to i1
; CHECK: call void @llvm.csa.inord(i1 %[[TRUNC]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.v2f32(<2 x float>, i8*, i32, i32)

define void @test_dgprefetch_ptr(i32* %gate, i8* %addr) {
; Does data-gated prefetch expansion work with a pointer input?
; CHECK-LABEL: test_dgprefetch_ptr

  call void @llvm.csa.gated.prefetch.p0i32(i32* %gate, i8* %addr, i32 0, i32 0)
  ; The pointer value needs to be ptrtoint casted to i1 to make LLVM happy.
; CHECK: %[[CAST:[a-z0-9_.]+]] = ptrtoint i32* %gate to i1
; CHECK: call void @llvm.csa.inord(i1 %[[CAST]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.p0i32(i32*, i8*, i32, i32)

define void @test_dgprefetch_array([2 x i32] %gate, i8* %addr) {
; Does data-gated prefetch expansion work with an array input?
; CHECK-LABEL: test_dgprefetch_array

  call void @llvm.csa.gated.prefetch.a2i32([2 x i32] %gate, i8* %addr, i32 0, i32 0)
  ; The values should be extracted from the array and all0'd together.
; CHECK: %[[V0:[a-z0-9_.]+]] = extractvalue [2 x i32] %gate, 0
; CHECK: %[[CAST0:[a-z0-9_.]+]] = trunc i32 %[[V0]] to i1
; CHECK: %[[V1:[a-z0-9_.]+]] = extractvalue [2 x i32] %gate, 1
; CHECK: %[[CAST1:[a-z0-9_.]+]] = trunc i32 %[[V1]] to i1
; CHECK: %[[ALL:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %[[CAST0]], i1 %[[CAST1]])
; CHECK: call void @llvm.csa.inord(i1 %[[ALL]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.a2i32([2 x i32], i8*, i32, i32)

define void @test_dgprefetch_struct({i1, i32, i32*, float, <2 x float>} %gate, i8* %addr) {
; Does data-gated prefetch expansion work with an struct input?
; CHECK-LABEL: test_dgprefetch_struct

  call void @llvm.csa.gated.prefetch.sl_i1i32p0i32f32v2f32s({i1, i32, i32*, float, <2 x float>} %gate, i8* %addr, i32 0, i32 0)
  ; The values should be extracted from the struct and all0'd together.
; CHECK: %[[V0:[a-z0-9_.]+]] = extractvalue { i1, i32, i32*, float, <2 x float> } %gate, 0
; CHECK: %[[V1:[a-z0-9_.]+]] = extractvalue { i1, i32, i32*, float, <2 x float> } %gate, 1
; CHECK: %[[CAST1:[a-z0-9_.]+]] = trunc i32 %[[V1]] to i1
; CHECK: %[[V2:[a-z0-9_.]+]] = extractvalue { i1, i32, i32*, float, <2 x float> } %gate, 2
; CHECK: %[[CAST2:[a-z0-9_.]+]] = ptrtoint i32* %[[V2]] to i1
; CHECK: %[[V3:[a-z0-9_.]+]] = extractvalue { i1, i32, i32*, float, <2 x float> } %gate, 3
; CHECK: %[[CAST3:[a-z0-9_.]+]] = bitcast float %[[V3]] to i32
; CHECK: %[[TRUNC3:[a-z0-9_.]+]] = trunc i32 %[[CAST3]] to i1
; CHECK: %[[V4:[a-z0-9_.]+]] = extractvalue { i1, i32, i32*, float, <2 x float> } %gate, 4
; CHECK: %[[CAST4:[a-z0-9_.]+]] = bitcast <2 x float> %[[V4]] to i64
; CHECK: %[[TRUNC4:[a-z0-9_.]+]] = trunc i64 %[[CAST4]] to i1
; CHECK: %[[ALL:[a-z0-9_.]+]] = call i1 (...) @llvm.csa.all0(i1 %[[V0]], i1 %[[CAST1]], i1 %[[CAST2]], i1 %[[TRUNC3]], i1 %[[TRUNC4]])
; CHECK: call void @llvm.csa.inord(i1 %[[ALL]])
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

declare void @llvm.csa.gated.prefetch.sl_i1i32p0i32f32v2f32s({i1, i32, i32*, float, <2 x float>}, i8*, i32, i32)

define void @test_dgprefetch_const(i8* %addr) {
; Does data-gated prefetch expansion work with a constant?
; CHECK-LABEL: test_dgprefetch_const

  call void @llvm.csa.gated.prefetch.i32(i32 0, i8* %addr, i32 0, i32 0)
  ; This should be translated to an unordered prefetch.
; CHECK: call void @llvm.csa.inord(i1 false)
; CHECK-NEXT: @llvm.prefetch.p0i8(i8* %addr, i32 0, i32 0, i32 1)
; CHECK-NEXT: %{{[a-z0-9_.]+}} = call i1 @llvm.csa.outord()
  ret void
}

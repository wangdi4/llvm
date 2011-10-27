;*******************************************
;scalarazer test
;check that mul8 is scalarized
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'mult8.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_clz_func_mul24_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_clz_func_mul24_parameters = appending global [92 x i8] c"float const __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <8 x float> addrspace(1)*)* @func_clz_func_mul24 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_clz_func_mul24_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_func_clz_func_mul24_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_clz_func_mul24(float addrspace(1)* nocapture %in, <8 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = sitofp i32 %1 to float                     ; <float> [#uses=1]
  %3 = insertelement <8 x float> undef, float %2, i32 0 ; <<8 x float>> [#uses=1]
  %4 = getelementptr inbounds float addrspace(1)* %in, i32 %1 ; <float addrspace(1)*> [#uses=1]
  %5 = load float addrspace(1)* %4                ; <float> [#uses=1]
  %6 = insertelement <8 x float> %3, float %5, i32 1 ; <<8 x float>> [#uses=1]
  %7 = insertelement <8 x float> %6, float 1.000000e+000, i32 2 ; <<8 x float>> [#uses=1]
  %8 = insertelement <8 x float> %7, float 2.300000e+001, i32 3 ; <<8 x float>> [#uses=1]
  %9 = insertelement <8 x float> %8, float 2.100000e+001, i32 4 ; <<8 x float>> [#uses=1]
  %10 = insertelement <8 x float> %9, float 3.000000e+000, i32 5 ; <<8 x float>> [#uses=1]
  %11 = insertelement <8 x float> %10, float 4.000000e+000, i32 6 ; <<8 x float>> [#uses=1]
  %12 = insertelement <8 x float> %11, float 4.500000e+001, i32 7 ; <<8 x float>> [#uses=2]
  %13 = fmul <8 x float> %12, %12                 ; <<8 x float>> [#uses=1]
  %14 = getelementptr inbounds <8 x float> addrspace(1)* %out, i32 %1 ; <<8 x float> addrspace(1)*> [#uses=1]
  store <8 x float> %13, <8 x float> addrspace(1)* %14
  ret void
}

declare i32 @get_global_id(i32)
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK: fmul
;CHECK-NOT: fmul <
;CHECK: ret


;*******************************************
;scalarazer test
;check  OpenCL vector mul with float const
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'float_const.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_fmul_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_fmul_parameters = appending global [92 x i8] c"float const __attribute__((address_space(1))) *, float2 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <2 x float> addrspace(1)*)* @fmul to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_fmul_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_fmul_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @fmul(float addrspace(1)* nocapture %in, <2 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = getelementptr inbounds float addrspace(1)* %in, i32 1 ; <float addrspace(1)*> [#uses=1]
  %3 = load float addrspace(1)* %2                ; <float> [#uses=1]
  %4 = fmul float %3, 3.500000e+000               ; <float> [#uses=1]
  %5 = sitofp i32 %1 to float                     ; <float> [#uses=2]
  %6 = fmul float %4, %5                          ; <float> [#uses=1]
  %7 = insertelement <2 x float> undef, float %6, i32 0 ; <<2 x float>> [#uses=1]
  %8 = getelementptr inbounds float addrspace(1)* %in, i32 2 ; <float addrspace(1)*> [#uses=1]
  %9 = load float addrspace(1)* %8                ; <float> [#uses=2]
  %10 = fmul float %9, 3.500000e+000              ; <float> [#uses=2]
  %11 = insertelement <2 x float> %7, float %10, i32 1 ; <<2 x float>> [#uses=1]
  %12 = fmul float %9, 4.500000e+000              ; <float> [#uses=1]
  %13 = fmul float %12, %5                        ; <float> [#uses=1]
  %14 = insertelement <2 x float> undef, float %13, i32 0 ; <<2 x float>> [#uses=1]
  %15 = insertelement <2 x float> %14, float %10, i32 1 ; <<2 x float>> [#uses=1]
  %16 = fmul <2 x float> %11, %15                 ; <<2 x float>> [#uses=1]
  %17 = getelementptr inbounds <2 x float> addrspace(1)* %out, i32 %1 ; <<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %16, <2 x float> addrspace(1)* %17
  ret void
}

declare i32 @get_global_id(i32)


;CHECK-NOT:  fmul <
;CHECK: ret
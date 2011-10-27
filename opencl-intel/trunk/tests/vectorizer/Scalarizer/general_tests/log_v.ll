;scalarazer test
;check  OpenCL vector mul with float const
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll



; ModuleID = 'log_v.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_abs_diff_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_abs_diff_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @func_abs_diff to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_abs_diff_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_func_abs_diff_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_abs_diff(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=4]
  %2 = mul i32 %1, %1                             ; <i32> [#uses=1]
  %3 = sitofp i32 %2 to float                     ; <float> [#uses=1]
  %4 = insertelement <4 x float> <float 2.500000e+001, float undef, float undef, float undef>, float %3, i32 1 ; <<4 x float>> [#uses=1]
  %5 = insertelement <4 x float> %4, float 0x400921FA00000000, i32 2 ; <<4 x float>> [#uses=1]
  %6 = sitofp i32 %1 to float                     ; <float> [#uses=1]
  %7 = insertelement <4 x float> %5, float %6, i32 3 ; <<4 x float>> [#uses=1]
  %8 = tail call <4 x float> @_Z3logDv4_f(<4 x float> %7) nounwind ; <<4 x float>> [#uses=3]
  %9 = extractelement <4 x float> %8, i32 0       ; <float> [#uses=1]
  %10 = extractelement <4 x float> %8, i32 1      ; <float> [#uses=1]
  %11 = fadd float %9, %10                        ; <float> [#uses=1]
  %12 = extractelement <4 x float> %8, i32 2      ; <float> [#uses=1]
  %13 = fadd float %11, %12                       ; <float> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %13, float addrspace(1)* %14
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z3logDv4_f(<4 x float>)

;CHECK: log
;CHECK: log
;CHECK: log
;CHECK: log
;CHECK-NOT: @_Z3logDv4_f(
;CHECK: ret
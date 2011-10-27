
;*******************************************
;scalarazer test
;check  check that vector is scalarazied and packed again before store operation
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'vect_scal_vect.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_abs_diff_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_abs_diff_parameters = appending global [92 x i8] c"float const __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <4 x float> addrspace(1)*)* @func_abs_diff to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_abs_diff_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_func_abs_diff_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_abs_diff(float addrspace(1)* nocapture %in, <4 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = getelementptr inbounds float addrspace(1)* %in, i32 1 ; <float addrspace(1)*> [#uses=1]
  %3 = load float addrspace(1)* %2                ; <float> [#uses=1]
  %4 = insertelement <4 x float> undef, float %3, i32 0 ; <<4 x float>> [#uses=1]
  %5 = getelementptr inbounds float addrspace(1)* %in, i32 2 ; <float addrspace(1)*> [#uses=1]
  %6 = load float addrspace(1)* %5                ; <float> [#uses=1]
  %7 = insertelement <4 x float> %4, float %6, i32 1 ; <<4 x float>> [#uses=1]
  %8 = getelementptr inbounds float addrspace(1)* %in, i32 3 ; <float addrspace(1)*> [#uses=1]
  %9 = load float addrspace(1)* %8                ; <float> [#uses=1]
  %10 = insertelement <4 x float> %7, float %9, i32 2 ; <<4 x float>> [#uses=1]
  %11 = getelementptr inbounds float addrspace(1)* %in, i32 4 ; <float addrspace(1)*> [#uses=1]
  %12 = load float addrspace(1)* %11              ; <float> [#uses=1]
  %13 = insertelement <4 x float> %10, float %12, i32 3 ; <<4 x float>> [#uses=1]
  %14 = getelementptr inbounds <4 x float> addrspace(1)* %out, i32 %1 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %13, <4 x float> addrspace(1)* %14
  ret void
}

declare i32 @get_global_id(i32)


;CHECK: [[NAME1:%[1-9]+]] = load float {{.*}}
;CHECK: [[NAME2:%[1-9]+]] = load float {{.*}}
;CHECK: [[NAME3:%[1-9]+]] = load float {{.*}}
;CHECK: [[NAME4:%[1-9]+]] = load float {{.*}}
;CHECK:  {{.*}}insertelement <4 x float> undef, float [[NAME1]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x float> %temp.vect, float [[NAME2]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x float> %temp.vect{{[1-9]+}}, float [[NAME3]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x float> %temp.vect{{[1-9]+}}, float [[NAME4]]{{.*}}

;CHECK: ret



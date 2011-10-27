;*******************************************
;scalarazer test
;constant parameter for scalarazible functions
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'scalarg_nonscal_func.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_dist_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_dist_parameters = appending global [92 x i8] c"float2 const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float> addrspace(1)*, float addrspace(1)*)* @dist to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_dist_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_dist_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @dist(<2 x float> addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=5]
  %2 = insertelement <4 x i32> undef, i32 %1, i32 0 ; <<4 x i32>> [#uses=1]
  %3 = insertelement <4 x i32> %2, i32 %1, i32 1  ; <<4 x i32>> [#uses=1]
  %4 = insertelement <4 x i32> %3, i32 %1, i32 2  ; <<4 x i32>> [#uses=1]
  %5 = insertelement <4 x i32> %4, i32 %1, i32 3  ; <<4 x i32>> [#uses=1]
  %6 = add nsw <4 x i32> %5, <i32 1564, i32 -4564, i32 0, i32 -9> ; <<4 x i32>> [#uses=3]
  %7 = mul <4 x i32> %6, %6                       ; <<4 x i32>> [#uses=1]
  %8 = bitcast <4 x i32> %6 to <4 x float>        ; <<4 x float>> [#uses=1]
  %9 = bitcast <4 x i32> %7 to <4 x float>        ; <<4 x float>> [#uses=1]
  %10 = tail call float @_Z8distanceU8__vector4fS_(<4 x float> %8, <4 x float> %9) nounwind ; <float> [#uses=1]
  %11 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %10, float addrspace(1)* %11
  ret void
}

declare i32 @get_global_id(i32)

declare float @_Z8distanceU8__vector4fS_(<4 x float>, <4 x float>)

;CHECK: ret
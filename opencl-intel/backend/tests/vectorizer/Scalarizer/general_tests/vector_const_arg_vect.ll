;*******************************************
;scalarazer test
; check const argument vectorizable func
; the mul 8 must be not scalarized
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'vector_const_arg_vect.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_mul_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_mul_parameters = appending global [186 x i8] c"float2 const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, char8 const __attribute__((address_space(1))), char8 const __attribute__((address_space(1)))\00", section "llvm.metadata" ; <[186 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float> addrspace(1)*, float addrspace(1)*, <8 x i8>, <8 x i8>)* @mul to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_mul_locals to i8*), i8* getelementptr inbounds ([186 x i8]* @opencl_mul_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @mul(<2 x float> addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out, <8 x i8> %charArg1, <8 x i8> %charArg2) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = mul <8 x i8> %charArg1, %charArg2          ; <<8 x i8>> [#uses=1]
  %3 = extractelement <8 x i8> %2, i32 0          ; <i8> [#uses=1]
  %4 = sext i8 %3 to i32                          ; <i32> [#uses=1]
  %5 = add nsw i32 %4, 1                          ; <i32> [#uses=1]
  %6 = sitofp i32 %5 to float                     ; <float> [#uses=1]
  %7 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %6, float addrspace(1)* %7
  ret void
}

declare i32 @get_global_id(i32)

;CHECK: mul <8 x i8> %charArg1, %charArg2
;CHECK: ret
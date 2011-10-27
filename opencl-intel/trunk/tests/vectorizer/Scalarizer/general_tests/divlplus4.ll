;*******************************************
;scalarazer test
;check div4, add4 will scaralized
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = divlplus4'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_mul_vector_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_mul_vector_parameters = appending global [93 x i8] c"float4 const __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @mul_vector to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_mul_vector_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_mul_vector_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @mul_vector(<4 x float> addrspace(1)* nocapture %in, <4 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = getelementptr inbounds <4 x float> addrspace(1)* %in, i32 %1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %3 = load <4 x float> addrspace(1)* %2          ; <<4 x float>> [#uses=1]
  %4 = fdiv <4 x float> %3, <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000> ; <<4 x float>> [#uses=1]
  %5 = fadd <4 x float> %4, <float 3.000000e+000, float 3.000000e+000, float 3.000000e+000, float 3.000000e+000> ; <<4 x float>> [#uses=1]
  %6 = getelementptr inbounds <4 x float> addrspace(1)* %out, i32 %1 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %5, <4 x float> addrspace(1)* %6
  ret void
}

declare i32 @get_global_id(i32)
;CHECK-NOT: fdiv <
;CHECK-NOT: fadd <
;CHECK: ret
;*******************************************
;scalarazer test
;Argument is vector and was already encountered but not converted to scalars.
; and second function is not in the hash
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'encounterd_not_scalar.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_encountered_not_scal_variable_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_encountered_not_scal_variable_parameters = appending global [92 x i8] c"float2 const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float> addrspace(1)*, float addrspace(1)*)* @encountered_not_scal_variable to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_encountered_not_scal_variable_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_encountered_not_scal_variable_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @encountered_not_scal_variable(<2 x float> addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = getelementptr inbounds <2 x float> addrspace(1)* %in, i32 1 ; <<2 x float> addrspace(1)*> [#uses=1]
  %3 = load <2 x float> addrspace(1)* %2          ; <<2 x float>> [#uses=2]
  %4 = tail call float @_Z8distanceU8__vector2fS_(<2 x float> %3, <2 x float> <float 3.140000e+002, float 3.140000e+002>) nounwind ; <float> [#uses=2]
  %5 = fmul float %4, 2.000000e+000               ; <float> [#uses=1]
  %6 = tail call float @_Z3dotU8__vector2fS_(<2 x float> %3, <2 x float> <float 3.140000e+002, float 3.140000e+002>) nounwind ; <float> [#uses=1]
  %7 = fmul float %6, %4                          ; <float> [#uses=1]
  %8 = fsub float %7, %5                          ; <float> [#uses=1]
  %9 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %8, float addrspace(1)* %9
  ret void
}

declare i32 @get_global_id(i32)

declare float @_Z8distanceU8__vector2fS_(<2 x float>, <2 x float>)

declare float @_Z3dotU8__vector2fS_(<2 x float>, <2 x float>)


;CHECK: @_Z8distanceU8__vector2fS_
;CHECK: ret
;CHECK: @_Z3dotU8__vector2fS_(

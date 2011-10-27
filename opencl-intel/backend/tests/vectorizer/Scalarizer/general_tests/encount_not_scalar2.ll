;*******************************************
;scalarazer test
;Argument is vector and was already encountered but not converted to scalars.
; and second function is in the hash
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'encount_not_scalar2.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_encountered_not_scal_variable_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_encountered_not_scal_variable_parameters = appending global [58 x i8] c"float4 const *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[58 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float>*, float addrspace(1)*)* @encountered_not_scal_variable to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_encountered_not_scal_variable_locals to i8*), i8* getelementptr inbounds ([58 x i8]* @opencl_encountered_not_scal_variable_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @encountered_not_scal_variable(<4 x float>* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = getelementptr inbounds <4 x float>* %in, i32 4 ; <<4 x float>*> [#uses=1]
  %3 = load <4 x float>* %2                       ; <<4 x float>> [#uses=2]
  %4 = sitofp i32 %1 to float                     ; <float> [#uses=1]
  %5 = insertelement <4 x float> undef, float %4, i32 0 ; <<4 x float>> [#uses=1]
  %6 = insertelement <4 x float> %5, float 1.000000e+000, i32 1 ; <<4 x float>> [#uses=1]
  %7 = insertelement <4 x float> %6, float 2.000000e+000, i32 2 ; <<4 x float>> [#uses=1]
  %8 = insertelement <4 x float> %7, float 3.000000e+000, i32 3 ; <<4 x float>> [#uses=2]
  %9 = tail call float @_Z8distanceDv4_fS_(<4 x float> %3, <4 x float> %8) nounwind ; <float> [#uses=2]
  %10 = fmul float %9, 7.000000e+000              ; <float> [#uses=1]
  %11 = tail call <4 x float> @_Z4fmodDv4_fS_(<4 x float> %3, <4 x float> %8) nounwind ; <<4 x float>> [#uses=1]
  %12 = extractelement <4 x float> %11, i32 2     ; <float> [#uses=1]
  %13 = fmul float %12, %9                        ; <float> [#uses=1]
  %14 = fsub float %13, %10                       ; <float> [#uses=1]
  %15 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %14, float addrspace(1)* %15
  ret void
}

declare i32 @get_global_id(i32)

declare float @_Z8distanceDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z4fmodDv4_fS_(<4 x float>, <4 x float>)



;CHECK: @_Z8distanceDv4_fS_(
;CHECK: @_Z4fmodff( 
;CHECK: @_Z4fmodff(
;CHECK: @_Z4fmodff(
;CHECK: @_Z4fmodff(

;CHECK: ret
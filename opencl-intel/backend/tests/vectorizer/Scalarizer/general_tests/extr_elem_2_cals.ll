
;*******************************************
;scalarazer test
;ExtractElement tests
;2 calls, in first call Input vector was not converted to scalars,
;second call with same instruction.
;Test first call adds instructions to convert vector into scalars scalar
; value into vectored value and adds the scalar values to SCM, 
;second instruction should use the scalar values in SCM.
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll




; ModuleID = 'extr_elem_2_cals.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_encountered_not_scal_variable_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_encountered_not_scal_variable_parameters = appending global [69 x i8] c"float *, uchar4 *, uchar __attribute__((address_space(1))) *, float4\00", section "llvm.metadata" ; <[69 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float*, <4 x i8>*, i8 addrspace(1)*, <4 x float>)* @encountered_not_scal_variable to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_encountered_not_scal_variable_locals to i8*), i8* getelementptr inbounds ([69 x i8]* @opencl_encountered_not_scal_variable_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @encountered_not_scal_variable(float* nocapture %in1, <4 x i8>* nocapture %in2, i8 addrspace(1)* nocapture %out, <4 x float> %fArg) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = extractelement <4 x float> %fArg, i32 0    ; <float> [#uses=1]
  %3 = extractelement <4 x float> %fArg, i32 1    ; <float> [#uses=2]
  %4 = fadd float %2, %3                          ; <float> [#uses=1]
  %5 = extractelement <4 x float> %fArg, i32 0    ; <float> [#uses=1]
  %6 = fmul float %5, %3                          ; <float> [#uses=1]
  %7 = fadd float %4, %6                          ; <float> [#uses=1]
  %8 = fptoui float %7 to i8                      ; <i8> [#uses=1]
  %9 = getelementptr inbounds i8 addrspace(1)* %out, i32 %1; <i8 addrspace(1)*> [#uses=1]
  store i8 %8, i8 addrspace(1)* %9
  ret void
}

declare i32 @get_global_id(i32)


;CHECK: extractelement
;CHECK: extractelement
;CHECK: extractelement
;CHECK: extractelement
;CHECK: fadd
;CHECK: fmul

;CHECK: ret
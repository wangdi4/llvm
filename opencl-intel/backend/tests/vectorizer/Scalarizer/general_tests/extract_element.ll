
;*******************************************
;scalarazer test
;ExtractElement tests
;Input vector was already converted to scalars
;Test if vector has scalar values, the scalar values are the ones that are used.
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll



; ModuleID = 'extract_element.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_encountered_not_scal_variable_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_encountered_not_scal_variable_parameters = appending global [62 x i8] c"uchar4 *, uchar4 *, uchar __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[62 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x i8>*, <4 x i8>*, i8 addrspace(1)*)* @encountered_not_scal_variable to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_encountered_not_scal_variable_locals to i8*), i8* getelementptr inbounds ([62 x i8]* @opencl_encountered_not_scal_variable_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @encountered_not_scal_variable(<4 x i8>* nocapture %in1, <4 x i8>* nocapture %in2, i8 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = load <4 x i8>* %in1                        ; <<4 x i8>> [#uses=2]
  %3 = load <4 x i8>* %in2                        ; <<4 x i8>> [#uses=1]
  %4 = mul <4 x i8> %2, %3                        ; <<4 x i8>> [#uses=2]
  %5 = tail call <4 x i8> @_Z5rhaddDv4_hS_(<4 x i8> %4, <4 x i8> %2) nounwind ; <<4 x i8>> [#uses=2]
  %6 = extractelement <4 x i8> %5, i32 0          ; <i8> [#uses=1]
  %7 = extractelement <4 x i8> %5, i32 1          ; <i8> [#uses=1]
  %8 = extractelement <4 x i8> %4, i32 3          ; <i8> [#uses=1]
  %9 = add i8 %7, %8                              ; <i8> [#uses=1]
  %10 = add i8 %9, %6                             ; <i8> [#uses=1]
  %11 = getelementptr inbounds i8 addrspace(1)* %out, i32 %1 ; <i8 addrspace(1)*> [#uses=1]
  store i8 %10, i8 addrspace(1)* %11
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x i8> @_Z5rhaddDv4_hS_(<4 x i8>, <4 x i8>)
;CHECK: @_Z5rhadd
;CHECK-NOT: extractelement
;CHECK: ret
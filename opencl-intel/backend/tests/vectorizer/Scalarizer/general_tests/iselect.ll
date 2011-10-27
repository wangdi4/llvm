;*******************************************
;scalarazer test
;check  OpenCL select product function
;this function is not in the hash and will not scalarizable
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'iselect.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_abs_diff_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_abs_diff_parameters = appending global [92 x i8] c"double const __attribute__((address_space(1))) *, long4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (double addrspace(1)*, <4 x i64> addrspace(1)*)* @func_abs_diff to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_abs_diff_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_func_abs_diff_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_abs_diff(double addrspace(1)* nocapture %in, <4 x i64> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = tail call <4 x i64> @_Z6selectU8__vector4lS_S_(<4 x i64> <i64 123, i64 58, i64 96, i64 -12>, <4 x i64> <i64 123, i64 58, i64 96, i64 -12>, <4 x i64> <i64 123, i64 58, i64 96, i64 -12>) nounwind ; <<4 x i64>> [#uses=1]
  %3 = getelementptr inbounds <4 x i64> addrspace(1)* %out, i32 %1 ; <<4 x i64> addrspace(1)*> [#uses=1]
  store <4 x i64> %2, <4 x i64> addrspace(1)* %3
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x i64> @_Z6selectU8__vector4lS_S_(<4 x i64>, <4 x i64>, <4 x i64>)


;CHECK: @_Z6selectU8__vector4lS_S_
;CHECK: ret
;CHECK: _Z6selectU8__vector4lS_S_
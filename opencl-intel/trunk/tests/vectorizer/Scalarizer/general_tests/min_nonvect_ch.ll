;*******************************************
;scalarazer test
;Tests the OpenCL min function when it recieves ushort4 constants.
;This function is NOT vectorizable.
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: 


; ModuleID = 'min_nonvect_ch.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_min_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_min_parameters = appending global [92 x i8] c"float const __attribute__((address_space(1))) *, float2 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[92 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <2 x float> addrspace(1)*)* @func_min to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_min_locals to i8*), i8* getelementptr inbounds ([92 x i8]* @opencl_func_min_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_min(float addrspace(1)* nocapture %in, <2 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = tail call <4 x i16> @_Z3minU8__vector4tS_(<4 x i16> <i16 1232, i16 47, i16 0, i16 1>, <4 x i16> <i16 -30906, i16 765, i16 2, i16 9283>) nounwind ; <<4 x i16>> [#uses=1]
  %3 = bitcast <4 x i16> %2 to <2 x float>        ; <<2 x float>> [#uses=1]
  %4 = getelementptr inbounds <2 x float> addrspace(1)* %out, i32 %1 ; <<2 x float> addrspace(1)*> [#uses=1]
  store <2 x float> %3, <2 x float> addrspace(1)* %4
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x i16> @_Z3minU8__vector4tS_(<4 x i16>, <4 x i16>)


;CHECK: @_Z3minU8__vector4tS_
;CHECK: reta
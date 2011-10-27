;*******************************************
;scalarazer test
;check  OpenCL rotate function when it recieves uchar constants vectors
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: 

; ModuleID = 'func_rotate_char.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_rotate_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_rotate_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @func_rotate to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_rotate_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_func_rotate_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_rotate(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = tail call zeroext i8 @_Z6rotatehh(i8 zeroext 82, i8 zeroext 19) nounwind ; <i8> [#uses=1]
  %3 = tail call <4 x i8> @_Z6rotateDv4_hS_(<4 x i8> <i8 22, i8 22, i8 22, i8 22>, <4 x i8> <i8 22, i8 22, i8 22, i8 22>) nounwind ; <<4 x i8>> [#uses=1]
  %4 = extractelement <4 x i8> %3, i32 1          ; <i8> [#uses=1]
  %5 = zext i8 %4 to i32                          ; <i32> [#uses=1]
  %6 = zext i8 %2 to i32                          ; <i32> [#uses=1]
  %7 = add nsw i32 %5, %6                         ; <i32> [#uses=1]
  %8 = sitofp i32 %7 to float                     ; <float> [#uses=1]
  %9 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %8, float addrspace(1)* %9
  ret void
}

declare i32 @get_global_id(i32)

declare zeroext i8 @_Z6rotatehh(i8 zeroext, i8 zeroext)

declare <4 x i8> @_Z6rotateDv4_hS_(<4 x i8>, <4 x i8>)


;CHECK: @_Z6rotatehh
;CHECK: @_Z6rotateDv4_hS_
;CHECK: ret
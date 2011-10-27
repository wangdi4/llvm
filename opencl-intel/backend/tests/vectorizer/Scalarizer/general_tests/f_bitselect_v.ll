;*******************************************
;scalarazer test
;check  OpenCL bitselect function when it recieves global uchar constants vectors - vectorasible
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; XFAIL: 
; ModuleID = 'f_bitselect_v.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_bitselect_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_bitselect_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @func_bitselect to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_bitselect_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_func_bitselect_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_bitselect(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = insertelement <2 x i32> undef, i32 %1, i32 0 ; <<2 x i32>> [#uses=1]
  %3 = add nsw i32 %1, 3                          ; <i32> [#uses=1]
  %4 = insertelement <2 x i32> %2, i32 %3, i32 1  ; <<2 x i32>> [#uses=4]
  %5 = tail call <2 x i32> @_Z9bitselectU8__vector2iS_S_(<2 x i32> %4, <2 x i32> <i32 3, i32 10>, <2 x i32> %4) nounwind ; <<2 x i32>> [#uses=1]
  %6 = extractelement <2 x i32> %5, i32 0         ; <i32> [#uses=1]
  %7 = tail call <2 x i32> @_Z9bitselectU8__vector2iS_S_(<2 x i32> %4, <2 x i32> <i32 3, i32 10>, <2 x i32> %4) nounwind ; <<2 x i32>> [#uses=1]
  %8 = extractelement <2 x i32> %7, i32 1         ; <i32> [#uses=1]
  %9 = shl i32 %8, 1                              ; <i32> [#uses=1]
  %10 = add nsw i32 %9, %6                        ; <i32> [#uses=1]
  %11 = sitofp i32 %10 to float                   ; <float> [#uses=1]
  %12 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %11, float addrspace(1)* %12
  ret void
}

declare i32 @get_global_id(i32)

declare <2 x i32> @_Z9bitselectU8__vector2iS_S_(<2 x i32>, <2 x i32>, <2 x i32>)


;CHECK-NOT: @_Z9bitselectU8__vector2iS_S_
;CHECK: ret
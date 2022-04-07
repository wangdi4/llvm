;*******************************************
;scalarazer test
;check  OpenCL absdiff + min function when it recieves global uchar constants vectors - vectorasible
;
;*******************************************


; RUN: %oclopt  -runtimelib %p/../../Full/runtime.bc -O3 -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'f_abs_diff.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_abs_diff_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_abs_diff_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @func_abs_diff to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_abs_diff_locals to i8*), i8* getelementptr inbounds ([91 x i8], [91 x i8]* @opencl_func_abs_diff_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_abs_diff(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind ; <i32> [#uses=1]
  %2 = tail call <2 x i64> @_Z8abs_diffDv2_mS_(<2 x i64> <i64 -21145345543, i64 14535365159567>, <2 x i64> <i64 -21145345543, i64 14535365159567>) nounwind ; <<2 x i64>> [#uses=1]
  %3 = extractelement <2 x i64> %2, i32 0         ; <i64> [#uses=1]
  %4 = tail call <2 x i64> @_Z3minDv2_mS_(<2 x i64> <i64 -21145345543, i64 14535365159567>, <2 x i64> <i64 -21145345543, i64 14535365159567>) nounwind ; <<2 x i64>> [#uses=1]
  %5 = extractelement <2 x i64> %4, i32 1         ; <i64> [#uses=1]
  %6 = mul i64 %5, %3                             ; <i64> [#uses=1]
  %7 = uitofp i64 %6 to float                     ; <float> [#uses=1]
  %8 = getelementptr inbounds float, float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %7, float addrspace(1)* %8
  ret void
}

declare i32 @_Z13get_global_idj(i32)

declare <2 x i64> @_Z8abs_diffDv2_mS_(<2 x i64>, <2 x i64>)

declare <2 x i64> @_Z3minDv2_mS_(<2 x i64>, <2 x i64>)

;CHECK: abs_diff
;CHECK: abs_diff
;CHECK: min
;CHECK: min
;CHECK-NOT: @_Z8abs_diffDv2_mS_(
;CHECK-NOT: @_Z3minDv2_mS_
;CHECK: ret
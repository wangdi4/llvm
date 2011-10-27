;*******************************************
;scalarazer test
;Tests the LLVM vicmp instruction when it recieves ulong2 constant.
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll



; ModuleID = 'vicmp.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_vicmp_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_vicmp_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @vicmp to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_vicmp_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_vicmp_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @vicmp(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = load float addrspace(1)* %in               ; <float> [#uses=1]
  %3 = fptoui float %2 to i64                     ; <i64> [#uses=2]
  %4 = insertelement <2 x i64> undef, i64 %3, i32 0 ; <<2 x i64>> [#uses=1]
  %5 = insertelement <2 x i64> %4, i64 %3, i32 1  ; <<2 x i64>> [#uses=1]
  %6 = icmp eq <2 x i64> %5, <i64 -21145345543, i64 14535365159567> ; <<2 x i1>> [#uses=1]
  %7 = sext <2 x i1> %6 to <2 x i64>              ; <<2 x i64>> [#uses=2]
  %8 = extractelement <2 x i64> %7, i32 0         ; <i64> [#uses=1]
  %9 = extractelement <2 x i64> %7, i32 1         ; <i64> [#uses=1]
  %10 = add nsw i64 %8, %9                        ; <i64> [#uses=1]
  %11 = sitofp i64 %10 to float                   ; <float> [#uses=1]
  %12 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %11, float addrspace(1)* %12
  ret void
}

declare i32 @get_global_id(i32)

;CHECK-NOT: icmp eq <
;CHECK: ret
;*******************************************
;scalarazer test
;Tests the LLVM udiv instruction when it recieves uchar8 constant
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize   -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'invertcolors.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_and_short2_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_and_short2_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @and_short2 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_and_short2_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_and_short2_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @and_short2(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = getelementptr inbounds float addrspace(1)* %in, i32 1 ; <float addrspace(1)*> [#uses=1]
  %3 = load float addrspace(1)* %2                ; <float> [#uses=1]
  %4 = fptosi float %3 to i16                     ; <i16> [#uses=1]
  %5 = insertelement <2 x i16> undef, i16 %4, i32 0 ; <<2 x i16>> [#uses=1]
  %6 = shufflevector <2 x i16> %5, <2 x i16> undef, <2 x i32> zeroinitializer ; <<2 x i16>> [#uses=1]
  %7 = and <2 x i16> %6, <i16 3, i16 -7>          ; <<2 x i16>> [#uses=2]
  %8 = extractelement <2 x i16> %7, i32 0         ; <i16> [#uses=1]
  %9 = sext i16 %8 to i32                         ; <i32> [#uses=1]
  %10 = extractelement <2 x i16> %7, i32 1        ; <i16> [#uses=1]
  %11 = sext i16 %10 to i32                       ; <i32> [#uses=1]
  %12 = add nsw i32 %9, %11                       ; <i32> [#uses=1]
  %13 = sitofp i32 %12 to float                   ; <float> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %13, float addrspace(1)* %14
  ret void
}

declare i32 @get_global_id(i32)
;CHECK-NOT: and <
;CHECK: ret
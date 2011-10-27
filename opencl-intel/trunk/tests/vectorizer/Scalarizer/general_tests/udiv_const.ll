;*******************************************
;scalarazer test
;Tests the LLVM udiv instruction when it recieves uchar8 constant
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'udiv_const.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_udiv_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_udiv_parameters = appending global [91 x i8] c"float const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[91 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*)* @udiv to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_udiv_locals to i8*), i8* getelementptr inbounds ([91 x i8]* @opencl_udiv_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @udiv(float addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = getelementptr inbounds float addrspace(1)* %in, i32 1 ; <float addrspace(1)*> [#uses=1]
  %3 = load float addrspace(1)* %2                ; <float> [#uses=1]
  %4 = fptoui float %3 to i8                      ; <i8> [#uses=1]
  %5 = insertelement <8 x i8> undef, i8 %4, i32 0 ; <<8 x i8>> [#uses=1]
  %6 = shufflevector <8 x i8> %5, <8 x i8> undef, <8 x i32> zeroinitializer ; <<8 x i8>> [#uses=1]
  %7 = udiv <8 x i8> %6, select (<8 x i1> zeroinitializer, <8 x i8> <i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1>, <8 x i8> <i8 54, i8 78, i8 3, i8 90, i8 102, i8 2, i8 9, i8 1>) ; <<8 x i8>> [#uses=8]
  %8 = extractelement <8 x i8> %7, i32 0          ; <i8> [#uses=1]
  %9 = zext i8 %8 to i32                          ; <i32> [#uses=1]
  %10 = extractelement <8 x i8> %7, i32 1         ; <i8> [#uses=1]
  %11 = zext i8 %10 to i32                        ; <i32> [#uses=1]
  %12 = extractelement <8 x i8> %7, i32 2         ; <i8> [#uses=1]
  %13 = zext i8 %12 to i32                        ; <i32> [#uses=1]
  %14 = extractelement <8 x i8> %7, i32 3         ; <i8> [#uses=1]
  %15 = zext i8 %14 to i32                        ; <i32> [#uses=1]
  %16 = extractelement <8 x i8> %7, i32 4         ; <i8> [#uses=1]
  %17 = zext i8 %16 to i32                        ; <i32> [#uses=1]
  %18 = extractelement <8 x i8> %7, i32 5         ; <i8> [#uses=1]
  %19 = zext i8 %18 to i32                        ; <i32> [#uses=1]
  %20 = extractelement <8 x i8> %7, i32 6         ; <i8> [#uses=1]
  %21 = zext i8 %20 to i32                        ; <i32> [#uses=1]
  %22 = extractelement <8 x i8> %7, i32 7         ; <i8> [#uses=1]
  %23 = zext i8 %22 to i32                        ; <i32> [#uses=1]
  %24 = add nsw i32 %21, %23                      ; <i32> [#uses=1]
  %25 = add nsw i32 %24, %19                      ; <i32> [#uses=1]
  %26 = add nsw i32 %25, %17                      ; <i32> [#uses=1]
  %27 = add nsw i32 %26, %15                      ; <i32> [#uses=1]
  %28 = add nsw i32 %27, %13                      ; <i32> [#uses=1]
  %29 = add nsw i32 %28, %11                      ; <i32> [#uses=1]
  %30 = add nsw i32 %29, %9                       ; <i32> [#uses=1]
  %31 = sitofp i32 %30 to float                   ; <float> [#uses=1]
  %32 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %31, float addrspace(1)* %32
  ret void
}

declare i32 @get_global_id(i32)

;CHECK-NOT: udiv <
;CHECK: ret

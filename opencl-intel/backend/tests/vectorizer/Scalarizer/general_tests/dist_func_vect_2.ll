;*******************************************
;scalarazer test
;check that mul8 is scalrazible
; and dist is not scalarazable
;
;*******************************************
 
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll



; ModuleID = 'dist_func_vect_2.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_vect_distance_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_vect_distance_parameters = appending global [90 x i8] c"long const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[90 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i64 addrspace(1)*, float addrspace(1)*)* @func_vect_distance to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_vect_distance_locals to i8*), i8* getelementptr inbounds ([90 x i8]* @opencl_func_vect_distance_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_vect_distance(i64 addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = getelementptr inbounds i64 addrspace(1)* %in, i32 3 ; <i64 addrspace(1)*> [#uses=1]
  %3 = load i64 addrspace(1)* %2                  ; <i64> [#uses=2]
  %4 = sitofp i64 %3 to double                    ; <double> [#uses=2]
  %5 = insertelement <4 x double> undef, double %4, i32 0 ; <<4 x double>> [#uses=1]
  %6 = shufflevector <4 x double> %5, <4 x double> undef, <4 x i32> zeroinitializer ; <<4 x double>> [#uses=1]
  %7 = insertelement <8 x double> undef, double %4, i32 0 ; <<8 x double>> [#uses=1]
  %8 = shufflevector <8 x double> %7, <8 x double> undef, <8 x i32> zeroinitializer ; <<8 x double>> [#uses=2]
  %9 = fmul <8 x double> %8, %8                   ; <<8 x double>> [#uses=1]
  %10 = shufflevector <8 x double> %9, <8 x double> undef, <2 x i32> <i32 0, i32 1> ; <<2 x double>> [#uses=2]
  %11 = load i64 addrspace(1)* %in                ; <i64> [#uses=1]
  %12 = sext i32 %1 to i64                        ; <i64> [#uses=2]
  %13 = mul i64 %11, %12                          ; <i64> [#uses=1]
  %14 = sitofp i64 %13 to double                  ; <double> [#uses=1]
  %15 = insertelement <4 x double> undef, double %14, i32 0 ; <<4 x double>> [#uses=1]
  %16 = add nsw i64 %3, %12                       ; <i64> [#uses=1]
  %17 = sitofp i64 %16 to double                  ; <double> [#uses=1]
  %18 = insertelement <4 x double> %15, double %17, i32 1 ; <<4 x double>> [#uses=1]
  %19 = getelementptr inbounds i64 addrspace(1)* %in, i32 2 ; <i64 addrspace(1)*> [#uses=1]
  %20 = load i64 addrspace(1)* %19                ; <i64> [#uses=1]
  %21 = sitofp i64 %20 to double                  ; <double> [#uses=1]
  %22 = insertelement <4 x double> %18, double %21, i32 2 ; <<4 x double>> [#uses=1]
  %23 = insertelement <4 x double> %22, double 1.200000e+001, i32 3 ; <<4 x double>> [#uses=1]
  %24 = tail call double @_Z8distanceU8__vector4dS_(<4 x double> %6, <4 x double> %23) nounwind ; <double> [#uses=1]
  %25 = fptrunc double %24 to float               ; <float> [#uses=1]
  %26 = sitofp i32 %1 to float                    ; <float> [#uses=1]
  %27 = fadd float %25, %26                       ; <float> [#uses=1]
  %28 = tail call double @_Z8distanceU8__vector2dS_(<2 x double> %10, <2 x double> %10) nounwind ; <double> [#uses=1]
  %29 = fptrunc double %28 to float               ; <float> [#uses=1]
  %30 = fadd float %27, %29                       ; <float> [#uses=1]
  %31 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %30, float addrspace(1)* %31
  ret void
}

declare i32 @get_global_id(i32)

declare double @_Z8distanceU8__vector4dS_(<4 x double>, <4 x double>)

declare double @_Z8distanceU8__vector2dS_(<2 x double>, <2 x double>)

declare i32 @get_global_id(i32)
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK: fmul double
;CHECK-NOT: fmul <8
;CHECK: @_Z8distanceU8__vector4dS_
;CHECK: @_Z8distanceU8__vector2dS_ 
;CHECK: ret

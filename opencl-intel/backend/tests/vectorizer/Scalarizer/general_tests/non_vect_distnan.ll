;*******************************************
;scalarazer test
;check that distance function is non scalarazible 
;for input vector, this function is not in hash
;
;******************************************* 



; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'non_vect_distnan.cl'
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
  %4 = sitofp i64 %3 to double                    ; <double> [#uses=1]
  %5 = insertelement <4 x double> undef, double %4, i32 0 ; <<4 x double>> [#uses=1]
  %6 = shufflevector <4 x double> %5, <4 x double> undef, <4 x i32> zeroinitializer ; <<4 x double>> [#uses=1]
  %7 = load i64 addrspace(1)* %in                 ; <i64> [#uses=1]
  %8 = sext i32 %1 to i64                         ; <i64> [#uses=2]
  %9 = mul i64 %7, %8                             ; <i64> [#uses=1]
  %10 = sitofp i64 %9 to double                   ; <double> [#uses=1]
  %11 = insertelement <4 x double> undef, double %10, i32 0 ; <<4 x double>> [#uses=1]
  %12 = add nsw i64 %3, %8                        ; <i64> [#uses=1]
  %13 = sitofp i64 %12 to double                  ; <double> [#uses=1]
  %14 = insertelement <4 x double> %11, double %13, i32 1 ; <<4 x double>> [#uses=1]
  %15 = getelementptr inbounds i64 addrspace(1)* %in, i32 2 ; <i64 addrspace(1)*> [#uses=1]
  %16 = load i64 addrspace(1)* %15                ; <i64> [#uses=1]
  %17 = sitofp i64 %16 to double                  ; <double> [#uses=1]
  %18 = insertelement <4 x double> %14, double %17, i32 2 ; <<4 x double>> [#uses=1]
  %19 = insertelement <4 x double> %18, double 1.200000e+001, i32 3 ; <<4 x double>> [#uses=1]
  %20 = tail call double @_Z8distanceU8__vector4dS_(<4 x double> %6, <4 x double> %19) nounwind ; <double> [#uses=1]
  %21 = fptrunc double %20 to float               ; <float> [#uses=1]
  %22 = sitofp i32 %1 to float                    ; <float> [#uses=1]
  %23 = fadd float %21, %22                       ; <float> [#uses=1]
  %24 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %23, float addrspace(1)* %24
  ret void
}

declare i32 @get_global_id(i32)

declare double @_Z8distanceU8__vector4dS_(<4 x double>, <4 x double>)


;CHECK: double @_Z8distanceU8__vector4dS_
;CHECK: ret



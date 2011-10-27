;*******************************************
;scalarazer test
;check that vectorazible scalar variables in the
; vectorazable function are not changes
; 
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'scalar_arg.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_check_vectrorazible_f_with_scalar_arg_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_check_vectrorazible_f_with_scalar_arg_parameters = appending global [85 x i8] c"ulong __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[85 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i64 addrspace(1)*, float addrspace(1)*)* @check_vectrorazible_f_with_scalar_arg to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_check_vectrorazible_f_with_scalar_arg_locals to i8*), i8* getelementptr inbounds ([85 x i8]* @opencl_check_vectrorazible_f_with_scalar_arg_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @check_vectrorazible_f_with_scalar_arg(i64 addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = sitofp i32 %1 to double                    ; <double> [#uses=2]
  %3 = load i64 addrspace(1)* %in                 ; <i64> [#uses=1]
  %4 = uitofp i64 %3 to double                    ; <double> [#uses=1]
  %5 = fadd double %2, %4                         ; <double> [#uses=2]
  %6 = fadd double %5, 3.140000e+000              ; <double> [#uses=2]
  %7 = fmul double %6, %2                         ; <double> [#uses=1]
  %8 = tail call double @_Z8distancedd(double 3.140000e+000, double %5) nounwind ; <double> [#uses=1]
  %9 = fadd double %7, %8                         ; <double> [#uses=2]
  %10 = fdiv double %9, 3.140000e+000             ; <double> [#uses=1]
  %11 = fmul double %10, %9                       ; <double> [#uses=1]
  %12 = fsub double %11, %6                       ; <double> [#uses=1]
  %13 = fptrunc double %12 to float               ; <float> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %13, float addrspace(1)* %14
  ret void
}

declare i32 @get_global_id(i32)

declare double @_Z8distancedd(double, double)

;CHECK: fadd double
;CHECK: fadd double
;CHECK: fmul double
;CHECK: @_Z8distancedd
;CHECK: fadd double
;CHECK: fdiv double
;CHECK: fmul double
;CHECK: fsub double
;CHECK: ret
;CHECK: @_Z8distancedd

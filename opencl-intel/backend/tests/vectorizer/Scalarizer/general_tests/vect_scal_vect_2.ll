
;*******************************************
;scalarazer test
;check  check that vector is scalarazied and packed again before store operation
;
;*******************************************
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'vect_scal_vect_2.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_abs_diff_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_abs_diff_parameters = appending global [88 x i8] c"double __attribute__((address_space(1))) *, double4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[88 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (double addrspace(1)*, <4 x double> addrspace(1)*)* @func_abs_diff to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_abs_diff_locals to i8*), i8* getelementptr inbounds ([88 x i8]* @opencl_func_abs_diff_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_abs_diff(double addrspace(1)* nocapture %in, <4 x double> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = getelementptr inbounds double addrspace(1)* %in, i32 1 ; <double addrspace(1)*> [#uses=1]
  %3 = load double addrspace(1)* %2               ; <double> [#uses=1]
  %4 = insertelement <4 x double> undef, double %3, i32 0 ; <<4 x double>> [#uses=1]
  %5 = getelementptr inbounds double addrspace(1)* %in, i32 2 ; <double addrspace(1)*> [#uses=1]
  %6 = load double addrspace(1)* %5               ; <double> [#uses=1]
  %7 = insertelement <4 x double> %4, double %6, i32 1 ; <<4 x double>> [#uses=1]
  %8 = getelementptr inbounds double addrspace(1)* %in, i32 3 ; <double addrspace(1)*> [#uses=1]
  %9 = load double addrspace(1)* %8               ; <double> [#uses=1]
  %10 = insertelement <4 x double> %7, double %9, i32 2 ; <<4 x double>> [#uses=1]
  %11 = getelementptr inbounds double addrspace(1)* %in, i32 4 ; <double addrspace(1)*> [#uses=1]
  %12 = load double addrspace(1)* %11             ; <double> [#uses=1]
  %13 = insertelement <4 x double> %10, double %12, i32 3 ; <<4 x double>> [#uses=1]
  %14 = getelementptr inbounds double addrspace(1)* %in, i32 %1 ; <double addrspace(1)*> [#uses=1]
  %15 = load double addrspace(1)* %14             ; <double> [#uses=2]
  %16 = insertelement <4 x double> undef, double %15, i32 0 ; <<4 x double>> [#uses=1]
  %17 = add nsw i32 %1, 2                         ; <i32> [#uses=1]
  %18 = getelementptr inbounds double addrspace(1)* %in, i32 %17 ; <double addrspace(1)*> [#uses=1]
  %19 = load double addrspace(1)* %18             ; <double> [#uses=1]
  %20 = insertelement <4 x double> %16, double %19, i32 1 ; <<4 x double>> [#uses=1]
  %21 = fadd double %15, 3.000000e+000            ; <double> [#uses=1]
  %22 = insertelement <4 x double> %20, double %21, i32 2 ; <<4 x double>> [#uses=1]
  %23 = insertelement <4 x double> %22, double 1.120000e+002, i32 3 ; <<4 x double>> [#uses=1]
  %24 = fadd <4 x double> %13, %23                ; <<4 x double>> [#uses=1]
  %25 = getelementptr inbounds <4 x double> addrspace(1)* %out, i32 %1 ; <<4 x double> addrspace(1)*> [#uses=1]
  store <4 x double> %24, <4 x double> addrspace(1)* %25
  ret void
}

declare i32 @get_global_id(i32)


;CHECK: [[NAME1:%[1-9]+]] = load double {{.*}}
;CHECK: [[NAME2:%[1-9]+]] = load double {{.*}}
;CHECK: [[NAME3:%[1-9]+]] = load double {{.*}}
;CHECK: [[NAME4:%[1-9]+]] = load double {{.*}}


;CHECK: [[NAME5:%[1-9]+]] = fadd double [[NAME1]]
;CHECK: [[NAME6:%[1-9]+]] = fadd double [[NAME2]]
;CHECK: [[NAME7:%[1-9]+[1-9]+]] = fadd double [[NAME3]]
;CHECK: [[NAME8:%[1-9]+]] = fadd double [[NAME4]]
;CHECK:  {{.*}}insertelement <4 x double> undef, double [[NAME5]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x double> %temp.vect, double [[NAME6]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x double> %temp.vect{{[1-9]+}}, double [[NAME7]]{{.*}}
;CHECK:  {{.*}}insertelement <4 x double> %temp.vect{{[1-9]+}}, double [[NAME8]]{{.*}}

;CHECK: ret
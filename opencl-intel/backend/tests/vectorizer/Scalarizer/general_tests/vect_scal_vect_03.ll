
;*******************************************
;scalarazer test
;check  check that vector is scalarazied and packed again before store operation
;
;*******************************************
; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'vect_scal_vect_03.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_vect_scan_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_vect_scan_parameters = appending global [88 x i8] c"int const __attribute__((address_space(1))) *, int2 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[88 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (i32 addrspace(1)*, <2 x i32> addrspace(1)*)* @func_vect_scan to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_vect_scan_locals to i8*), i8* getelementptr inbounds ([88 x i8]* @opencl_func_vect_scan_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_vect_scan(i32 addrspace(1)* nocapture %in, <2 x i32> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = getelementptr inbounds i32 addrspace(1)* %in, i32 1 ; <i32 addrspace(1)*> [#uses=1]
  %3 = load i32 addrspace(1)* %2                  ; <i32> [#uses=1]
  %4 = insertelement <2 x i32> undef, i32 %3, i32 0 ; <<2 x i32>> [#uses=1]
  %5 = insertelement <2 x i32> %4, i32 %1, i32 1  ; <<2 x i32>> [#uses=1]
  %6 = add nsw <2 x i32> %5, <i32 14, i32 257>    ; <<2 x i32>> [#uses=1]
  %7 = getelementptr inbounds <2 x i32> addrspace(1)* %out, i32 %1 ; <<2 x i32> addrspace(1)*> [#uses=1]
  store <2 x i32> %6, <2 x i32> addrspace(1)* %7
  ret void
}

declare i32 @get_global_id(i32)



;CHECK: ret
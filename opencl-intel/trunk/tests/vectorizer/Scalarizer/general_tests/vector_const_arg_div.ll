;*******************************************
;scalarazer test
;constant parameter for scalarazible functions
; function mul is not scalar but div is scalaraziblr du to clang.
;
;*******************************************

; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'vector_const_arg2.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_mul_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_mul_parameters = appending global [278 x i8] c"float2 const __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, char8 const __attribute__((address_space(1))), char8 const __attribute__((address_space(1))), int4 const __attribute__((address_space(1))), int4 const __attribute__((address_space(1)))\00", section "llvm.metadata" ; <[278 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float> addrspace(1)*, float addrspace(1)*, <8 x i8>, <8 x i8>, <4 x i32>, <4 x i32>)* @mul to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_mul_locals to i8*), i8* getelementptr inbounds ([278 x i8]* @opencl_mul_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @mul(<2 x float> addrspace(1)* nocapture %in, float addrspace(1)* nocapture %out, <8 x i8> %charArg1, <8 x i8> %charArg2, <4 x i32> %intArg1, <4 x i32> %intArg2) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = icmp eq <8 x i8> %charArg2, zeroinitializer ; <<8 x i1>> [#uses=1]
  %3 = icmp eq <8 x i8> %charArg1, <i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128, i8 -128> ; <<8 x i1>> [#uses=1]
  %4 = icmp eq <8 x i8> %charArg2, <i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1, i8 -1> ; <<8 x i1>> [#uses=1]
  %5 = and <8 x i1> %3, %4                        ; <<8 x i1>> [#uses=1]
  %6 = or <8 x i1> %2, %5                         ; <<8 x i1>> [#uses=1]
  %7 = select <8 x i1> %6, <8 x i8> <i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1>, <8 x i8> %charArg2 ; <<8 x i8>> [#uses=1]
  %8 = sdiv <8 x i8> %charArg1, %7                ; <<8 x i8>> [#uses=1]
  %9 = mul <4 x i32> %intArg2, %intArg2           ; <<4 x i32>> [#uses=1]
  %10 = extractelement <8 x i8> %8, i32 0         ; <i8> [#uses=1]
  %11 = sitofp i8 %10 to float                    ; <float> [#uses=1]
  %12 = fpext float %11 to double                 ; <double> [#uses=1]
  %13 = fadd double %12, 1.000000e+000            ; <double> [#uses=1]
  %14 = extractelement <4 x i32> %9, i32 1        ; <i32> [#uses=1]
  %15 = sitofp i32 %14 to float                   ; <float> [#uses=1]
  %16 = fpext float %15 to double                 ; <double> [#uses=1]
  %17 = fadd double %13, %16                      ; <double> [#uses=1]
  %18 = fptrunc double %17 to float               ; <float> [#uses=1]
  %19 = getelementptr inbounds float addrspace(1)* %out, i32 %1 ; <float addrspace(1)*> [#uses=1]
  store float %18, float addrspace(1)* %19
  ret void
}

declare i32 @get_global_id(i32)

;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: sdiv i8
;CHECK: mul <4 x i32> %intArg2, %intArg2
;CHECK: ret
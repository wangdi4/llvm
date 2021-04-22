; RUN: opt -mtriple=x86_64 -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -mtriple=x86_64 -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{i8, i8, i8}>

; CHECK: @t1
define void @t1(%struct.struct1* %arg1, <2 x i8> %arg2, i32 %arg3, <8 x i16> %arg4, i8 %arg5, i32* %arg6 ) #0 {
entry:
  ret void
}

;; new func - Win64
;;struct1 my_struct1 arg1 - struct is byVal- size:3
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*

;; char2 arg2 - alignment: 2 - in UniformArgs 0+3 = 3 is aligned to 4
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 4
; CHECK-NEXT: [[ARG1_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG1_BUFF_INDEX]] to <2 x i8>*
; CHECK-NEXT: %explicit_1 = load <2 x i8>, <2 x i8>* [[ARG1_TYPECAST]], align 2

;; int arg3 - alignment: 4 - in UniformArgs 4+2=6 is aligned to 8
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 8
; CHECK-NEXT: [[ARG2_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG2_BUFF_INDEX]] to i32*
; CHECK-NEXT: %explicit_2 = load i32, i32* [[ARG2_TYPECAST]], align 4

;; short8 arg4 - alignment: 16 - in UniformArgs 8+4=12 is aligned to 16
; CHECK-NEXT: [[ARG3_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 16
; CHECK-NEXT: [[ARG3_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG3_BUFF_INDEX]] to <8 x i16>*
; CHECK-NEXT: %explicit_3 = load <8 x i16>, <8 x i16>* [[ARG3_TYPECAST]], align 16

;; char arg5 - alignment: 1 - in UniformArgs 16+16=32 is aligned to 32
; CHECK-NEXT: [[ARG4_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK-NEXT: %explicit_4 = load i8, i8* [[ARG4_BUFF_INDEX]], align 1

;; int* arg6 - alignment: 8 - in UniformArgs 32+1=33 is aligned to 40
; CHECK-NEXT: [[ARG5_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 40
; CHECK-NEXT: [[ARG5_TYPECAST:%[a-zA-Z0-9]+]] = bitcast i8* [[ARG5_BUFF_INDEX]] to i32**
; CHECK-NEXT: %explicit_5 = load i32*, i32** [[ARG5_TYPECAST]], align 8
;;implicit args
; CHECK: ret void


attributes #0 = { "sycl_kernel" }

; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-f80:32:32-n8:16:32-S32"

%struct.struct1 = type <{ <4 x i32> , i8}>
%struct.struct2 = type <{ i32 ,i32 ,i32 }>

; CHECK: @t1
define void @t1(%struct.struct1* %arg1, %struct.struct2* %arg2, <4 x i32>* byval(<4 x i32>) %arg3) #0 {
entry:
  ret void
}

;; struct1 my_struct1 arg1 - 17 bytes - expected alignment: 0
; CHECK: [[ARG0_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 0
; CHECK-NEXT: %explicit_0 = bitcast i8* [[ARG0_BUFF_INDEX]] to %struct.struct1*
;; struct2 my_struct2 arg2 - 12 bytes - expected alignment: 0
; CHECK-NEXT: [[ARG1_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 17
; CHECK-NEXT: %explicit_1 = bitcast i8* [[ARG1_BUFF_INDEX]] to %struct.struct2*
;; int4 byvalue , size is the actual size not the pointer size - expected alignment:16 (17+12 = 29 so it's aligned to 32)
; CHECK-NEXT: [[ARG2_BUFF_INDEX:%[a-zA-Z0-9]+]] = getelementptr i8, i8* %UniformArgs, i32 32
; CHECK-NEXT: %explicit_2 = bitcast i8* [[ARG2_BUFF_INDEX]] to <4 x i32>*
;;implicit args
; CHECK: ret void

attributes #0 = { "sycl_kernel" }

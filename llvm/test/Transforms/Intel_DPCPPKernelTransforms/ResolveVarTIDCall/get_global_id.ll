; RUN: opt -passes=dpcpp-kernel-resolve-var-tid-call -S < %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S < %s | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-resolve-var-tid-call -S < %s | FileCheck %s
; RUN: opt -enable-new-pm=0 -dpcpp-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S < %s | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z13get_global_idj(i32)

; Function Attrs: convergent nounwind
define void @testKernel(i32 %a, i64* %b) {
entry:
  %call = call i64 @_Z13get_global_idj(i32 %a)
; CHECK: entry:
; CHECK-NEXT: [[LID0:%.*]] = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: [[CMP0:%.*]] = icmp eq i32 %a, 0
; CHECK-NEXT: br i1 [[CMP0]], label %[[L_RES:.*]], label %[[L_1:.*]]

; CHECK: [[L_1]]:
; CHECK-NEXT: [[LID1:%.*]] = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: [[CMP1:%.*]] = icmp eq i32 %a, 1
; CHECK-NEXT: br i1 [[CMP1]], label %[[L_RES]], label %[[L_2:.*]]

; CHECK: [[L_2]]:
; CHECK-NEXT: [[LID2:%.*]] = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: [[CMP2:%.*]] = icmp eq i32 %a, 2
; CHECK-NEXT: br i1 [[CMP2]], label %[[L_RES]], label %[[L_OOB:.*]]

; CHECK: [[L_OOB]]:
; CHECK-NEXT: br label %[[L_RES]]

; CHECK: [[L_RES]]:
; CHECK-NEXT: [[CALL:%.*]] = phi i64
; CHECK-DAG: [ 0, %[[L_OOB]] ]
; CHECK-DAG: [ [[LID0]], %entry ]
; CHECK-DAG: [ [[LID1]], %[[L_1]] ]
; CHECK-DAG: [ [[LID2]], %[[L_2]] ]
  store i64 %call, i64* %b
; CHECK: store i64 [[CALL]], i64* %b
  ret void
}

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!sycl.kernels = !{!3}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{void (i32, i64*)* @testKernel}

; DEBUGIFY-NOT: WARNING

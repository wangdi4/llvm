; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -S < %s | FileCheck %s
; RUN: opt -passes=sycl-kernel-resolve-var-tid-call -enable-debugify -disable-output 2>&1 -S < %s | FileCheck %s -check-prefix=DEBUGIFY

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

declare i64 @_Z12get_group_idj(i32)

; Function Attrs: convergent nounwind
define void @testKernel(i32 %a, ptr %b) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %call = call i64 @_Z12get_group_idj(i32 %a)
; CHECK:      [[GroupID0:%.*]] = call i64 @_Z12get_group_idj(i32 0)
; CHECK-NEXT: [[GroupID1:%.*]] = call i64 @_Z12get_group_idj(i32 1)
; CHECK-NEXT: [[GroupID2:%.*]] = call i64 @_Z12get_group_idj(i32 2)
; CHECK-NEXT: [[CMP0:%.*]] = icmp eq i32 %a, 0
; CHECK-NEXT: [[SEL0:%.*]] = select i1 [[CMP0]], i64 [[GroupID0]], i64 0
; CHECK-NEXT: [[CMP1:%.*]] = icmp eq i32 %a, 1
; CHECK-NEXT: [[SEL1:%.*]] = select i1 [[CMP1]], i64 [[GroupID1]], i64 [[SEL0]]
; CHECK-NEXT: [[CMP2:%.*]] = icmp eq i32 %a, 2
; CHECK-NEXT: [[CALL:%.*]] = select i1 [[CMP2]], i64 [[GroupID2]], i64 [[SEL1]]
  store i64 %call, ptr %b
; CHECK: store i64 [[CALL]], ptr %b
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @testKernel}
!1 = !{!"int", !"long*"}
!2 = !{i32 0, ptr null}

; DEBUGIFY-NOT: WARNING

; RUN: %oclopt -B-Barrier -verify -S < %s | FileCheck %s
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"

target triple = "i686-pc-win32"
define void @main(i32 %x) nounwind !vectorized_width !1 {
L1:
  call void @dummybarrier.()
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %y = xor i32 %x, %lid
  br label %L2
L2:
  call void @_Z7barrierj(i32 1)
; CHECK-LABEL: CallBB:
; CHECK: [[LID:%.*]] = load i32, i32* %pLocalId_0, align 4
; CHECK-NEXT: [[INC:%.*]] = add nuw i32 [[LID]], 8
; CHECK-NEXT: store i32 [[INC]], i32* %pLocalId_0, align 4
  call void @foo(i32 %y)
  br label %L3
L3:
  call void @dummybarrier.()
  ret void
}

define void @foo(i32 %x) #0 nounwind {
L1:
  call void @dummybarrier.()
  %y = xor i32 %x, %x
  br label %L2
L2:
  call void @_Z7barrierj(i32 2)
; CHECK: [[LID:%.*]] = load i32, i32* %pLocalId_0, align 4
; CHECK-NEXT: [[INC:%.*]] = add nuw i32 [[LID]], 8
; CHECK-NEXT: store i32 [[INC]], i32* %pLocalId_0, align 4
  ret void
}

declare void @_Z7barrierj(i32)
declare i32 @_Z12get_local_idj(i32)
declare void @dummybarrier.()

attributes #0 = { "widened-size"="8" }

!opencl.kernels = !{!0}

!0 = !{void (i32)* @main}
!1 = !{i32 8}

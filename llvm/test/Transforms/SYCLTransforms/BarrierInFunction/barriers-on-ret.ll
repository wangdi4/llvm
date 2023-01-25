; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier-in-function -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass inserts barriers only
;; in reachable basic blocks.
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @foo(i32 %x) nounwind {
entry:
  %y = xor i32 %x, %x
  call void @_Z18work_group_barrierj(i32 2)
  br label %reachable.exit

reachable.exit:                                   ; preds = entry
  ret void

unreachable.exit:                                 ; No predecessors!
  ret void
; CHECK:      reachable.exit:
; CHECK-NEXT: @_Z18work_group_barrierj(i32 1)
; CHECK:      unreachable.exit:
; CHECK-NOT:  @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: ret void
}

declare void @_Z18work_group_barrierj(i32)

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo -- call void @dummy_barrier.()
; DEBUGIFY-NOT: WARNING

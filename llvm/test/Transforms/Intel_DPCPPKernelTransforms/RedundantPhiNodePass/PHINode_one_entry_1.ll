; RUN: opt -passes=dpcpp-kernel-redundant-phi-node %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-redundant-phi-node %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the RedundantPhiNode pass
;; The case: function "main" contains a PHINode "%isOk" of length one,
;;           with uniform value "%check" in its entry
;;           "%isOk" is not in use
;; The expected result:
;;      1. Remove the PHINode "%isOk"
;;*****************************************************************************

; ModuleID = 'Program'

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br label %L1
L1:
  br label %L2
L2:
  br label %L3
L3:
  %isOk = phi i1 [ %check, %L2 ]
  ret void
; CHECK:   %check = icmp ult i32 %x, 0
; CHECK:   br label %L1
; CHECK: L1:
; CHECK:   br label %L2
; CHECK: L2:
; CHECK:   br label %L3
; CHECK: L3:
; CHECK-NEXT:   ret void
}

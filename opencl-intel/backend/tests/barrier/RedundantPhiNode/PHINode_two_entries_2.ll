; RUN: %oclopt -B-RedundantPhiNode -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -B-RedundantPhiNode -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the RedundantPhiNode pass
;; The case: function "main" contains a PHINode "%isOk" of length two,
;;           with same uniform value "%check" in both entries
;; The expected result:
;;      1. Remove the PHINode "%isOk"
;;      2. Replace all places that used "%isOk" with the value "%check"
;;*****************************************************************************

; ModuleID = 'Program'

; CHECK: @main
define i1 @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ %check, %L1 ], [ %check, %L2 ]
  ret i1 %isOk
; CHECK:   %check = icmp ult i32 %x, 0
; CHECK:   br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK:   br label %L3
; CHECK: L2:
; CHECK:   br label %L3
; CHECK: L3:
; CHECK-NEXT:   ret i1 %check
}

;; Phi node is removed
; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY-NOT: WARNING

; RUN: opt -passes=dpcpp-kernel-redundant-phi-node %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-redundant-phi-node %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-redundant-phi-node %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-redundant-phi-node %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the RedundantPhiNode pass
;; The case: function "main" contains a PHINode "%isOk" of length one,
;;           with uniform value "false" in its entry
;; The expected result:
;;      1. Remove the PHINode "%isOk"
;;      2. Replace all places that used "%isOk" with the value "false"
;;*****************************************************************************

; ModuleID = 'Program'

; CHECK: @main
define i1 @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br label %L1
L1:
  br label %L2
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L2 ]
  ret i1 %isOk
; CHECK:   %check = icmp ult i32 %x, 0
; CHECK:   br label %L1
; CHECK: L1:
; CHECK:   br label %L2
; CHECK: L2:
; CHECK:   br label %L3
; CHECK: L3:
; CHECK-NEXT:   ret i1 false
}

; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY-NOT: WARNING

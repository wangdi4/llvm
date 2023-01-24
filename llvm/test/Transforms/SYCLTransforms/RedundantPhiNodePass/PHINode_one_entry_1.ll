; RUN: opt -passes=sycl-kernel-redundant-phi-node %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=sycl-kernel-redundant-phi-node %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=sycl-kernel-redundant-phi-node -sycl-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=sycl-kernel-redundant-phi-node -sycl-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

;;*****************************************************************************
;; This test checks the RedundantPhiNode pass
;; The case: function "main" contains a PHINode "%isOk" of length one,
;;           with uniform value "%check" in its entry
;;           "%isOk" is not in use
;; The expected result:
;;      1. Remove the PHINode "%isOk"
;;*****************************************************************************

; ModuleID = 'Program'

; NOSKIP: @main
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
; NOSKIP:   %check = icmp ult i32 %x, 0
; NOSKIP:   br label %L1
; NOSKIP: L1:
; NOSKIP:   br label %L2
; NOSKIP: L2:
; NOSKIP:   br label %L3
; NOSKIP: L3:
; SKIP: %isOk = phi i1 [ %check, %L2 ]
; NOSKIP: ret void
}

; DEBUGIFY-NOSKIP: WARNING: Missing line 5
; DEBUGIFY-ALL-NOT: WARNING

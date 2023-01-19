; RUN: opt -passes=dpcpp-kernel-redundant-phi-node %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-redundant-phi-node %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=dpcpp-kernel-redundant-phi-node -dpcpp-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-redundant-phi-node -dpcpp-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

;;*****************************************************************************
;; This test checks the RedundantPhiNode pass
;; The case: function "main" contains a PHINode "%isOk" of length two,
;;           with same value "false" in both entries
;; The expected result:
;;      1. Remove the PHINode "%isOk"
;;      2. Replace all places that used "%isOk" with the value "false"
;;*****************************************************************************

; ModuleID = 'Program'

; NOSKIP: @main
define i1 @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  br i1 %check, label %L1, label %L2
L1:
  br label %L3
L2:
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ false, %L2 ]
  ret i1 %isOk

; NOSKIP:   %check = icmp ult i32 %x, 0
; NOSKIP:   br i1 %check, label %L1, label %L2
; NOSKIP: L1:
; NOSKIP:   br label %L3
; NOSKIP: L2:
; NOSKIP:   br label %L3
; NOSKIP: L3:
; NOSKIP:   ret i1 false
; SKIP: %isOk = phi i1 [ false, %L1 ], [ false, %L2 ]
; SKIP-NEXT: ret i1 %isOk
}

; DEBUGIFY-NOSKIP: WARNING: Missing line 5
; DEBUGIFY-ALL-NOT: WARNING

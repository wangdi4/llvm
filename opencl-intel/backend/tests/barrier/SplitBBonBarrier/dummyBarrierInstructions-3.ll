; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-SplitOnBarrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the SplitBBonBarrier pass
;; The case: function "main" with 4 dummybarrier instructions
;;           2 start at begining of its basic block while other 2 are in middle
;; The expected result:
;;      1. 4 dummybarrier instructions each start at begining of basic block
;;      2. All the instruction stay in the same orders
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  call void @dummybarrier.()
  br i1 %check, label %L1, label %L2
L1:
  call void @dummybarrier.()
  call void @dummybarrier.()
  br label %L3
L2:
  call void @dummybarrier.()
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  call void @dummybarrier.()
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br
; CHECK: :
; CHECK: call void @dummybarrier.()
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: call void @dummybarrier.()
; CHECK: br
; CHECK: :
; CHECK: call void @dummybarrier.()
; CHECK: br label %L3
; CHECK: L2:
; CHECK: call void @dummybarrier.()
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %
  ; CHECK: ], [ true, %L2 ]
; CHECK: br
; CHECK: :
; CHECK: call void @dummybarrier.()
; CHECK: ret void
}

declare void @dummybarrier.()
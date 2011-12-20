; RUN: llvm-as %s -o %t.bc
; RUN: opt -B-SplitOnBarrier -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;;*****************************************************************************
;; This test checks the SplitBBonBarrier pass
;; The case: function "main" with 3 barrier instructions
;;           2 start at begining of its basic block while another 1 is in middle
;; The expected result:
;;      1. 3 barrier instructions each start at begining of basic block
;;      2. All the instruction stay in the same orders
;;*****************************************************************************

; CHECK: @main
define void @main(i32 %x) nounwind {
  %check = icmp ult i32 %x, 0
  call void @barrier(i32 2)
  br i1 %check, label %L1, label %L2
L1:
  call void @barrier(i32 2)
  br label %L3
L2:
  call void @barrier(i32 2)
  br label %L3
L3:
  %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
  ret void
; CHECK: %check = icmp ult i32 %x, 0
; CHECK: br
; CHECK: :
; CHECK: call void @barrier(i32 2)
; CHECK: br i1 %check, label %L1, label %L2
; CHECK: L1:
; CHECK: call void @barrier(i32 2)
; CHECK: br label %L3
; CHECK: L2:
; CHECK: call void @barrier(i32 2)
; CHECK: br label %L3
; CHECK: L3:
; CHECK: %isOk = phi i1 [ false, %L1 ], [ true, %L2 ]
; CHECK: ret void
}

declare void @barrier(i32)
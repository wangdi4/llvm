; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Verify that we are able to handle a GEP index which has a different size (32 bits) than the base pointer (64 bits).

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   loop:
; CHECK: |   %ptr.int = ptrtoint.i8*.i64(&((%1)[i1 + 1]));
; CHECK: |   if (trunc.i64.i2(%ptr.int) != 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto loop;
; CHECK: |   }
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define hidden fastcc void @lzma_check_update(i32, i8*) unnamed_addr {
entry:
  br label %loop

loop:                                      ; preds = %loop, %entry
  %ptr = phi i8* [ %ptr.inc, %loop ], [ %1, %entry ]
  %ptr.inc = getelementptr inbounds i8, i8* %ptr, i32 1
  %ptr.int = ptrtoint i8* %ptr.inc to i64
  %and = and i64 %ptr.int, 3
  %cmp = icmp eq i64 %and, 0
  br i1 %cmp, label %exit, label %loop

exit:                                     ; preds = %loop
  ret void
}


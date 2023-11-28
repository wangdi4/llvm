; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s


; Verify that we are able to handle a GEP index which has a different size (32 bits) than the base pointer (64 bits).

; CHECK: + DO i1 = 0, zext.i2.i64((-1 + (-1 * trunc.i64.i2(ptrtoint.ptr.i64(%1))))), 1   <DO_LOOP>  <MAX_TC_EST = 4>
; CHECK: |   %ptr.inc = &((%1)[i1 + 1]);
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define hidden fastcc void @lzma_check_update(i32, ptr) unnamed_addr {
entry:
  br label %loop

loop:                                      ; preds = %loop, %entry
  %ptr = phi ptr [ %ptr.inc, %loop ], [ %1, %entry ]
  %ptr.inc = getelementptr inbounds i8, ptr %ptr, i32 1
  %ptr.int = ptrtoint ptr %ptr.inc to i64
  %and = and i64 %ptr.int, 3
  %cmp = icmp eq i64 %and, 0
  br i1 %cmp, label %exit, label %loop

exit:                                     ; preds = %loop
  %ptr.lcssa = phi ptr [ %ptr.inc, %loop ]
  ret void
}


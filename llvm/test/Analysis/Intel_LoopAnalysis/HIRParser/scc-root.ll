; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser | FileCheck %s

; Verify that the loop is parsed correctly.

; CHECK: + DO i1 = 0, %len + -1 * smin(1, %len), 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK: |   %res.0.out = %res.0;
; CHECK: |   %code.addr.0.out = %code.addr.0;
; CHECK: |   %or = %res.0.out  |  trunc.i32.i1(%code.addr.0.out);
; CHECK: |   %code.addr.0 = %code.addr.0  >>  1;
; CHECK: |   %res.0 = %or  <<  1;
; CHECK: + END LOOP


; ModuleID = 'bits.ll'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind readnone
define i32 @bi_reverse(i32 %code, i32 %len) #2 {
entry:
  br label %do.cond

do.cond:                                          ; preds = %entry, %do.cond
  %code.addr.0 = phi i32 [ %code, %entry ], [ %shr, %do.cond ]
  %len.addr.0 = phi i32 [ %len, %entry ], [ %dec, %do.cond ]
  %res.0 = phi i32 [ 0, %entry ], [ %shl, %do.cond ]
  %and = and i32 %code.addr.0, 1
  %or = or i32 %res.0, %and
  %shr = lshr i32 %code.addr.0, 1
  %shl = shl i32 %or, 1
  %dec = add nsw i32 %len.addr.0, -1
  %cmp = icmp sgt i32 %len.addr.0, 1
  br i1 %cmp, label %do.cond, label %do.end

do.end:                                           ; preds = %do.cond
  %or.lcssa = phi i32 [ %or, %do.cond ]
  %shr1 = and i32 %or.lcssa, 2147483647
  ret i32 %shr1
}


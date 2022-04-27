; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll" -print-after=hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s

; The trip count of this loop can be 2^8 (interpreted as 0) when n is equal to 7.
; To handle that case, we add a zero trip check on the remainder loop to make it run all the iterations.
; Note that original LEGAL_MAX_TC/MAX_TC_EST remains on the remainder loop.

; Input HIR-
; + DO i1 = 0, -1 * %n + 6, 1   <DO_LOOP> <MAX_TC_EST = 256>  <LEGAL_MAX_TC = 256>
; |   (%A)[i1 + %n] = i1 + %n;
; + END LOOP

; CHECK: + DO i1 = 0, %tgu + -1, 1

; CHECK: %bound.check = 8 * %tgu <u -1 * %n + 7;
; CHECK: %zero.tc.check = -1 * %n + 7 == 0;
; CHECK: %combined.ztt = %bound.check  |  %zero.tc.check;
; CHECK: if (%combined.ztt != 0)
; CHECK: {
; CHECK: + DO i1 = 8 * %tgu, -1 * %n + 6, 1   <DO_LOOP> <MAX_TC_EST = 256>  <LEGAL_MAX_TC = 256> <nounroll>
; CHECK-NOT: MAX_TC_EST
; CHECK-NOT: LEGAL_MAX_TC


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i8 noundef %n, i64* nocapture noundef writeonly %A) {
entry:
  br label %do.body

do.body:                                          ; preds = %do.body, %entry
  %i.0 = phi i8 [ %n, %entry ], [ %inc, %do.body ]
  %idxprom = zext i8 %i.0 to i64
  %arrayidx = getelementptr inbounds i64, i64* %A, i64 %idxprom
  store i64 %idxprom, i64* %arrayidx, align 4
  %inc = add i8 %i.0, 1
  %cmp.not = icmp eq i8 %inc, 7
  br i1 %cmp.not, label %do.end, label %do.body

do.end:                                           ; preds = %do.body
  ret void
}


; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -S < %s 2>&1 | FileCheck %s

; This test checks that the inner If condition is hoisted out
; even if the outer If can't be hoisted, but the switch is not
; hoisted since we don't allow any deeper condition to be a
; candidate for this kind of unswitching.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if ((%p)[i1] == 8)
;       |   {
;       |      if (%n == 5)
;       |      {
;       |         (%p)[i1] = 1;
;       |         switch(%x)
;       |         {
;       |         case 0:
;       |            (%p)[i1] = i1;
;       |            break;
;       |         case 2:
;       |            (%q)[i1] = i1;
;       |            break;
;       |         default:
;       |            (%q)[i1 + 1] = i1;
;       |            break;
;       |         }
;       |      }
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%n == 5)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   if ((%p)[i1] == 8)
; CHECK:          |   {
; CHECK:          |      (%p)[i1] = 1;
; CHECK:          |      switch(%x)
; CHECK:          |      {
; CHECK:          |      case 0:
; CHECK:          |         (%p)[i1] = i1;
; CHECK:          |         break;
; CHECK:          |      case 2:
; CHECK:          |         (%q)[i1] = i1;
; CHECK:          |         break;
; CHECK:          |      default:
; CHECK:          |         (%q)[i1 + 1] = i1;
; CHECK:          |         break;
; CHECK:          |      }
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i64 %n, i32 %x, ptr %p, ptr %q) {
entry:
  %0 = trunc i64 %n to i32
  br label %for.body

for.body:
  %i = phi i32 [ 0, %entry ], [ %ip, %for.inc ]
  %idxprom = sext i32 %i to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %p, i64 %idxprom
  %1 = load i32, ptr %arrayidx2, align 4
  %cmp1 = icmp eq i32 %1, 8
  br i1 %cmp1, label %if.then, label %for.inc

if.then:
  %cmp2 = icmp eq i32 %0, 5
  br i1 %cmp2, label %if.inner, label %for.inc

if.inner:
  store i32 1, ptr %arrayidx2
  switch i32 %x, label %sw.default [
    i32 0, label %sw.bb
    i32 2, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %p, i64 %idxprom
  %2 = trunc i64 %idxprom to i32
  store i32 %2, ptr %arrayidx, align 4
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, ptr %q, i64 %idxprom
  %3 = trunc i64 %idxprom to i32
  store i32 %3, ptr %arrayidx3, align 4
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %4 = add nuw nsw i64 %idxprom, 1
  %arrayidx5 = getelementptr inbounds i32, ptr %q, i64 %4
  %5 = trunc i64 %idxprom to i32
  store i32 %5, ptr %arrayidx5, align 4
  br label %for.inc

for.inc:
  %ip = add nsw i32 %i, 1
  %cmp = icmp slt i32 %i, 99
  br i1 %cmp, label %for.body, label %for.end

for.end:
  ret void
}

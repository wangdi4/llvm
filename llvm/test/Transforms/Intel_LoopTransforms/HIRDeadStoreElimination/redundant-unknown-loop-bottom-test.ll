; RUN: opt -passes="hir-ssa-deconstruction,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that we keep the redundant bottom test which was exposed as a result
; of performing DSE and copy propagation. Previously we were simply removing it
; which is incorrect as it disrupts the loop structure causing assertion in the
; verifier. Proper handling requires removing the loop node by moving all the
; children outside it and invoking makeConsistent() on them. This is too much
; work considering that this kind of input is unlikely to occur in real test
; cases.

; CHECK: Dump Before

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   land.lhs.true:
; CHECK: |   (@b)[0] = 3;
; CHECK: |   (@a)[0] = 1;
; CHECK: |   %ld = (@b)[0];
; CHECK: |   (@b)[0] = %ld + -1;
; CHECK: |   if (%ld == 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto land.lhs.true;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: modified

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   land.lhs.true:
; CHECK: |   (@a)[0] = 1;
; CHECK: |   (@b)[0] = 2;
; CHECK: |   if (3 == 0)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto land.lhs.true;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i16 0, align 2
@b = internal unnamed_addr global i8 0, align 8

define dso_local i64 @d() local_unnamed_addr #0 {
entry:
  br label %land.lhs.true

land.lhs.true:                                    ; preds = %land.lhs.true, %entry
  store i8 3, ptr @b, align 1
  store i16 1, ptr @a, align 2
  %ld = load i8, ptr @b, align 1
  %dec = add i8 %ld, -1
  store i8 %dec, ptr @b, align 1
  %cmp5 = icmp eq i8 %ld, 0
  br i1 %cmp5, label %land.lhs.true, label %cleanup.cont

cleanup.cont:                                     ; preds = %land.lhs.true
  ret i64 undef
}


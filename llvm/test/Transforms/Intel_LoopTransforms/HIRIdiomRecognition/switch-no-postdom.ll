; RUN: opt -hir-ssa-deconstruction -hir-idiom -print-after=hir-idiom -disable-output -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-idiom,print<hir>" -aa-pipeline="basic-aa" -disable-output -S < %s 2>&1 | FileCheck %s

; Check that the store will not be transformed into memset because it's not executed on every iteration.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (@A)[0][i1];
;       |   switch(%0)
;       |   {
;       |   case 1:
;       |      break;
;       |   case 2:
;       |      break;
;       |   default:
;       |      goto L;
;       |   }
;       |   (@B)[0][i1] = 0;
;       |   L:
;       + END LOOP
; END REGION

; CHECK-LABEL: foo
; CHECK-NOT: memset

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [100 x i32] zeroinitializer, align 16
@B = dso_local global [100 x i32] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.01 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.01 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %idxprom
  %0 = load i32, i32* %arrayidx, align 4
  switch i32 %0, label %sw.default [
    i32 1, label %sw.bb
    i32 2, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  br label %sw.epilog

sw.bb1:                                           ; preds = %for.body
  br label %sw.epilog

sw.default:                                       ; preds = %for.body
  br label %L

sw.epilog:                                        ; preds = %sw.bb1, %sw.bb
  %idxprom2 = sext i32 %i.01 to i64
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %idxprom2
  store i32 0, i32* %arrayidx3, align 4
  br label %L

L:                                                ; preds = %sw.epilog, %sw.default
  %i.02 = phi i32 [ %i.01, %sw.epilog ], [ %i.01, %sw.default ]
  br label %for.inc

for.inc:                                          ; preds = %L
  %inc = add nsw i32 %i.02, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

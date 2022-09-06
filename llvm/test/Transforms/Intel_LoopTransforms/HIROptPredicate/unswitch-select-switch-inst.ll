; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; This test case checks that the Select instruction inside case 0 was not
; converted into If/Else and unswitch wasn't applied. The only unswitching
; applied was for the Switch instruction.

; HIR before transformation

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   switch(%x)
; CHECK:        |   {
; CHECK:        |   case 0:
; CHECK:        |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        |      |   %0 = (%n > 0) ? 0 : 1;
; CHECK:        |      |   (%p)[i2] = i1 + %0;
; CHECK:        |      + END LOOP
; CHECK:        |      break;
; CHECK:        |   case 2:
; CHECK:        |      (%q)[i1] = i1;
; CHECK:        |      break;
; CHECK:        |   default:
; CHECK:        |      (%q)[i1 + 1] = i1;
; CHECK:        |      break;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

; Only the Switch instruction was unswitched.

; CHECK: BEGIN REGION { modified }
; CHECK:       switch(%x)
; CHECK:       {
; CHECK:       case 0:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   |   %0 = (%n > 0) ? 0 : 1;
; CHECK:          |   |   (%p)[i2] = i1 + %0;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 2:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       default:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1 + 1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       }
; CHECK: END REGION



target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32 %x, i32 %n) local_unnamed_addr #0 {
entry:
  %comp = icmp sgt i32 %n, 0
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  switch i32 %x, label %sw.default [
    i32 0, label %sw.bb
    i32 2, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  br label %new.for.body

new.for.body:
  %indvars.iv2 = phi i64 [ 0, %sw.bb ], [ %indvars.iv2.next, %new.for.body ]
  %0 = select i1 %comp, i32 0, i32 1
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv2
  %tr = trunc i64 %indvars.iv to i32
  %a = add i32 %0, %tr
  store i32 %a, i32* %arrayidx, align 4
  %indvars.iv2.next = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv2.next, 100
  br i1 %exitcond2, label %sw.exit, label %new.for.body

sw.exit:
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx3, align 4
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, i32* %q, i64 %2
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, i32* %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}


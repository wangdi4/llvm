; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; This test case checks that the Select instruction inside case 0 wasn't
; converted into If/Else because the loop has a Switch statement. This
; case handles when the Select instruction is outside the Switch.

; HIR before transformation

; Function: foo

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   switch(%x)
;       |   {
;       |   case 0:
;       |      (%p)[i1] = i1;
;       |      break;
;       |   case 2:
;       |      (%q)[i1] = i1;
;       |      break;
;       |   default:
;       |      (%q)[i1 + 1] = i1;
;       |      break;
;       |   }
;       |   %4 = (%n > 0) ? 0 : 1;
;       |   (%p)[i1] = i1 + %4;
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK:  BEGIN REGION { modified }
; CHECK:        switch(%x)
; CHECK:        {
; CHECK:        case 0:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%p)[i1] = i1;
; CHECK:           |   %4 = (%n > 0) ? 0 : 1;
; CHECK:           |   (%p)[i1] = i1 + %4;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        case 2:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%q)[i1] = i1;
; CHECK:           |   %4 = (%n > 0) ? 0 : 1;
; CHECK:           |   (%p)[i1] = i1 + %4;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        default:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%q)[i1 + 1] = i1;
; CHECK:           |   %4 = (%n > 0) ? 0 : 1;
; CHECK:           |   (%p)[i1] = i1 + %4;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        }
; CHECK:  END REGION


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
  %arrayidx1 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx1, align 4
  br label %new.for.preheader

sw.exit:
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx3, align 4
  br label %new.for.preheader

sw.default:                                       ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 1
  %arrayidx5 = getelementptr inbounds i32, i32* %q, i64 %2
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, i32* %arrayidx5, align 4
  br label %new.for.preheader

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

new.for.preheader:
  br label %new.for.body

new.for.body:
  %4 = select i1 %comp, i32 0, i32 1
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %tr = trunc i64 %indvars.iv to i32
  %a = add i32 %4, %tr
  store i32 %a, i32* %arrayidx, align 4
  br label %new.exit

new.exit:
  br label %for.inc
}


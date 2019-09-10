; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-before=hir-opt-predicate -print-after=hir-opt-predicate %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" %s 2>&1 | FileCheck %s

; Check simple loop switch un-switch.

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
;       + END LOOP
; END REGION

; CHECK: BEGIN REGION { modified }
; CHECK:        switch(%x)
; CHECK:        {
; CHECK:        case 0:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%p)[i1] = i1;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        case 2:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%q)[i1] = i1;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        default:
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   (%q)[i1 + 1] = i1;
; CHECK:           + END LOOP
; CHECK:           break;
; CHECK:        }
; CHECK:  END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32 %x) local_unnamed_addr #0 {
entry:
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
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
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


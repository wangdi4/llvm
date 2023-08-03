; REQUIRES: asserts
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -hir-opt-predicate-max-cases-threshold=8 -debug-only=hir-opt-predicate %s 2>&1 | FileCheck %s

; This test case checks that HIROptPredicate was applied because the
; number of cases in the switch statement is smaller than the max threshold.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   switch(%x)
;       |   {
;       |   case 0:
;       |      (%p)[i1] = i1;
;       |      break;
;       |   case 1:
;       |      (%q)[i1] = i1;
;       |      break;
;       |   case 2:
;       |      (%q)[i1 + 1] = i1;
;       |      break;
;       |   case 3:
;       |      (%q)[i1 + 2] = i1;
;       |      break;
;       |   default:
;       |      (%q)[i1 + 3] = i1;
;       |      break;
;       |   }
;       + END LOOP
; END REGION

; Check that there is one candidate for the optimization since the number of
; cases is smaller than the threshold.
; CHECK: Opt Predicate for Function: foo
; CHECK: Region: 0:
; CHECK-NOT: Disabling opportunity for
; CHECK: Opportunity: <2>          switch(%x) --> Level 0, Candidate: Yes
; CHECK: Candidates, count: 1
; CHECK: {<2>          switch(%x), L: 0, PU: [ F/F ]}
; CHECK: Unswitching loop <43>:
; CHECK: H: {<2>          switch(%x), L: 0, PU: [ F/F ]}

; CHeck that the HIR was updated correctly
; CHECK: BEGIN REGION { modified }
; CHECK:       switch(%x)
; CHECK:       {
; CHECK:       case 0:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%p)[i1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 2:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1 + 1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 1:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1] = i1;
; CHECK:          + END LOOP
; CHECK:          break;
; CHECK:       case 3:
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1 + 1] = i1;
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

define dso_local void @foo(ptr nocapture %p, ptr nocapture %q, i32 %x) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  switch i32 %x, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
    i32 2, label %sw.bb2
    i32 3, label %sw.bb3
  ]

sw.bb:                                            ; preds = %for.body
  %idx0 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %idx0, align 4
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %idx1 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, ptr %idx1, align 4
  br label %for.inc

sw.bb2:                                       ; preds = %for.body
  %2 = add nuw nsw i64 %indvars.iv, 1
  %idx2 = getelementptr inbounds i32, ptr %q, i64 %2
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, ptr %idx2, align 4
  br label %for.inc

sw.bb3:                                       ; preds = %for.body
  %4 = add nuw nsw i64 %indvars.iv, 1
  %idx3 = getelementptr inbounds i32, ptr %q, i64 %4
  %5 = trunc i64 %indvars.iv to i32
  store i32 %5, ptr %idx3, align 4
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %6 = add nuw nsw i64 %indvars.iv, 1
  %idx.def = getelementptr inbounds i32, ptr %q, i64 %6
  %7 = trunc i64 %indvars.iv to i32
  store i32 %7, ptr %idx.def, align 4
  br label %for.inc

for.inc:                                          ; preds = %sw.bb, %sw.bb1, %sw.bb2, %sw.bb3, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void
}


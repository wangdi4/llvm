; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -disable-output -S < %s 2>&1 | FileCheck %s --check-prefixes="REGULAR"
; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output -S < %s 2>&1 | FileCheck %s --check-prefixes="REGULAR"

; RUN: opt -hir-opt-predicate-early-opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -disable-output -S < %s 2>&1 | FileCheck %s --check-prefixes="EARLY"
; RUN: opt -hir-opt-predicate-early-opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output -S < %s 2>&1 | FileCheck %s --check-prefixes="EARLY"

; Check that hir-opt-predicate will not hoist 'if (i1 >u 50)' out of the
; innermost loop if "keep-perfect" (early predicate opt) mode is active because
; the innermost loop is complete unroll candidate.

; BEGIN REGION { }
;      + DO i1 = 0, 99, 1   <DO_LOOP>
;      |   %ld = (%a)[i1][0];
;      |   if (%ld > 4)
;      |   {
;      |      (%a)[i1][0] = 10;
;      |   }
;      |
;      |   + DO i2 = 0, 2, 1   <DO_LOOP>
;      |   |   if (i1 >u 50)
;      |   |   {
;      |   |      (%a)[i1][i2] = i1 + i2;
;      |   |   }
;      |   |   else
;      |   |   {
;      |   |      (%a)[i1][i2] = i1 + -1 * i2;
;      |   |   }
;      |   + END LOOP
;      + END LOOP
; END REGION

; REGULAR: modified
; REGULAR: DO i1
; REGULAR: if (i1 >u 50)
; REGULAR: DO i2
; REGULAR: DO i2


; EARLY-NOT: modified
; EARLY: DO i1
; EARLY-NOT: if (i1 >u 50)
; EARLY: DO i2
; EARLY: if (i1 >u 50)
; EARLY-NOT: DO i2


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable writeonly
define dso_local void @foo([100 x i32]* nocapture %a) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %indvars.iv40 = phi i64 [ 0, %entry ], [ %indvars.iv.next41, %for.cond.cleanup3 ]
  %cmp5 = icmp ugt i64 %indvars.iv40, 50
  %0 = add nsw i64 %indvars.iv40, -1
  %arrayidx4 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 %indvars.iv40, i64 0
  %ld = load i32, i32* %arrayidx4, align 4
  %cmp1 = icmp sgt i32 %ld, 4
  br i1 %cmp1, label %if.outer.then, label %if.outer.merge

if.outer.then:
  store i32 10, i32* %arrayidx4, align 4
  br label %if.outer.merge

if.outer.merge:
  br label %for.body4

for.body4:                                        ; preds = %for.inc, %if.outer.merge
  %indvars.iv = phi i64 [ 0, %if.outer.merge ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %for.body4
  %1 = add nuw nsw i64 %indvars.iv, %indvars.iv40
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 %indvars.iv40, i64 %indvars.iv
  %2 = trunc i64 %1 to i32
  store i32 %2, i32* %arrayidx7, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body4
  %3 = sub nsw i64 %indvars.iv40, %indvars.iv
  %arrayidx16 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 %indvars.iv40, i64 %indvars.iv
  %4 = trunc i64 %3 to i32
  store i32 %4, i32* %arrayidx16, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup3:                                ; preds = %for.inc
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %exitcond43 = icmp eq i64 %indvars.iv.next41, 100
  br i1 %exitcond43, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void
}


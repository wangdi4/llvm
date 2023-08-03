; RUN: opt -hir-create-function-level-region -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that after we DSE (@abm)[0][9][9][9] by replacing store and
; corresponding load with temps, copy propagation utility does the right thing
; by not propagating use of %ld across its definition. Copies like this are
; not valid copy candidates due to live range violation-
;
; %ld = %ld + 1;

; HIR before unroll-
; + DO i1 = 0, 3, 1   <DO_LOOP>
; |   + DO i2 = 0, i1 + 9, 1   <DO_LOOP>  <MAX_TC_EST = 13>  <LEGAL_MAX_TC = 13>
; |   |   %ld = (@abm)[0][i1 + 9][i1 + 9][i1 + 9];
; |   |   (@abm)[0][i1 + 9][i1 + 9][i1 + 9] = %ld + 1;
; |   + END LOOP
; + END LOOP

; Partial dumps-

; CHECK: Dump Before

; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;
; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK: (@abm)[0][9][9][9] = %ld + 1;


; CHECK: Dump After

; CHECK: %ld = (@abm)[0][9][9][9];
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: %ld = %ld + 1;
; CHECK-NEXT: (@abm)[0][9][9][9] = %ld + 1;


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@abm = dso_local local_unnamed_addr global [54 x [54 x [54 x i32]]] zeroinitializer, align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc15
  %indvars.iv36 = phi i64 [ 9, %entry ], [ %indvars.iv.next37, %for.inc15 ]
  %indvars.iv34 = phi i64 [ 10, %entry ], [ %indvars.iv.next35, %for.inc15 ]
  %arrayidx13 = getelementptr inbounds [54 x [54 x [54 x i32]]], ptr @abm, i64 0, i64 %indvars.iv36, i64 %indvars.iv36, i64 %indvars.iv36
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %ld = load i32, ptr %arrayidx13, align 4
  %inc = add i32 %ld, 1
  store i32 %inc, ptr %arrayidx13, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %indvars.iv34
  br i1 %exitcond, label %for.inc15, label %for.body3

for.inc15:                                        ; preds = %for.body3
  %indvars.iv.next37 = add nuw nsw i64 %indvars.iv36, 1
  %indvars.iv.next35 = add nuw nsw i64 %indvars.iv34, 1
  %exitcond38.not = icmp eq i64 %indvars.iv.next37, 13
  br i1 %exitcond38.not, label %for.end17, label %for.cond1.preheader

for.end17:                                        ; preds = %for.inc15
  ret i32 0
}


; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions -disable-output 2>&1 | FileCheck %s

; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions -hir-create-function-level-region-filter-func=foo | FileCheck %s --check-prefix=ONEREG1
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions -disable-output 2>&1 -hir-create-function-level-region-filter-func=foo | FileCheck %s --check-prefix=ONEREG1

; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions -hir-create-function-level-region-filter-func=bar | FileCheck %s --check-prefix=ONEREG2
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions -disable-output 2>&1 -hir-create-function-level-region-filter-func=bar | FileCheck %s --check-prefix=ONEREG2

; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -hir-create-function-level-region -disable-hir-create-fusion-regions | FileCheck %s --check-prefix=ONEREGALL
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-output 2>&1 -hir-create-function-level-region -disable-hir-create-fusion-regions | FileCheck %s --check-prefix=ONEREGALL

; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -hir-create-function-level-region -hir-create-function-level-region-filter-func=bar -disable-hir-create-fusion-regions | FileCheck %s --check-prefix=ONEREG3
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-output 2>&1 -hir-create-function-level-region -hir-create-function-level-region-filter-func=bar -disable-hir-create-fusion-regions | FileCheck %s --check-prefix=ONEREG3

; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions -hir-create-function-level-region-filter-func=foo,bar | FileCheck %s --check-prefix=ONEREG4
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions -disable-output 2>&1 -hir-create-function-level-region-filter-func=foo,bar | FileCheck %s --check-prefix=ONEREG4

; Verify the behavior of -hir-create-function-level-region -hir-create-function-level-region-filter-func.


; INPUT:
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   (%a)[i1] = i1;
;    + END LOOP
;
;
;    + DO i1 = 0, 99, 1   <DO_LOOP>
;    |   %1 = (%a)[i1];
;    |   (%b)[i1] = %1 + 1;
;    + END LOOP
;
;    ret ;

; default no one region
; CHECK: Region 1
; CHECK: Region 2

; With a right name, one region
; ONEREG1: Region 1
; ONEREG1-NOT: Region 2

; With a wrong name, no one region
; ONEREG2: Region 1
; ONEREG2: Region 2

; With a wrong name, no one region, even with -hir-create-function-level-region
; ONEREG3: Region 1
; ONEREG3: Region 2

; One region regardless of name, only with -hir-create-function-level-region
; ONEREGALL: Region 1
; ONEREGALL-NOT: Region 2

; A right name in multiple names, one region
; ONEREG4: Region 1
; ONEREG4-NOT: Region 2


source_filename = "enclosed-simple.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* nocapture %b, i32 %n) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv24 = phi i64 [ 0, %entry ], [ %indvars.iv.next25, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv24
  %0 = trunc i64 %indvars.iv24 to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %exitcond26 = icmp eq i64 %indvars.iv.next25, 100
  br i1 %exitcond26, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  ret void

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx7, align 4
  %add = add nsw i32 %1, 1
  %arrayidx9 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  store i32 %add, i32* %arrayidx9, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



; Verify that the detached nodes will not be invalidated:
;
; do i1 {
;   if (i1 == 0) {
;   do i2
;     if (i2 == %b)
;   }
; }
;
; 1) do i2 will be split into three loops. Where the original
;    loop is the first one.
; 2) The original loop will be added to the NodesToInvalidate.
; 3) Then, do i1 loop will be split into two loops, a first one
;    will be removed as redundant, making the original loop
;    from 1) detached.

; RUN: opt -hir-ssa-deconstruction -hir-dd-analysis -hir-dd-analysis-dump-nodes=-1 -hir-opt-var-predicate -print-after=hir-opt-var-predicate -disable-output < %s 2>&1 | FileCheck %s

; Note: -hir-dd-analysis is used to construct DDGraph before the var predicate to catch the issue while invalidation.

; SOURCE:
; void foo(char *a, int n, int b) {
;   int i,j;
;   for (i=0;i<100;i++) {
;     if (i == 0) {
;       for (j=0;j<100;j++) {
;         if (j < b) {
;           a[i] += j;
;         }
;       }
;     }
;   }
; }

; CHECK: BEGIN REGION

;Module Before HIR; ModuleID = '1.c'
source_filename = "1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i8* nocapture %a, i32 %n, i32 %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc9, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc9 ]
  %cmp1 = icmp eq i64 %indvars.iv, 0
  br i1 %cmp1, label %for.body4.preheader, label %for.inc9

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.inc
  %j.021 = phi i32 [ %inc, %for.inc ], [ 0, %for.body4.preheader ]
  %cmp5 = icmp slt i32 %j.021, %b
  br i1 %cmp5, label %if.then6, label %for.inc

if.then6:                                         ; preds = %for.body4
  %0 = load i8, i8* %a, align 1
  %conv20 = zext i8 %0 to i32
  %add = add i32 %conv20, %j.021
  %conv7 = trunc i32 %add to i8
  store i8 %conv7, i8* %a, align 1
  br label %for.inc

for.inc:                                          ; preds = %for.body4, %if.then6
  %inc = add nuw nsw i32 %j.021, 1
  %exitcond = icmp eq i32 %inc, 100
  br i1 %exitcond, label %for.inc9.loopexit, label %for.body4

for.inc9.loopexit:                                ; preds = %for.inc
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond23, label %for.end11, label %for.body

for.end11:                                        ; preds = %for.inc9
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


; Transformation of the outer loops considered non-benificial

; RUN: opt -hir-ssa-deconstruction -S -print-after=hir-opt-var-predicate -disable-output -hir-opt-var-predicate -disable-hir-opt-var-predicate-cost-model < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -S -print-after=hir-opt-var-predicate -disable-output -hir-opt-var-predicate < %s 2>&1 | FileCheck %s -check-prefix=CM-CHECK

; Source code:
;
; void foo(int *p, int *q, long n, long d) {
;   int i,j;
;   for (i=0;i<n;++i) {
;   for (j=0;j<n;++j) {
;     if (d < i) {
;       p[j]++;
;     }
;   }
; }
;
; *** IR Dump Before HIR Var OptPredicate ***
; Function: foo
;
;           BEGIN REGION { }
; <23>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <22>            |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; <7>             |   |   if (i1 > %d)
; <7>             |   |   {
; <12>            |   |      %0 = (%p)[i2];
; <14>            |   |      (%p)[i2] = %0 + 1;
; <7>             |   |   }
; <22>            |   + END LOOP
; <23>            + END LOOP
;           END REGION
;
; CHECK: *** IR Dump After
; Function: foo
;
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i1 = 0, %n + -1 * smax(0, (1 + %d)) + -1, 1   <DO_LOOP>
; CHECK:          |   + DO i2 = 0, zext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK:          |   |   %0 = (%p)[i2];
; CHECK:          |   |   (%p)[i2] = %0 + 1;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:    END REGION

; CM-CHECK: *** IR Dump After
; CM-CHECK: BEGIN REGION { }

;Module Before HIR; ModuleID = 'iv-outer.c'
source_filename = "iv-outer.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32 %n, i32 %d) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %n, 0
  br i1 %cmp18, label %for.body3.lr.ph.preheader, label %for.end8

for.body3.lr.ph.preheader:                        ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc6
  %i.019 = phi i32 [ %inc7, %for.inc6 ], [ 0, %for.body3.lr.ph.preheader ]
  %cmp4 = icmp sgt i32 %i.019, %d
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %0, 1
  store i32 %inc, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.inc
  %inc7 = add nuw nsw i32 %i.019, 1
  %exitcond20 = icmp eq i32 %inc7, %n
  br i1 %exitcond20, label %for.end8.loopexit, label %for.body3.lr.ph

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }



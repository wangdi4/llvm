; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -S -print-after=hir-opt-var-predicate -print-before=hir-opt-var-predicate -disable-output  < %s 2>&1 | FileCheck %s

; Source code:
;
; void foo(int *p, int *q, long n, long d) {
;   long j;
;   for (j=0;j<n;++j) {
;     if (j == d) {
;       p[j] = j;
;     } else {
;       q[j] = j;
;     }
;   }
; }
;
; *** IR Dump Before HIR Var OptPredicate ***
; Function: foo
;
;           BEGIN REGION { }
; <16>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <5>             |   if (i1 == %d)
; <5>             |   {
; <14>            |      (%p)[i1] = i1;
; <5>             |   }
; <5>             |   else
; <5>             |   {
; <10>            |      (%q)[i1] = i1;
; <5>             |   }
; <16>            + END LOOP
;           END REGION
;
; *** IR Dump After HIR Var OptPredicate ***
; Function: foo
;
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i1 = 0, (-1 + (-1 * smax((-1 * %n), (-1 * %d)))), 1   <DO_LOOP>
; CHECK:          |   (%q)[i1] = i1;
; CHECK:          + END LOOP
;
; CHECK:          + DO i1 = 0, (-1 + (-1 * smax((-1 + (-1 * %d)), (-1 * %n)))) + -1 * smax(0, %d), 1   <DO_LOOP>
; CHECK:          |   (%p)[%d] = i1 + smax(0, %d);
; CHECK:          + END LOOP
;
; CHECK:          + DO i1 = 0, %n + -1 * smax(0, (1 + %d)) + -1, 1   <DO_LOOP>
; CHECK:          |   (%q)[i1 + smax(0, (1 + %d))] = i1 + smax(0, (1 + %d));
; CHECK:          + END LOOP
; CHECK:    END REGION

;Module Before HIR; ModuleID = 'iv-eq.c'
source_filename = "iv-eq.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture %q, i64 %n, i64 %d) local_unnamed_addr #0 {
entry:
  %cmp9 = icmp sgt i64 %n, 0
  br i1 %cmp9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %d
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %j.010 = phi i64 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %cmp1 = icmp eq i64 %j.010, %d
  %conv = trunc i64 %j.010 to i32
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store i32 %conv, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %q, i64 %j.010
  store i32 %conv, i32* %arrayidx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i64 %j.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }



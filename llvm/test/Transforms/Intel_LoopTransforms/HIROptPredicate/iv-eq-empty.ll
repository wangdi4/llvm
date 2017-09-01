; RUN: opt -hir-ssa-deconstruction -hir-opt-var-predicate -disable-output -print-after=hir-opt-var-predicate < %s 2>&1 | FileCheck %s

; Source:
; void foo(int *p, int *q, long n, long d) {
;   long j,i;
;   for (j=0;j<n;++j)
;   for (i=0;i<n;++i) {
;     if (j == d) {
;       p[j] += j;
;     }
;   }
; }

; HIR:
; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;       |   + DO i2 = 0, %n + -1, 1   <DO_LOOP>
;       |   |   if (i1 == %d)
;       |   |   {
;       |   |      %0 = (%p)[i1];
;       |   |      (%p)[i1] = %d + zext.i32.i64(%0);
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; The iv predicate optimization will create 3 i1/i2 loop nests, but i2 loops will be empty and they could be removed.
; Check that there is only one DO i1 loop after empty loop removal.

; CHECK:     DO i1
; CHECK-NOT: DO i1

;Module Before HIR; ModuleID = 'iv-eq-empty.c'
source_filename = "iv-eq-empty.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture readnone %q, i64 %n, i64 %d) local_unnamed_addr #0 {
entry:
  %cmp19 = icmp sgt i64 %n, 0
  br i1 %cmp19, label %for.body3.lr.ph.preheader, label %for.end8

for.body3.lr.ph.preheader:                        ; preds = %entry
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc6
  %j.020 = phi i64 [ %inc7, %for.inc6 ], [ 0, %for.body3.lr.ph.preheader ]
  %cmp4 = icmp eq i64 %j.020, %d
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %j.020
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.body3.lr.ph
  %i.018 = phi i64 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  br i1 %cmp4, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body3
  %0 = load i32, i32* %arrayidx, align 4
  %conv16 = zext i32 %0 to i64
  %add = add i64 %conv16, %d
  %conv5 = trunc i64 %add to i32
  store i32 %conv5, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3, %if.then
  %inc = add nuw nsw i64 %i.018, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.inc
  %inc7 = add nuw nsw i64 %j.020, 1
  %exitcond22 = icmp eq i64 %inc7, %n
  br i1 %exitcond22, label %for.end8.loopexit, label %for.body3.lr.ph

for.end8.loopexit:                                ; preds = %for.inc6
  br label %for.end8

for.end8:                                         ; preds = %for.end8.loopexit, %entry
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }



; This is a workaround because of not precise DDG for multiple level
; loopnests. We consider any * outer level DV illegal. Ex.: (* * ?).
; Only outer "=" DVs are handled for multilevel loopnests. Ex.: (= = ?).

; Here is an issue:
; <51>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; <61>               |   %limm64 = (%q)[i1 + 1];
; <53>               |   (%q)[i1] = (%p)[i1];
; <51>               + END LOOP
; <60>                  (%x1)[0] = %limm64;
;
; 61:53 (%q)[i1 + 1] --> (%q)[i1] ANTI (<)
;
; <43>               + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <44>               |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; <59>               |   |   %limm = (%q)[i2 + 1];
; <46>               |   |   (%q)[i2] = (%p)[i2];
; <44>               |   + END LOOP
; <58>               |      (%x1)[0] = %limm;
; <43>               + END LOOP
;
; 59:46 (%q)[i2 + 1] --> (%q)[i2] ANTI (* >) <<<<< this edge is not precise
; 46:59 (%q)[i2] --> (%q)[i2 + 1] FLOW (* <)
; 46:46 (%q)[i2] --> (%q)[i2] OUTPUT (* =)

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; HIR:
; + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; |   + DO i2 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   |   (%x1)[0] = (%q)[i2 + 1];
; |   |   (%q)[i2] = (%p)[i2];
; |   + END LOOP
; + END LOOP

; CHECK: Function
; CHECK-NOT: memcpy

;Module Before HIR; ModuleID = 'innermost-loopnest-workaround.c'
source_filename = "innermost-loopnest-workaround.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr noalias nocapture %q, ptr nocapture readonly %p, i32 %n, ptr nocapture %x1, ptr nocapture readnone %x2) local_unnamed_addr #0 {
entry:
  %cmp23 = icmp sgt i32 %n, 0
  br i1 %cmp23, label %for.body4.preheader.preheader, label %for.cond.cleanup

for.body4.preheader.preheader:                    ; preds = %entry
  %wide.trip.count = zext i32 %n to i64
  br label %for.body4.preheader

for.body4.preheader:                              ; preds = %for.body4.preheader.preheader, %for.cond.cleanup3
  %j.024 = phi i32 [ %inc10, %for.cond.cleanup3 ], [ 0, %for.body4.preheader.preheader ]
  br label %for.body4

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4
  %inc10 = add nuw nsw i32 %j.024, 1
  %exitcond26 = icmp eq i32 %inc10, %n
  br i1 %exitcond26, label %for.cond.cleanup.loopexit, label %for.body4.preheader

for.body4:                                        ; preds = %for.body4, %for.body4.preheader
  %indvars.iv = phi i64 [ 0, %for.body4.preheader ], [ %indvars.iv.next, %for.body4 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx = getelementptr inbounds i32, ptr %q, i64 %indvars.iv.next
  %0 = load i32, ptr %arrayidx, align 4
  store i32 %0, ptr %x1, align 4
  %arrayidx6 = getelementptr inbounds i32, ptr %p, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6, align 4
  %arrayidx8 = getelementptr inbounds i32, ptr %q, i64 %indvars.iv
  store i32 %1, ptr %arrayidx8, align 4
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }



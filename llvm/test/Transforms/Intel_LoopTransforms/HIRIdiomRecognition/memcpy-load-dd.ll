; Check that DD in load prevents idiom hoisting

; RUN: opt -hir-ssa-deconstruction -disable-output -hir-temp-cleanup -hir-idiom -print-after=hir-idiom < %s 2>&1 | FileCheck %s

; HIR:
; + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; |   (%q)[3] = %n;                <<<<< q[3] -> q[i] prevents hoisting of %p[i] store.
; |   (%p)[i1] = (%q)[i1];
; + END LOOP

; CHECK: IR Dump After
; CHECK-NOT: memcpy

;Module Before HIR; ModuleID = 'memcopy-load-dd.c'
source_filename = "memcopy-load-dd.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* noalias nocapture %p, i32* nocapture %q, i32 %n, i32* nocapture readnone %x) local_unnamed_addr #0 {
entry:
  %cmp10 = icmp sgt i32 %n, 0
  br i1 %cmp10, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %arrayidx = getelementptr inbounds i32, i32* %q, i64 3
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret i32 undef

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  store i32 %n, i32* %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx1, align 4
  %arrayidx3 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv
  store i32 %0, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


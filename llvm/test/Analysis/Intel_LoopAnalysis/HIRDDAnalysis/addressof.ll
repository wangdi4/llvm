; Check that there is no edge between memory reference and addressof reference

; RUN: opt -hir-ssa-deconstruction -analyze -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s

; HIR:
; <0>       BEGIN REGION { }
; <11>            + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; <4>             |   (%a)[i1] = &((%b)[i1]);
; <11>            + END LOOP
; <0>       END REGION

; CHECK: DD graph for function
; CHECK-NOT: --> 

;Module Before HIR; ModuleID = 'addressof.c'
source_filename = "addressof.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: norecurse nounwind
define void @foo(i32** nocapture %a, i32* %b, i32** nocapture readnone %c, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %n, 0
  br i1 %cmp6, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.07 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i32 %i.07
  %arrayidx1 = getelementptr inbounds i32*, i32** %a, i32 %i.07
  store i32* %arrayidx, i32** %arrayidx1, align 4
  %inc = add nuw nsw i32 %i.07, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }



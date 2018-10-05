; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -vplan-force-vf=8 < %s 2>&1 | FileCheck %s

; Verify that vectorizer generates a scatter for this loop due to possible wraparound.

;  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;  |   (%A)[zext.i3.i64(i1)] = i1;
;  + END LOOP

; CHECK: + DO i1 = 0, 8 * %tgu + -1, 8   <DO_LOOP>  <MAX_TC_EST = 536870911>
; CHECK: |   (<8 x i32>*)(%A)[i1 + <i3 0, i3 1, i3 2, i3 3, i3 -4, i3 -3, i3 -2, i3 -1>] = i1 + <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK: + END LOOP


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -print-after=VPlanDriverHIR -vplan-force-vf=8 -hir-ignore-wraparound < %s 2>&1 | FileCheck %s --check-prefix=IGNORE-WRAP

; Verify that -hir-ignore-wraparound  works as expected.
; IGNORE-WRAP: (<8 x i32>*)(%A)[i1]


;Module Before HIR; ModuleID = 'wrap1.c'
source_filename = "wrap1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32* nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %i.06 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %and = and i32 %i.06, 7
  %0 = zext i32 %and to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  store i32 %i.06, i32* %arrayidx, align 4, !tbaa !2
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7277ef25cac670b925c89fab076a39a389835fc5) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 4617c4591c7662e130fbfa02515e1147cbe2e049)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}


; INTEL_FEATURE_CSA
; RUN: opt -passes="hir-ssa-deconstruction,require<hir-parvec-analysis>,hir-recognize-par-loop,print<hir>" -S < %s 2>&1 | FileCheck %s
;
; REQUIRES: csa-registered-target
;
; The test checks if hir-recognize-par-loop pass is able to identify parallel loops
; even without `#pragma omp parallel for`.

target triple = "csa"

; CHECK:    <DO_LOOP>{{.*}}<parallel>

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @offload(ptr nocapture %ip, i32 %n) local_unnamed_addr #0 {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, ptr %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable writeonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ccf510eec9f97a259051c79781d5f966addcad35) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c4ddd82a0f21f093b3cf7b94932990c38430e7cd)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
; end INTEL_FEATURE_CSA

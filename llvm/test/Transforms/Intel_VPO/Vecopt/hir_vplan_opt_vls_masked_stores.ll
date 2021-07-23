; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -debug-only=vplan-vls-analysis 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" \
; RUN: -debug-only=vplan-vls-analysis 2>&1 | FileCheck %s
;
; REQUIRES: asserts

; CHECK-NOT: VLSA: Added instruction

; Check that masked memrefs are not collected for OptVLS.
; This test must be fixed with proper support of a mask in CG and in OptVLS
; itself.

@a = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 4
@b = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 4

; Function Attrs: norecurse nounwind uwtable
define dso_local void @_Z20masked_strided_storei(i32 %c_size) local_unnamed_addr #0 {
entry:
  %cmp13 = icmp sgt i32 %c_size, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.014 = phi i32 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  %and = and i32 %i.014, 5
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul = mul nsw i32 %i.014, 3
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @b, i32 0, i32 %mul, !intel-tbaa !3
  store i32 1, i32* %arrayidx, align 4, !tbaa !3
  %add2 = add nuw nsw i32 %mul, 1
  %arrayidx3 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i32 0, i32 %add2, !intel-tbaa !3
  store i32 2, i32* %arrayidx3, align 4, !tbaa !3
  %add5 = add nuw nsw i32 %mul, 2
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i32 0, i32 %add5, !intel-tbaa !3
  store i32 3, i32* %arrayidx6, align 4, !tbaa !3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i32 %i.014, 1
  %exitcond = icmp eq i32 %inc, %c_size
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e63da525aa53f760aaab01045ae5cb3a1f296fbf) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e6ed64d1f04f15c988f51cf9c223d6e32bb90156)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}

; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-post-vec-complete-unroll -analyze -hir-safe-reduction-analysis  | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-post-vec-complete-unroll,print<hir-safe-reduction-analysis>" -disable-output 2>&1 | FileCheck %s

; Checks that safe reduction is recognized after loop unrolling

;*** IR Dump After HIR PostVec Complete Unroll ***
;
; BEGIN REGION { modified }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %t.019 = (@A)[0][i1][0]  +  %t.019;
;       |   %t.019 = (@A)[0][i1][1]  +  %t.019;
;       |   %t.019 = (@A)[0][i1][2]  +  %t.019;
;       + END LOOP
; END REGION

; CHECK: %t.019 = (@A)[0][i1][0]  +  %t.019; <Safe Reduction>
; CHECK: %t.019 = (@A)[0][i1][1]  +  %t.019; <Safe Reduction>
; CHECK: %t.019 = (@A)[0][i1][2]  +  %t.019; <Safe Reduction>
;
;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc6, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.inc6 ]
  %t.019 = phi i32 [ 0, %entry ], [ %add.lcssa, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.117 = phi i32 [ %t.019, %for.cond1.preheader ], [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %indvars.iv20, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %add = add nsw i32 %0, %t.117
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 100
  br i1 %exitcond22, label %for.end8, label %for.cond1.preheader

for.end8:                                         ; preds = %for.inc6
  %add.lcssa.lcssa = phi i32 [ %add.lcssa, %for.inc6 ]
  ret i32 %add.lcssa.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 75251c447951a5a8c1526f5e9b69dfb5d68bce8e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 51bdaaba1481306c0aa879974d77f4932cb0aa70)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA100_A100_i", !4, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

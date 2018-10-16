; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-unroll-and-jam -analyze -hir-dd-analysis -hir-dd-analysis-verify=Innermost < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir-dd-analysis>" -hir-dd-analysis-verify=Innermost -disable-output 2>&1 < %s | FileCheck %s

; unroll and jam reuses instructions from original loopnnest when the remainder loop is not needed. Verify that we do not retain stale (duplicate) edges from the original loopnest.

; HIR-
; + DO i1 = 0, 1, 1   <DO_LOOP>
; |   %temp = 0;
; |   %t.018 = 0;
; |
; |   + DO i2 = 0, 9, 1   <DO_LOOP>
; |   |   %temp = (@A)[0][i2]  +  %temp;
; |   |   %t.018 = (@A)[0][i2]  +  %t.018;
; |   + END LOOP
; |
; |   (@B)[0][2 * i1] = %temp;
; |   (@B)[0][2 * i1 + 1] = %t.018;
; + END LOOP


; CHECK: %t.018 --> %t.018 FLOW
; CHECK-NOT: %t.018 --> %t.018 FLOW


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %indvars.iv20 = phi i64 [ 0, %entry ], [ %indvars.iv.next21, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.018 = phi i32 [ 0, %for.cond1.preheader ], [ %add, %for.body3 ]
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, %t.018
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add.lcssa = phi i32 [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [10 x i32], [10 x i32]* @B, i64 0, i64 %indvars.iv20
  store i32 %add.lcssa, i32* %arrayidx5, align 4, !tbaa !2
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 4
  br i1 %exitcond22, label %for.end8, label %for.cond1.preheader, !llvm.loop !7

for.end8:                                         ; preds = %for.end
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 0caec453655197ac8aec9eb86c1dc485b40576dd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c0caa96dee9cccd6f6077f7509bc40b488c895b7)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.unroll_and_jam.count", i32 2}

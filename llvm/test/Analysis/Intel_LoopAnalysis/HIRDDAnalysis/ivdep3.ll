; Test for ivdep on outer loop
;  #pragma ivdep
;  for (i=0; i< 2000; i++) {
;      s = 0;
;    for (j=0; j< 6; j++) {
;        s = s + C[i][j];
;   }
;  A[i] += B[i]+ s + 1000;
;  }
;
; RUN: opt < %s -hir-ssa-deconstruction | opt -hir-dd-analysis -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK: DD graph for function sub:
; CHECK-DAG:  (%B)[i1] --> (%A)[i1] ANTI (=)
; CHECK-DAG:  (@C)[0][i1][i2] --> (%A)[i1] ANTI (=)
;
;Module Before HIR; ModuleID = 'ivdep3.c'
source_filename = "ivdep3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@C = common dso_local local_unnamed_addr global [1000 x [100 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @sub(i32 %N1, float* nocapture %A, float* nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %indvars.iv29 = phi i64 [ 0, %entry ], [ %indvars.iv.next30, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %s.027 = phi float [ 0.000000e+00, %for.cond1.preheader ], [ %add, %for.body3 ]
  %arrayidx5 = getelementptr inbounds [1000 x [100 x float]], [1000 x [100 x float]]* @C, i64 0, i64 %indvars.iv29, i64 %indvars.iv
  %0 = load float, float* %arrayidx5, align 4, !tbaa !2
  %add = fadd float %s.027, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %add.lcssa = phi float [ %add, %for.body3 ]
  %arrayidx7 = getelementptr inbounds float, float* %B, i64 %indvars.iv29
  %1 = load float, float* %arrayidx7, align 4, !tbaa !8
  %add8 = fadd float %add.lcssa, %1
  %add9 = fadd float %add8, 1.000000e+03
  %arrayidx11 = getelementptr inbounds float, float* %A, i64 %indvars.iv29
  %2 = load float, float* %arrayidx11, align 4, !tbaa !8
  %add12 = fadd float %2, %add9
  store float %add12, float* %arrayidx11, align 4, !tbaa !8
  %indvars.iv.next30 = add nuw nsw i64 %indvars.iv29, 1
  %exitcond31 = icmp eq i64 %indvars.iv.next30, 2000
  br i1 %exitcond31, label %for.end15, label %for.cond1.preheader, !llvm.loop !9

for.end15:                                        ; preds = %for.end
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 7277ef25cac670b925c89fab076a39a389835fc5) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 372f88114e0e88951a1653bd9fe5cf3cb313b50f)"}
!2 = !{!3, !5, i64 0}
!3 = !{!"array@_ZTSA1000_A100_f", !4, i64 0}
!4 = !{!"array@_ZTSA100_f", !5, i64 0}
!5 = !{!"float", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!5, !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.vectorize.ivdep_back"}

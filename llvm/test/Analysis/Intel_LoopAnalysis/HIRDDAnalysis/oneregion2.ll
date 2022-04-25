; float B[1000];
; int C[1000];
;   n =1000;
;   for (j=0; j <  n; j++)
;       B[j] =  1;
;   B[2] = 3;
;   C[2] = B[m];
;    for (j=0; j <  n; j++)
;        B[j] +=  1;
;
; RUN: opt < %s -hir-ssa-deconstruction -hir-create-function-level-region  -hir-dd-analysis  -hir-dd-analysis-verify=Region -analyze -enable-new-pm=0  | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-create-function-level-region -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s
;
; CHECK-DAG:  (@B)[0][2] --> (@B)[0][%m] FLOW (=)
; CHECK-DAG:  (@B)[0][2] --> (@B)[0][i1] FLOW (=)
; CHECK-DAG:  (@B)[0][2] --> (@B)[0][i1] OUTPUT (=)
;

;Module Before HIR; ModuleID = 'oneregion2.c'
source_filename = "oneregion2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16
@C = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub2(i32 %n, i32 %m) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv22
  store float 1.000000e+00, float* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 1000
  br i1 %exitcond24, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  store float 3.000000e+00, float* getelementptr inbounds ([1000 x float], [1000 x float]* @B, i64 0, i64 2), align 8, !tbaa !2
  %idxprom1 = sext i32 %m to i64
  %arrayidx2 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %idxprom1
  %0 = load float, float* %arrayidx2, align 4, !tbaa !2
  %conv = fptosi float %0 to i32
  store i32 %conv, i32* getelementptr inbounds ([1000 x i32], [1000 x i32]* @C, i64 0, i64 2), align 8, !tbaa !7
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.end
  %indvars.iv = phi i64 [ 0, %for.end ], [ %indvars.iv.next, %for.body6 ]
  %arrayidx8 = getelementptr inbounds [1000 x float], [1000 x float]* @B, i64 0, i64 %indvars.iv
  %1 = load float, float* %arrayidx8, align 4, !tbaa !2
  %add = fadd float %1, 1.000000e+00
  store float %add, float* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end11, label %for.body6

for.end11:                                        ; preds = %for.body6
  ret void
}
attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1000_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA1000_i", !9, i64 0}
!9 = !{!"int", !5, i64 0}


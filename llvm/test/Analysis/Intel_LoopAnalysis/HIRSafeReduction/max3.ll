; for(i=0; i<n; i++) {
;   j = j >= A[i] ? j : A[i];
; }

; 'max' recognition for safe reduction analysis, case of floats

; RUN: opt < %s -hir-ssa-deconstruction -analyze -hir-temp-cleanup -force-hir-safe-reduction-analysis -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -force-hir-safe-reduction-analysis 2>&1 | FileCheck %s

; CHECK:   %j.011 = (%j.011 >= %0) ? %j.011 : %0; <Safe Reduction>

;Module Before HIR; ModuleID = 'max3.c'
source_filename = "max3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = local_unnamed_addr constant i32 100, align 4
@A = external local_unnamed_addr global [100 x float], align 16

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %j.011 = phi float [ 1.000000e+00, %entry ], [ %.j.0, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %cmp1.inv = fcmp oge float %j.011, %0
  %.j.0 = select i1 %cmp1.inv, float %j.011, float %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %.j.0.lcssa = phi float [ %.j.0, %for.body ]
  %conv = fptosi float %.j.0.lcssa to i32
  ret i32 %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20398) (llvm/branches/loopopt 20470)"}
!1 = !{!2, !3, i64 0}
!2 = !{!"array@_ZTSA100_f", !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

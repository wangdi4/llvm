; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -analyze  -hir-safe-reduction-analysis -hir-safe-reduction-analysis-print-op | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" -hir-safe-reduction-analysis-print-op -disable-output 2>&1 | FileCheck %s

; CHECK:   %sub = [[VAR:%q.[0-9]+]]  -  (@a)[0][i1]; <Safe Reduction> Red Op: [[REDOP:[0-9]+]]
; CHECK:   [[VAR]] = %sub  +  (@b)[0][i1]; <Safe Reduction> Red Op: [[REDOP]]

; Check if a safe reduction chain is recognized q = q + (@b - @a) 
; when - and + operators are present. Reduction operation is set to the operation found
; in the last instruction of a chain. For this example, RedOp: 14 is FAdd.

;      BEGIN REGION { }
;            + DO i1 = 0, 99, 1   <DO_LOOP>
;            |  %sub = %q.010  -  (@a)[0][i1]; <Safe Reduction> Red Op: 14
;            |  %q.010 = %sub  +  (@b)[0][i1]; <Safe Reduction> Red Op: 14
;            + END LOOP
;      END REGION

; ModuleID = 's.c'
source_filename = "s.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local float @min() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  ret float %add.lcssa

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %q.010 = phi float [ 5.000000e+00, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %sub = fsub float %q.010, %0
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* @b, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load float, float* %arrayidx2, align 4, !tbaa !2
  %add = fadd float %sub, %1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

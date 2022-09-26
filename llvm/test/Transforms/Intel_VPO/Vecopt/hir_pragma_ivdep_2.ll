; C Source:
; void foo(float *b, float *c, float *d)
; {
;   int i;
;
; #pragma ivdep
;   for (i = 0; i < 1024; i++)
;     b[i] += c[i] * d[i];
; }
; RUN: opt -S -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -S -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; CHECK: DO i1 = 0, 1023, 4
;

; ModuleID = 'tivdep2.c'
source_filename = "tivdep2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(float* nocapture %b, float* nocapture readonly %c, float* nocapture readonly %d) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds float, float* %d, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4, !tbaa !2
  %mul = fmul float %0, %1
  %arrayidx4 = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %2 = load float, float* %arrayidx4, align 4, !tbaa !2
  %add = fadd float %2, %mul
  store float %add, float* %arrayidx4, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !6

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang f3ac5fca228b9eac509df1bda97bd6c1cb9ec0c0) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 13eb072a44c133912ac683cdcf769bf80e3aba3d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.vectorize.ivdep_back"}

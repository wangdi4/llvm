;unsigned foo() {
;  unsigned max =255;
;  for (int i = 0; i != 1024; ++i)
;    max = a[i] > max ? a[i] : max;
;  return max;
;}
;
; unsigned max recognition for safe reduction analysis
;
; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-temp-cleanup -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s
;
; CHECK: %max.011 = (%0 >u %max.011) ? %0 : %max.011; <Safe Reduction>
;
;Module Before HIR
; ModuleID = 't4.c'
source_filename = "t4.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %spec.select.lcssa = phi i32 [ %spec.select, %for.body ]
  ret i32 %spec.select.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %max.011 = phi i32 [ 255, %entry ], [ %spec.select, %for.body ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp ugt i32 %0, %max.011
  %spec.select = select i1 %cmp1, i32 %0, i32 %max.011
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp eq i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.cond.cleanup, label %for.body
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="broadwell" "target-features"="+adx,+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_j", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

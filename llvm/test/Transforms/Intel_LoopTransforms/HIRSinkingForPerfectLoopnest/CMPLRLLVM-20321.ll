; Avoid the case when the store node is before the innermost loop
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -print-after=hir-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Sinking For Perfect Loopnest ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<28>               + DO i1 = 0, 61, 1   <DO_LOOP>
;<3>                |   %0 = (%sv)[-1 * i1 + 63];
;<6>                |   (%x)[-1 * i1 + 63] = %0 + 62;
;<29>               |
;<29>               |   + DO i2 = 0, 43, 1   <DO_LOOP>
;<10>               |   |   %1 = (%k)[0];
;<12>               |   |   %2 = (%t)[i2 + 1];
;<14>               |   |   (%t)[i2 + 1] = %1 + %2;
;<29>               |   + END LOOP
;<28>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Sinking For Perfect Loopnest ***
;Function: foo
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 61, 1   <DO_LOOP>
; CHECK:           |   %0 = (%sv)[-1 * i1 + 63];
; CHECK:           |   (%x)[-1 * i1 + 63] = %0 + 62;
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 43, 1   <DO_LOOP>
; CHECK:           |   |   %1 = (%k)[0];
; CHECK:           |   |   %2 = (%t)[i2 + 1];
; CHECK:           |   |   (%t)[i2 + 1] = %1 + %2;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'atg.c'
source_filename = "atg.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @foo(i32* nocapture readonly %sv, i32* nocapture %t, i32* nocapture %x, i32* nocapture readonly %k) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc10
  %indvars.iv21 = phi i64 [ 63, %entry ], [ %indvars.iv.next22, %for.inc10 ]
  %arrayidx = getelementptr inbounds i32, i32* %sv, i64 %indvars.iv21
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %add = add nsw i32 %0, 62
  %arrayidx2 = getelementptr inbounds i32, i32* %x, i64 %indvars.iv21
  store i32 %add, i32* %arrayidx2, align 4, !tbaa !2
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.body
  %indvars.iv = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.body5 ]
  %1 = load i32, i32* %k, align 4, !tbaa !2
  %arrayidx8 = getelementptr inbounds i32, i32* %t, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx8, align 4, !tbaa !2
  %add9 = add nsw i32 %2, %1
  store i32 %add9, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 45
  br i1 %exitcond, label %for.inc10, label %for.body5

for.inc10:                                        ; preds = %for.body5
  %indvars.iv.next22 = add nsw i64 %indvars.iv21, -1
  %cmp = icmp ugt i64 %indvars.iv.next22, 1
  br i1 %cmp, label %for.body, label %for.end11

for.end11:                                        ; preds = %for.inc10
  ret i32 0
}

attributes #0 = { nofree norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

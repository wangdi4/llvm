; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that there is are GEP savings of 3 due to-
; 1. Multiplication of i1 and stride.
; 2. Addition of i1 and i2.
; 3. Addition of trailing offset of 1.

; Savings are multiplied by trip count.
; CHECK: GEPSavings: 30

; CHECK: + DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 9, 1   <DO_LOOP>
; CHECK: |   |   %0 = (%A)[i1][i2].1;
; CHECK: |   |   %t.020 = %t.020  +  %0;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %t.020.out = %t.020;
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, float }

; Function Attrs: norecurse nounwind readonly uwtable
define float @foo(ptr nocapture readonly %A) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc6, %entry
  %indvars.iv21 = phi i64 [ 0, %entry ], [ %indvars.iv.next22, %for.inc6 ]
  %t.020 = phi float [ undef, %entry ], [ %add.lcssa, %for.inc6 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %t.118 = phi float [ %t.020, %for.body ], [ %add, %for.body3 ]
  %f = getelementptr inbounds [10 x %struct.S], ptr %A, i64 %indvars.iv21, i64 %indvars.iv, i32 1
  %0 = load float, ptr %f, align 4, !tbaa !2
  %add = fadd float %t.118, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %add.lcssa = phi float [ %add, %for.body3 ]
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next22, 10
  br i1 %exitcond23, label %for.end8, label %for.body

for.end8:                                         ; preds = %for.inc6
  %add.lcssa.lcssa = phi float [ %add.lcssa, %for.inc6 ]
  ret float %add.lcssa.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 756622f1dabd3965f80d20e0d52127a138802cbd) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5a802fab0ba2e9752da2f60d1d895066c0ebf88a)"}
!2 = !{!3, !7, i64 4}
!3 = !{!"struct@S", !4, i64 0, !7, i64 4}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"float", !5, i64 0}

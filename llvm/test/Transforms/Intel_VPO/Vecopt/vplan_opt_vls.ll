; RUN: opt < %s -S -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -debug \
; RUN:     2>&1 | FileCheck %s
; REQUIRES: asserts

; #include <stdio.h>
; #include <stdlib.h>
; #include <stdint.h>
; #include <math.h>
;
; const int32_t c_size = 10000;
; int32_t a[c_size], b[c_size], c[c_size];
;
; int32_t foo()
; {
;   int32_t ret = 0;
;     for (int32_t i = 0; i < c_size; ++i) {
;         ret += a[3*i] + a[3*i + 1] + a[3*i+2];
;     }
;   return ret;
; }
; Test for OptVLS Interface inside VPlan

; CHECK: VLSA: Added instruction
; CHECK-NEXT: VLSA: Added instruction
; CHECK: Fixed all OVLSTypes for previously collected memrefs.

@a = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16
@c = dso_local local_unnamed_addr global [10000 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind readonly uwtable
define dso_local i32 @_Z3foov() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %add10

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ret.018 = phi i32 [ 0, %entry ], [ %add10, %for.body ]
  %0 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx = getelementptr inbounds [10000 x i32], [10000 x i32]* @a, i64 0, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %2 = add nuw nsw i64 %0, 1
  %arrayidx3 = getelementptr inbounds [10000 x i32], [10000 x i32]* @a, i64 0, i64 %2
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %4 = add nuw nsw i64 %0, 2
  %arrayidx8 = getelementptr inbounds [10000 x i32], [10000 x i32]* @a, i64 0, i64 %4
  %5 = load i32, i32* %arrayidx8, align 4, !tbaa !2
  %add4 = add i32 %1, %ret.018
  %add9 = add i32 %add4, %3
  %add10 = add i32 %add9, %5
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10000
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

attributes #0 = { noinline norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 75251c447951a5a8c1526f5e9b69dfb5d68bce8e) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f194a7002f9e8b3e87b140d6d0a5f67ac540d9ea)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA10000_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}

; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that there are GEP savings of 2 due to-
; 1. Multiplication of i1 and stride in (%ptr)[i1][%idx].1.
; 2. Addition of i1 and the trailing offset 1 for (%ptr)[i1][%idx].1.

; Savings are multiplied by trip count.
; CHECK: GEPSavings: 20

; CHECK:  DO i1 = 0, 9, 1   <DO_LOOP>
; CHECK: |   %0 = (%ptr)[i1][%idx].1;
; CHECK: |   %t.07 = %0  +  %t.07;
; CHECK:  END LOOP

%struct.S = type { float, i32 }

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(ptr nocapture readonly %ptr, i64 %idx) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %t.07 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %d = getelementptr inbounds [10 x %struct.S], ptr %ptr, i64 %indvars.iv, i64 %idx, i32 1
  %0 = load i32, ptr %d, align 4, !tbaa !1
  %add = add nsw i32 %0, %t.07
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ]
  ret i32 %add.lcssa
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="fxsr,mmx,sse,sse2,x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !6, i64 4}
!2 = !{!"struct@S", !3, i64 0, !6, i64 4}
!3 = !{!"float", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C TBAA"}
!6 = !{!"int", !4, i64 0}


; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -analyze -hir-safe-reduction-analysis | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-safe-reduction-analysis>" 2>&1 | FileCheck %s

; Verify that we successfully identify %t2.09's safe reduction by ignoring the flow edge from the other temp operand %mul.

; HIR-
; + DO i1 = 0, 79, 1   <DO_LOOP>
; |   %add1 = (%A)[i1]  +  (%B)[i1];
; |   %t2.09 = %add1  +  %t2.09;
; + END LOOP

; CHECK: + DO i1 = 0, 79, 1   <DO_LOOP>
; CHECK: |   %t2.09 = %add1  +  %t2.09; <Safe Reduction>
; CHECK: + END LOOP

;Module Before HIR; ModuleID = 'red.c'
source_filename = "red.c"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly
define float @foo(float* nocapture readonly %A, float* nocapture readonly %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.010 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %t2.09 = phi float [ undef, %entry ], [ %add, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %A, i32 %i.010
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds float, float* %B, i32 %i.010
  %1 = load float, float* %arrayidx1, align 4, !tbaa !1
  %add1 = fadd float %0, %1
  %add = fadd float %add1, %t2.09
  %inc = add nuw nsw i32 %i.010, 1
  %exitcond = icmp eq i32 %inc, 80
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  ret float %add.lcssa
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="pentium4" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20731) (llvm/branches/loopopt 20741)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

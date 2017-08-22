; Test for infinite loop fix when compiling for MCU target
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -S  < %s | FileCheck %s
; Check for vectorized HIR loop

; CHECK-NOT: fadd <4 x float>
; CHECK: ret void

; ModuleID = 'mcu.c'
source_filename = "mcu.c"
target datalayout = "e-m:e-p:32:32-i64:32-f64:32-f128:32-n8:16:32-a:0:32-S32"
target triple = "i586-intel-elfiamcu"

; Function Attrs: noinline norecurse nounwind
define void @foo(float* noalias nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c, i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.09 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i32 %i.09
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds float, float* %c, i32 %i.09
  %1 = load float, float* %arrayidx1, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx2 = getelementptr inbounds float, float* %a, i32 %i.09
  store float %add, float* %arrayidx2, align 4, !tbaa !1
  %inc = add nuw nsw i32 %i.09, 1
  %exitcond = icmp eq i32 %inc, 1024
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="lakemont" "unsafe-fp-math"="false" "use-soft-float"="true" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21198) (llvm/branches/loopopt 21233)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

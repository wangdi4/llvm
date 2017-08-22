; Test for overflow during cost modeling - check that the loop gets vectorized
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPODriverHIR -hir-cg -S  < %s | FileCheck %s
; Check for vectorized HIR loop

; CHECK: fadd <4 x float>
; CHECK-NEXT: store <4 x float>
; CHECK: nextiv{{.*}} = add {{.*}}, 4

; Generated from the following example using 
; clang -O1 -S -emit-llvm
; void foo(float *restrict a, float *b, float *c, int N){
;   int i,j;
; 
;   for (i=0;i<1000000000;i++){
;      a[i] = b[i] + c[i];
;   } 
; }

; ModuleID = 'loop.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(float* noalias nocapture %a, float* nocapture readonly %b, float* nocapture readonly %c, i32 %N) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float, float* %b, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4, !tbaa !1
  %arrayidx2 = getelementptr inbounds float, float* %c, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4, !tbaa !1
  %add = fadd float %0, %1
  %arrayidx4 = getelementptr inbounds float, float* %a, i64 %indvars.iv
  store float %add, float* %arrayidx4, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000000000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (branches/vpo 1987)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

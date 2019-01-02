; RUN: llc -csa-enable-all-strides <%s | FileCheck %s
; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-3f7a382.bc"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind
define void @.nondebug_wrapper.(double* noalias nocapture readonly %k, double* noalias nocapture %l) #0 {
; CHECK: sld64
; CHECK-SAME: -1
entry:
  %val = load double, double* %l, align 8
  %N = bitcast double %val to i64
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %entry
  %u.addr.04.i = phi i64 [ %inc.i, %for.body.i ], [ 0, %entry ]
  %k.addr.03.i = phi double* [ %incdec.ptr.i, %for.body.i ], [ %k, %entry ]
  %0 = load double, double* %k.addr.03.i, align 8, !tbaa !1, !alias.scope !5, !noalias !8
  %mul19.i = fmul double %0, 0.1
  %add21.i = fadd double 1.0, %mul19.i
  store double %add21.i, double* %l, align 8, !tbaa !11, !noalias !13
  %inc.i = add nuw nsw i64 %u.addr.04.i, 1
  %incdec.ptr.i = getelementptr inbounds double, double* %k.addr.03.i, i64 -1
  %exitcond.i = icmp eq i64 %inc.i, %N
  br i1 %exitcond.i, label %__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982.exit, label %for.body.i

__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982.exit: ; preds = %for.body.i
  ret void
}

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 6.0.0 (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-clang ca0ff8f6a2597cf765cad1835931ca2fe0d1c79e) (ssh://git-amr-3.devtools.intel.com:29418/dcg-knp-arch-lpu-llvm 47e94a08e18c2c20c47d4822db17f5f9fb9c45b3)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"double", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6}
!6 = distinct !{!6, !7, !"__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982: %k"}
!7 = distinct !{!7, !"__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982"}
!8 = !{!9, !10}
!9 = distinct !{!9, !7, !"__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982: %k_pixels"}
!10 = distinct !{!10, !7, !"__omp_offloading_51_6d007e0_MorphologyPrimitive_l2982: %k_indexes"}
!11 = !{!12, !2, i64 24}
!12 = !{!"_DoublePixelPacket", !2, i64 0, !2, i64 8, !2, i64 16, !2, i64 24, !2, i64 32}
!13 = !{!6, !9, !10}

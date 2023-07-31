;RUN: opt -passes="hir-ssa-deconstruction,hir-loop-distribute-loopnest,print<hir>" -aa-pipeline="basic-aa"  < %s 2>&1 | FileCheck %s

; Test checks that distribution happens on liniarized version of matrix multiplication.


; <0>          BEGIN REGION { }
; <43>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <44>               |   + DO i2 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <8>                |   |   (%a)[zext.i32.i64(%N) * i1 + i2] = 0.000000e+00;
; <45>               |   |
; <45>               |   |   + DO i3 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
; <14>               |   |   |   %load = (%b)[zext.i32.i64(%N) * i1 + i3];
; <18>               |   |   |   %load2 = (%c)[i2 + zext.i32.i64(%N) * i3];
; <19>               |   |   |   %mul17.us.us = %load2  *  %load;
; <20>               |   |   |   %load3 = (%a)[zext.i32.i64(%N) * i1 + i2];
; <21>               |   |   |   %add22.us.us = %load3  +  %mul17.us.us;
; <22>               |   |   |   (%a)[zext.i32.i64(%N) * i1 + i2] = %add22.us.us;
; <45>               |   |   + END LOOP
; <44>               |   + END LOOP
; <43>               + END LOOP
; <0>          END REGION


; CHECK: BEGIN REGION { }

; check first loop nest has only initialization of a[][]

; CHECK: DO i1 = 0, zext.i32.i64(%N) + -1, 1
; CHECK:   DO i2 = 0, zext.i32.i64(%N) + -1, 1
; CHECK:     (%a)[zext.i32.i64(%N) * i1 + i2] = 0.000000e+00;
; CHECK:   END LOOP
; CHECK: END LOOP

; second loop has only matmul code

; CHECK: DO i1 = 0, zext.i32.i64(%N) + -1, 1
; CHECK-NEXT:   DO i2 = 0, zext.i32.i64(%N) + -1, 1
; CHECK-NEXT:     DO i3 = 0, zext.i32.i64(%N) + -1, 1
; CHECK-NEXT:       %load = (%b)[zext.i32.i64(%N) * i1 + i3];
; CHECK-NEXT:       %load2 = (%c)[i2 + zext.i32.i64(%N) * i3];
; CHECK-NEXT:       %mul.mm = %load2  *  %load;
; CHECK-NEXT:       %load3 = (%a)[zext.i32.i64(%N) * i1 + i2];
; CHECK-NEXT:       %add.mm = %load3  +  %mul.mm;
; CHECK-NEXT:       (%a)[zext.i32.i64(%N) * i1 + i2] = %add.mm;
; CHECK-NEXT:     END LOOP
; CHECK-NEXT:   END LOOP
; CHECK-NEXT: END LOOP

; CHECK-NEXT: END REGION


target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable mustprogress
define dso_local void @_Z15matrix_multiplyiPdS_S_(i32 %N, ptr noalias nocapture readonly %b, ptr noalias nocapture readonly %c, ptr noalias nocapture %a) local_unnamed_addr #0 {
entry:
  %cmp55 = icmp sgt i32 %N, 0
  br i1 %cmp55, label %for.cond1.preheader.us.us.preheader, label %for.cond.cleanup

for.cond1.preheader.us.us.preheader:              ; preds = %entry
  %g = zext i32 %N to i64
  br label %for.cond1.preheader.us.us

for.cond1.preheader.us.us:                        ; preds = %for.cond1.preheader.us.us.preheader, %for.cond1.for.cond.cleanup3_crit_edge.us.us
  %indvars.iv66 = phi i64 [ 0, %for.cond1.preheader.us.us.preheader ], [ %indvars.iv.next67, %for.cond1.for.cond.cleanup3_crit_edge.us.us ]
  %e = mul nsw i64 %indvars.iv66, %g
  br label %for.body4.us.us

for.cond1.for.cond.cleanup3_crit_edge.us.us:      ; preds = %for.cond5.for.cond.cleanup7_crit_edge.us.us
  %indvars.iv.next67 = add nuw nsw i64 %indvars.iv66, 1
  %exitcond70.not = icmp eq i64 %indvars.iv.next67, %g
  br i1 %exitcond70.not, label %for.cond.cleanup.loopexit, label %for.cond1.preheader.us.us, !llvm.loop !2

for.body4.us.us:                                  ; preds = %for.cond5.for.cond.cleanup7_crit_edge.us.us, %for.cond1.preheader.us.us
  %indvars.iv61 = phi i64 [ %indvars.iv.next62, %for.cond5.for.cond.cleanup7_crit_edge.us.us ], [ 0, %for.cond1.preheader.us.us ]
  %f = add nuw nsw i64 %indvars.iv61, %e
  %ptridx.us.us = getelementptr inbounds double, ptr %a, i64 %f
  store double 0.000000e+00, ptr %ptridx.us.us, align 8, !tbaa !4
  br label %for.body8.us.us

for.cond5.for.cond.cleanup7_crit_edge.us.us:      ; preds = %for.body8.us.us
  %indvars.iv.next62 = add nuw nsw i64 %indvars.iv61, 1
  %exitcond65.not = icmp eq i64 %indvars.iv.next62, %g
  br i1 %exitcond65.not, label %for.cond1.for.cond.cleanup3_crit_edge.us.us, label %for.body4.us.us, !llvm.loop !8

for.body8.us.us:                                  ; preds = %for.body8.us.us, %for.body4.us.us
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body8.us.us ], [ 0, %for.body4.us.us ]
  %add = add nuw nsw i64 %indvars.iv, %e
  %ptridx12.us.us = getelementptr inbounds double, ptr %b, i64 %add
  %load = load double, ptr %ptridx12.us.us, align 8, !tbaa !4
  %mul = mul nsw i64 %indvars.iv, %g
  %add2 = add nuw nsw i64 %mul, %indvars.iv61
  %ptridx16.us.us = getelementptr inbounds double, ptr %c, i64 %add2
  %load2 = load double, ptr %ptridx16.us.us, align 8, !tbaa !4
  %mul.mm = fmul fast double %load2, %load
  %load3 = load double, ptr %ptridx.us.us, align 8
  %add.mm = fadd fast double %load3, %mul.mm
  store double %add.mm, ptr %ptridx.us.us, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %g
  br i1 %exitcond.not, label %for.cond5.for.cond.cleanup7_crit_edge.us.us, label %for.body8.us.us, !llvm.loop !9

for.cond.cleanup.loopexit:                        ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.mustprogress"}
!4 = !{!5, !5, i64 0}
!5 = !{!"double", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = distinct !{!8, !3}
!9 = distinct !{!9, !3}

;RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup   -hir-loop-distribute-memrec    -print-after=hir-loop-distribute-memrec  < %s 2>&1 | FileCheck %s
;RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa"   < %s 2>&1 | FileCheck %s
;  Loop Distribution is expected to happen when there are too many
;   memory references in the loop
; Testing for triangluar loop
;  for (j = 0; j < n; j++) {
;    for (i = j; i < n; i++) {
;      A1[i] += i+j;
;      A2[i] += i-j;
;      A3[i] += i;
;      A4[i] += i;
;      A5[i] += i;
;      A6[i] += i;
;      A7[i] += i;
;      A8[i] += i;
;      B1[i] -= i;
;      ...  etc
;   ==>
;         + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;         |  + DO i2 = 0, (-1 * i1 + sext.i32.i64(%n) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 100>
;         |  |   %min = (-1 * i1 + -64 * i2 + sext.i32.i64(%n) + -1 <= 63) ? -1 * i1 + -64 * i2 + sext.i32.i64(%n)  + -1 : 63;
;         |   |
;         |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;         |   |   |   (@C1)[0][i1 + 64 * i2 + i3] = 1.000000e+00;
;         |   |   |   %conv13 = sitofp.i32.double(i1 + 64 * i2 + i3);
;         |   |   |   (%.TempArray)[0][i3] = %conv13;
;         |   |   END LOOP
;         |   |   + DO i3 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;         |   |   |   %conv13 = (%.TempArray)[0][i3];
;         |   |   |   %sub60 = (@B7)[0][i1 + 64 * i2 + i3]  -  %conv13;
;  Note: just verify for key HIRs
; CHECK: BEGIN REGION
; CHECK-NEXT:  DO i1 = 0, sext.i32.i64(%n) + -1, 1
; CHECK:           DO i2 = 0, -1 * i1 + sext.i32.i64(%n) + -1
; CHECK:             %conv13 = sitofp.i32.double(i1 + i2);
; CHECK:           END LOOP
; CHECK:           DO i2 = 0, -1 * i1 + sext.i32.i64(%n) + -1
; CHECK:             %conv13 = sitofp.i32.double(i1 + i2);
; CHECK:           END LOOP
;
;Module Before HIR; ModuleID = 'scexpansion3.c'
source_filename = "scexpansion3.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A1 = external local_unnamed_addr global [100 x float], align 16
@A2 = external local_unnamed_addr global [100 x float], align 16
@A3 = external local_unnamed_addr global [100 x float], align 16
@A4 = external local_unnamed_addr global [100 x double], align 16
@A5 = external local_unnamed_addr global [100 x double], align 16
@A6 = external local_unnamed_addr global [100 x double], align 16
@A7 = external local_unnamed_addr global [100 x double], align 16
@A8 = external local_unnamed_addr global [100 x double], align 16
@B1 = external local_unnamed_addr global [100 x double], align 16
@B2 = external local_unnamed_addr global [100 x double], align 16
@B3 = external local_unnamed_addr global [100 x double], align 16
@B4 = external local_unnamed_addr global [100 x double], align 16
@B5 = external local_unnamed_addr global [100 x double], align 16
@B6 = external local_unnamed_addr global [100 x double], align 16
@B7 = external local_unnamed_addr global [100 x double], align 16
@B8 = external local_unnamed_addr global [100 x double], align 16
@B9 = external local_unnamed_addr global [100 x double], align 16
@C1 = external local_unnamed_addr global [100 x double], align 16
@C2 = external local_unnamed_addr global [100 x double], align 16
@C3 = external local_unnamed_addr global [100 x double], align 16
@C4 = external local_unnamed_addr global [100 x double], align 16
@C5 = external local_unnamed_addr global [100 x double], align 16
@C6 = external local_unnamed_addr global [100 x double], align 16
@C7 = external local_unnamed_addr global [100 x double], align 16
@C8 = external local_unnamed_addr global [100 x double], align 16
@C9 = external local_unnamed_addr global [100 x double], align 16

; Function Attrs: norecurse nounwind uwtable
define void @SCExpansion1(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp159 = icmp sgt i32 %n, 0
  br i1 %cmp159, label %for.body.lr.ph, label %for.end97

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body.lr.ph, %for.inc95
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.inc95 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv162 = phi i64 [ %indvars.iv, %for.body3.lr.ph ], [ %indvars.iv.next163, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv162, %indvars.iv
  %1 = trunc i64 %0 to i32
  %conv = sitofp i32 %1 to float
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A1, i64 0, i64 %indvars.iv162
  %2 = load float, float* %arrayidx, align 4, !tbaa !2
  %add4 = fadd float %2, %conv
  store float %add4, float* %arrayidx, align 4, !tbaa !2
  %3 = sub nuw nsw i64 %indvars.iv162, %indvars.iv
  %4 = trunc i64 %3 to i32
  %conv5 = sitofp i32 %4 to float
  %arrayidx7 = getelementptr inbounds [100 x float], [100 x float]* @A2, i64 0, i64 %indvars.iv162
  %5 = load float, float* %arrayidx7, align 4, !tbaa !2
  %add8 = fadd float %5, %conv5
  store float %add8, float* %arrayidx7, align 4, !tbaa !2
  %6 = trunc i64 %indvars.iv162 to i32
  %conv9 = sitofp i32 %6 to float
  %arrayidx11 = getelementptr inbounds [100 x float], [100 x float]* @A3, i64 0, i64 %indvars.iv162
  %7 = load float, float* %arrayidx11, align 4, !tbaa !2
  %add12 = fadd float %7, %conv9
  store float %add12, float* %arrayidx11, align 4, !tbaa !2
  %conv13 = sitofp i32 %6 to double
  %arrayidx15 = getelementptr inbounds [100 x double], [100 x double]* @A4, i64 0, i64 %indvars.iv162
  %8 = load double, double* %arrayidx15, align 8, !tbaa !7
  %add16 = fadd double %8, %conv13
  store double %add16, double* %arrayidx15, align 8, !tbaa !7
  %arrayidx19 = getelementptr inbounds [100 x double], [100 x double]* @A5, i64 0, i64 %indvars.iv162
  %9 = load double, double* %arrayidx19, align 8, !tbaa !7
  %add20 = fadd double %9, %conv13
  store double %add20, double* %arrayidx19, align 8, !tbaa !7
  %arrayidx23 = getelementptr inbounds [100 x double], [100 x double]* @A6, i64 0, i64 %indvars.iv162
  %10 = load double, double* %arrayidx23, align 8, !tbaa !7
  %add24 = fadd double %10, %conv13
  store double %add24, double* %arrayidx23, align 8, !tbaa !7
  %arrayidx27 = getelementptr inbounds [100 x double], [100 x double]* @A7, i64 0, i64 %indvars.iv162
  %11 = load double, double* %arrayidx27, align 8, !tbaa !7
  %add28 = fadd double %11, %conv13
  store double %add28, double* %arrayidx27, align 8, !tbaa !7
  %arrayidx31 = getelementptr inbounds [100 x double], [100 x double]* @A8, i64 0, i64 %indvars.iv162
  %12 = load double, double* %arrayidx31, align 8, !tbaa !7
  %add32 = fadd double %12, %conv13
  store double %add32, double* %arrayidx31, align 8, !tbaa !7
  %arrayidx35 = getelementptr inbounds [100 x double], [100 x double]* @B1, i64 0, i64 %indvars.iv162
  %13 = load double, double* %arrayidx35, align 8, !tbaa !7
  %sub36 = fsub double %13, %conv13
  store double %sub36, double* %arrayidx35, align 8, !tbaa !7
  %arrayidx39 = getelementptr inbounds [100 x double], [100 x double]* @B2, i64 0, i64 %indvars.iv162
  %14 = load double, double* %arrayidx39, align 8, !tbaa !7
  %sub40 = fsub double %14, %conv13
  store double %sub40, double* %arrayidx39, align 8, !tbaa !7
  %arrayidx43 = getelementptr inbounds [100 x double], [100 x double]* @B3, i64 0, i64 %indvars.iv162
  %15 = load double, double* %arrayidx43, align 8, !tbaa !7
  %sub44 = fsub double %15, %conv13
  store double %sub44, double* %arrayidx43, align 8, !tbaa !7
  %arrayidx47 = getelementptr inbounds [100 x double], [100 x double]* @B4, i64 0, i64 %indvars.iv162
  %16 = load double, double* %arrayidx47, align 8, !tbaa !7
  %sub48 = fsub double %16, %conv13
  store double %sub48, double* %arrayidx47, align 8, !tbaa !7
  %arrayidx51 = getelementptr inbounds [100 x double], [100 x double]* @B5, i64 0, i64 %indvars.iv162
  %17 = load double, double* %arrayidx51, align 8, !tbaa !7
  %sub52 = fsub double %17, %conv13
  store double %sub52, double* %arrayidx51, align 8, !tbaa !7
  %arrayidx55 = getelementptr inbounds [100 x double], [100 x double]* @B6, i64 0, i64 %indvars.iv162
  %18 = load double, double* %arrayidx55, align 8, !tbaa !7
  %sub56 = fsub double %18, %conv13
  store double %sub56, double* %arrayidx55, align 8, !tbaa !7
  %arrayidx59 = getelementptr inbounds [100 x double], [100 x double]* @B7, i64 0, i64 %indvars.iv162
  %19 = load double, double* %arrayidx59, align 8, !tbaa !7
  %sub60 = fsub double %19, %conv13
  store double %sub60, double* %arrayidx59, align 8, !tbaa !7
  %arrayidx63 = getelementptr inbounds [100 x double], [100 x double]* @B8, i64 0, i64 %indvars.iv162
  %20 = load double, double* %arrayidx63, align 8, !tbaa !7
  %sub64 = fsub double %20, %conv13
  store double %sub64, double* %arrayidx63, align 8, !tbaa !7
  %arrayidx67 = getelementptr inbounds [100 x double], [100 x double]* @B9, i64 0, i64 %indvars.iv162
  %21 = load double, double* %arrayidx67, align 8, !tbaa !7
  %sub68 = fsub double %21, %conv13
  store double %sub68, double* %arrayidx67, align 8, !tbaa !7
  %arrayidx70 = getelementptr inbounds [100 x double], [100 x double]* @C1, i64 0, i64 %indvars.iv162
  store double 1.000000e+00, double* %arrayidx70, align 8, !tbaa !7
  %arrayidx73 = getelementptr inbounds [100 x double], [100 x double]* @C2, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx73, align 8, !tbaa !7
  %arrayidx76 = getelementptr inbounds [100 x double], [100 x double]* @C3, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx76, align 8, !tbaa !7
  %arrayidx79 = getelementptr inbounds [100 x double], [100 x double]* @C4, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx79, align 8, !tbaa !7
  %arrayidx82 = getelementptr inbounds [100 x double], [100 x double]* @C5, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx82, align 8, !tbaa !7
  %arrayidx85 = getelementptr inbounds [100 x double], [100 x double]* @C6, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx85, align 8, !tbaa !7
  %arrayidx88 = getelementptr inbounds [100 x double], [100 x double]* @C7, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx88, align 8, !tbaa !7
  %arrayidx91 = getelementptr inbounds [100 x double], [100 x double]* @C8, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx91, align 8, !tbaa !7
  %arrayidx94 = getelementptr inbounds [100 x double], [100 x double]* @C9, i64 0, i64 %indvars.iv162
  store double %conv13, double* %arrayidx94, align 8, !tbaa !7
  %indvars.iv.next163 = add nuw nsw i64 %indvars.iv162, 1
  %exitcond = icmp eq i64 %indvars.iv.next163, %wide.trip.count
  br i1 %exitcond, label %for.inc95, label %for.body3

for.inc95:                                        ; preds = %for.body3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond167 = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond167, label %for.end97.loopexit, label %for.body3.lr.ph

for.end97.loopexit:                               ; preds = %for.inc95
  br label %for.end97

for.end97:                                        ; preds = %for.end97.loopexit, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5984bb175ab218fac7daec2670f63514cacd1c4f)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA100_f", !4, i64 0}
!4 = !{!"float", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA100_d", !9, i64 0}
!9 = !{!"double", !5, i64 0}

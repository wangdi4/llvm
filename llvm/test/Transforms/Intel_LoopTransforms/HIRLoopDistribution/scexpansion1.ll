;RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -aa-pipeline="basic-aa" -hir-loop-distribute-max-mem=17  -disable-output < %s 2>&1 | FileCheck %s
;  Loop Distribution is expected to happen when there are too many
;   memory references in the loop
;  for (i = 0; i < 99; i++) {
;      A1[i] += i;
;      A2[i] += i;
;      A3[i] += i;
;      A4[i] += i;
;      A5[i] += i;
;      A6[i] += i;
;      A7[i] += i;
;      A8[i] += i;
;      B1[i] -= i;
;      B2[i] -= i;
;      B3[i] -= i;
;      B4[i] -= i;
;      ...  etc
;   ==>
;    + DO i1 = 0, 1, 1   <DO_LOOP>
;    |   %min = (-64 * i1 + 98 <= 63) ? -64 * i1 + 98 : 63;
;    |   + DO i2 = 0, %min, 1   <DO_LOOP>
;    |   |   (@C1)[0][64 * i1 + i2] = 1.000000e+00;
;    |   |   %conv9 = sitofp.i32.double(64 * i1 + i2);
;    |   |   (%.TempArray)[0][i2] = %conv9;
;    |   |   (@C9)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C8)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C7)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C6)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C5)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C4)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C3)[0][64 * i1 + i2] = %conv9;
;    |   |   (@C2)[0][64 * i1 + i2] = %conv9;
;    |   |   %sub63 = (@B9)[0][64 * i1 + i2]  -  %conv9;
;    |   |   (@B9)[0][64 * i1 + i2] = %sub63;
;    |   |   %sub59 = (@B8)[0][64 * i1 + i2]  -  %conv9;
;    |   |   (%.TempArray1)[0][i2] = %sub59;
;    |   + END LOOP
;    |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;    |   |   %conv9 = sitofp.i32.double(64 * i1 + i2);
;    |   |   %sub63 = (%.TempArray)[0][i2];
;    |   |   (@B9)[0][64 * i1 + i2] = %sub63;
;    |   |   (@C1)[0][64 * i1 + i2] = 1.000000e+00;
;
;  Note: just verify for key HIRs
; CHECK: BEGIN REGION
; CHECK:       DO i1 = 0, 1, 1
; CHECK:         DO i2 = 0, %min, 1
; CHECK:         %conv9 = sitofp.i32.double(64 * i1 + i2);
; CHECK:         END LOOP
; CHECK:         DO i2 = 0, %min, 1
; CHECK:         %conv9 = sitofp.i32.double(64 * i1 + i2)
; CHECK:         END LOOP
; CHECK:       END LOOP
;
;Module Before HIR; ModuleID = 'scexpansion1.c'
source_filename = "scexpansion1.c"
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
define void @SCExpansion1() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = trunc i64 %indvars.iv to i32
  %conv = sitofp i32 %0 to float
  %arrayidx = getelementptr inbounds [100 x float], ptr @A1, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4, !tbaa !2
  %add = fadd float %1, %conv
  store float %add, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx3 = getelementptr inbounds [100 x float], ptr @A2, i64 0, i64 %indvars.iv
  %2 = load float, ptr %arrayidx3, align 4, !tbaa !2
  %add4 = fadd float %2, %conv
  store float %add4, ptr %arrayidx3, align 4, !tbaa !2
  %arrayidx7 = getelementptr inbounds [100 x float], ptr @A3, i64 0, i64 %indvars.iv
  %3 = load float, ptr %arrayidx7, align 4, !tbaa !2
  %add8 = fadd float %3, %conv
  store float %add8, ptr %arrayidx7, align 4, !tbaa !2
  %conv9 = sitofp i32 %0 to double
  %arrayidx11 = getelementptr inbounds [100 x double], ptr @A4, i64 0, i64 %indvars.iv
  %4 = load double, ptr %arrayidx11, align 8, !tbaa !7
  %add12 = fadd double %4, %conv9
  store double %add12, ptr %arrayidx11, align 8, !tbaa !7
  %arrayidx15 = getelementptr inbounds [100 x double], ptr @A5, i64 0, i64 %indvars.iv
  %5 = load double, ptr %arrayidx15, align 8, !tbaa !7
  %add16 = fadd double %5, %conv9
  store double %add16, ptr %arrayidx15, align 8, !tbaa !7
  %arrayidx19 = getelementptr inbounds [100 x double], ptr @A6, i64 0, i64 %indvars.iv
  %6 = load double, ptr %arrayidx19, align 8, !tbaa !7
  %add20 = fadd double %6, %conv9
  store double %add20, ptr %arrayidx19, align 8, !tbaa !7
  %arrayidx23 = getelementptr inbounds [100 x double], ptr @A7, i64 0, i64 %indvars.iv
  %7 = load double, ptr %arrayidx23, align 8, !tbaa !7
  %add24 = fadd double %7, %conv9
  store double %add24, ptr %arrayidx23, align 8, !tbaa !7
  %arrayidx27 = getelementptr inbounds [100 x double], ptr @A8, i64 0, i64 %indvars.iv
  %8 = load double, ptr %arrayidx27, align 8, !tbaa !7
  %add28 = fadd double %8, %conv9
  store double %add28, ptr %arrayidx27, align 8, !tbaa !7
  %arrayidx31 = getelementptr inbounds [100 x double], ptr @B1, i64 0, i64 %indvars.iv
  %9 = load double, ptr %arrayidx31, align 8, !tbaa !7
  %sub = fsub double %9, %conv9
  store double %sub, ptr %arrayidx31, align 8, !tbaa !7
  %arrayidx34 = getelementptr inbounds [100 x double], ptr @B2, i64 0, i64 %indvars.iv
  %10 = load double, ptr %arrayidx34, align 8, !tbaa !7
  %sub35 = fsub double %10, %conv9
  store double %sub35, ptr %arrayidx34, align 8, !tbaa !7
  %arrayidx38 = getelementptr inbounds [100 x double], ptr @B3, i64 0, i64 %indvars.iv
  %11 = load double, ptr %arrayidx38, align 8, !tbaa !7
  %sub39 = fsub double %11, %conv9
  store double %sub39, ptr %arrayidx38, align 8, !tbaa !7
  %arrayidx42 = getelementptr inbounds [100 x double], ptr @B4, i64 0, i64 %indvars.iv
  %12 = load double, ptr %arrayidx42, align 8, !tbaa !7
  %sub43 = fsub double %12, %conv9
  store double %sub43, ptr %arrayidx42, align 8, !tbaa !7
  %arrayidx46 = getelementptr inbounds [100 x double], ptr @B5, i64 0, i64 %indvars.iv
  %13 = load double, ptr %arrayidx46, align 8, !tbaa !7
  %sub47 = fsub double %13, %conv9
  store double %sub47, ptr %arrayidx46, align 8, !tbaa !7
  %arrayidx50 = getelementptr inbounds [100 x double], ptr @B6, i64 0, i64 %indvars.iv
  %14 = load double, ptr %arrayidx50, align 8, !tbaa !7
  %sub51 = fsub double %14, %conv9
  store double %sub51, ptr %arrayidx50, align 8, !tbaa !7
  %arrayidx54 = getelementptr inbounds [100 x double], ptr @B7, i64 0, i64 %indvars.iv
  %15 = load double, ptr %arrayidx54, align 8, !tbaa !7
  %sub55 = fsub double %15, %conv9
  store double %sub55, ptr %arrayidx54, align 8, !tbaa !7
  %arrayidx58 = getelementptr inbounds [100 x double], ptr @B8, i64 0, i64 %indvars.iv
  %16 = load double, ptr %arrayidx58, align 8, !tbaa !7
  %sub59 = fsub double %16, %conv9
  store double %sub59, ptr %arrayidx58, align 8, !tbaa !7
  %arrayidx62 = getelementptr inbounds [100 x double], ptr @B9, i64 0, i64 %indvars.iv
  %17 = load double, ptr %arrayidx62, align 8, !tbaa !7
  %sub63 = fsub double %17, %conv9
  store double %sub63, ptr %arrayidx62, align 8, !tbaa !7
  %arrayidx65 = getelementptr inbounds [100 x double], ptr @C1, i64 0, i64 %indvars.iv
  store double 1.000000e+00, ptr %arrayidx65, align 8, !tbaa !7
  %arrayidx68 = getelementptr inbounds [100 x double], ptr @C2, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx68, align 8, !tbaa !7
  %arrayidx71 = getelementptr inbounds [100 x double], ptr @C3, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx71, align 8, !tbaa !7
  %arrayidx74 = getelementptr inbounds [100 x double], ptr @C4, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx74, align 8, !tbaa !7
  %arrayidx77 = getelementptr inbounds [100 x double], ptr @C5, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx77, align 8, !tbaa !7
  %arrayidx80 = getelementptr inbounds [100 x double], ptr @C6, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx80, align 8, !tbaa !7
  %arrayidx83 = getelementptr inbounds [100 x double], ptr @C7, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx83, align 8, !tbaa !7
  %arrayidx86 = getelementptr inbounds [100 x double], ptr @C8, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx86, align 8, !tbaa !7
  %arrayidx89 = getelementptr inbounds [100 x double], ptr @C9, i64 0, i64 %indvars.iv
  store double %conv9, ptr %arrayidx89, align 8, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 99
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
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

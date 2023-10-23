; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-loop-distribute-memrec,hir-vec-dir-insert,print<hir>" -hir-loop-distribute-skip-vectorization-profitability-check=true -disable-output < %s 2>&1 | FileCheck %s

; Check that %tmp4650 is scalar expanded and not recomputed,
; because RVal %tmp4646 of the dependent instruction %tmp4649
; does not have a use in the last loop chunk.

; Previously, loop was distributed into 3 chunks and only one small chunk was
; vectorizable. Now, loop is distributed into 4 chunks with 2 main chunks as
; vectorizable.

; HIR-
; + DO i1 = 0, 25, 1   <DO_LOOP>
; |   %tmp4442 = (undef)[0];
; |   %tmp4443 = %tmp4442  *  5.000000e-01;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   %tmp4517 = (undef)[0];
; |   %tmp4518 = %tmp4517  *  5.000000e-01;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; |   %tmp4561 = (undef)[0];
; |   %tmp4562 = %tmp4561  *  5.000000e-01;
; |   (undef)[0] = %tmp4562;
; |   %tmp4565 = (undef)[0];
; |   %tmp4566 = %tmp4565  *  5.000000e-01;
; |   (undef)[0] = undef;
; |   %tmp4587 = (undef)[0];
; |   %tmp4588 = undef  /  %tmp4587;
; |   %tmp4646 = @llvm.minnum.f64(undef,  1.500000e+01);
; |   %tmp4649 = %tmp4646  +  4.000000e+00;
; |   %tmp4650 = @zot(%tmp4649);
; |   %tmp4654 = undef  *  %tmp4650;
; |   %tmp4655 = %tmp4646  +  1.000000e+00;
; |   %tmp4656 = @zot(%tmp4655);
; |   %tmp4678 = %tmp4650  *  undef;
; |   (undef)[0] = undef;
; |   (undef)[0] = undef;
; + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK: %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 25, 1   <DO_LOOP>
; CHECK: |   %tmp4442 = (undef)[0];
; CHECK: |   %tmp4443 = %tmp4442  *  5.000000e-01;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   %tmp4517 = (undef)[0];
; CHECK: |   %tmp4518 = %tmp4517  *  5.000000e-01;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   %tmp4561 = (undef)[0];
; CHECK: |   %tmp4562 = %tmp4561  *  5.000000e-01;
; CHECK: |   (undef)[0] = %tmp4562;
; CHECK: |   %tmp4565 = (undef)[0];
; CHECK: |   %tmp4566 = %tmp4565  *  5.000000e-01;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   %tmp4587 = (undef)[0];
; CHECK: |   %tmp4588 = undef  /  %tmp4587;
; CHECK: |   %tmp4646 = @llvm.minnum.f64(undef,  1.500000e+01);
; CHECK: |   %tmp4649 = %tmp4646  +  4.000000e+00;
; CHECK: |   (%.TempArray)[0][i1] = %tmp4649;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 25, 1   <DO_LOOP>
; CHECK: |   %tmp4649 = (%.TempArray)[0][i1];
; CHECK: |   %tmp4650 = @zot(%tmp4649);
; CHECK: |   (%.TempArray20)[0][i1] = %tmp4650;
; CHECK: + END LOOP

; CHECK: %entry.region24 = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 25, 1   <DO_LOOP>
; CHECK: |   %tmp4646 = @llvm.minnum.f64(undef,  1.500000e+01);
; CHECK: |   %tmp4650 = (%.TempArray20)[0][i1];
; CHECK: |   %tmp4654 = undef  *  %tmp4650;
; CHECK: |   %tmp4655 = %tmp4646  +  1.000000e+00;
; CHECK: |   (%.TempArray22)[0][i1] = %tmp4655;
; CHECK: |   %tmp4678 = %tmp4650  *  undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: |   (undef)[0] = undef;
; CHECK: + END LOOP

; CHECK: @llvm.directive.region.exit(%entry.region24); [ DIR.VPO.END.AUTO.VEC() ]

; CHECK: + DO i1 = 0, 25, 1   <DO_LOOP>
; CHECK: |   %tmp4655 = (%.TempArray22)[0][i1];
; CHECK: |   %tmp4656 = @zot(%tmp4655);
; CHECK: + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.minnum.f64(double, double) #1

; Function Attrs: nounwind readnone
declare hidden fastcc double @zot(double) unnamed_addr #3

; Function Attrs: nounwind uwtable
define hidden fastcc void @wobble(ptr noalias nocapture readonly %arg, i32 %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3, ptr noalias nocapture %arg4, ptr noalias nocapture %arg5, ptr noalias nocapture %arg6, ptr noalias nocapture %arg7, ptr noalias nocapture %arg8, ptr noalias nocapture %arg9, ptr noalias nocapture %arg10) unnamed_addr #4 {
bb:
  br label %bb4371

bb4371:                                           ; preds = %bb4371, %bb
  %tmp4372 = phi i64 [ %tmp4458, %bb4371 ], [ 1, %bb ]
  %tmp4442 = load double, ptr undef, align 1
  %tmp4443 = fmul fast double %tmp4442, 5.000000e-01
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  %tmp4458 = add nuw nsw i64 %tmp4372, 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  %tmp4517 = load double, ptr undef, align 1
  %tmp4518 = fmul fast double %tmp4517, 5.000000e-01
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  %tmp4561 = load double, ptr undef, align 1
  %tmp4562 = fmul fast double %tmp4561, 5.000000e-01
  store double %tmp4562, ptr undef, align 1
  %tmp4565 = load double, ptr undef, align 1
  %tmp4566 = fmul fast double %tmp4565, 5.000000e-01
  store double undef, ptr undef, align 1
  %tmp4587 = load double, ptr undef, align 1
  %tmp4588 = fdiv fast double undef, %tmp4587
  %tmp4646 = tail call fast double @llvm.minnum.f64(double undef, double 1.500000e+01)
  %tmp4649 = fadd fast double %tmp4646, 4.000000e+00
  %tmp4650 = tail call fastcc double @zot(double %tmp4649)
  %tmp4654 = fmul fast double undef, %tmp4650
  %tmp4655 = fadd fast double %tmp4646, 1.000000e+00
  %tmp4656 = tail call fastcc double @zot(double %tmp4655)
  %tmp4678 = fmul fast double %tmp4650, undef
  store double undef, ptr undef, align 1
  store double undef, ptr undef, align 1
  %tmp4740 = icmp eq i64 %tmp4458, 27
  br i1 %tmp4740, label %bb4743, label %bb4371

bb4743:                                           ; preds = %bb4371
  unreachable
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { "unsafe-fp-math"="true" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }


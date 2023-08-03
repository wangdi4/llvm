; XFAIL:*
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output -hir-loop-distribute-max-mem=14 %s 2>&1 | FileCheck %s

; Check that we do not recompute for Scalar Expansion candidate due to
; it being too complex for recomputation. For CEs with multiple blobs,
; we do not allow dependent instructions. Trying to handle these cases may lead to
; verification assertions.

; CHECK:   BEGIN REGION { modified }
;                + DO i1 = 0, 62, 1   <DO_LOOP> <ivdep>
;                |   %"calc_$IND[]_fetch.76" = (null)[0];
;                |   %"calc_$T110[]_fetch.79.fr" = freeze((null)[0]);
;                |   %"calc_$RR50[]_fetch.83" = (null)[0];
;                |   %"calc_$PP62[]_fetch.85" = (null)[0];
;                |   %div.18 = (%"calc_$PP62[]_fetch.85" * %"calc_$RR50[]_fetch.83")  /  0;
;                |   %"calc_$S142[]_fetch.100" = (null)[0];
;                |   (%.TempArray)[0][i1] = %"calc_$S142[]_fetch.100";
;                |   %div.19 = (null)[0]  /  0;
;                |   %div.20 = 0  /  %div.19;
;                |   %div.21 = (null)[0]  /  %div.20 + %div.18;
;                |   (null)[0] = %div.21;
;                |   %div.22 = 0  /  %"calc_$T110[]_fetch.79.fr";
;                |   (%.TempArray9)[0][i1] = %div.22;
;                |   %"calc_$P1[]_fetch.120" = (null)[0];
;                |   (%.TempArray11)[0][i1] = %"calc_$P1[]_fetch.120";
;                |   %"calc_$XX2[]_fetch.128" = (null)[0];
;                |   (null)[0] = (%"calc_$IND[]_fetch.76" * %"calc_$XX2[]_fetch.128");
;                |   %add.22 = 0.000000e+00  +  0.000000e+00;
;                |   %"calc_$ONE38[]_fetch.140" = (null)[0];
;                |   %"(float)mul.30$" = sitofp.i32.float((%"calc_$P1[]_fetch.120" * %"calc_$P1[]_fetch.120" * %"calc_$P1[]_fetch.120" * %"calc_$ONE38[]_fetch.140"));
;                |   (%.TempArray13)[0][i1] = %"(float)mul.30$";
;                |   %"calc_$DW34[]_fetch.142" = (null)[0];
;                |   (%.TempArray15)[0][i1] = %"calc_$DW34[]_fetch.142";
;                |   %"calc_$TMP54[]_fetch.148" = (null)[0];
;                |   (%.TempArray17)[0][i1] = %"calc_$TMP54[]_fetch.148";
;                |   %div.25 = (null)[0]  /  (null)[0];
;                |   (%.TempArray19)[0][i1] = %div.25;
; CHECK:         + END LOOP
;
;
; CHECK:         + DO i1 = 0, 62, 1   <DO_LOOP> <ivdep>
;                |   %"calc_$S142[]_fetch.100" = (%.TempArray)[0][i1];
;                |   %div.22 = (%.TempArray9)[0][i1];
;                |   %"calc_$P1[]_fetch.120" = (%.TempArray11)[0][i1];
; CHECK:         |   %"(float)mul.30$" = (%.TempArray13)[0][i1];
;                |   %"calc_$DW34[]_fetch.142" = (%.TempArray15)[0][i1];
;                |   %"calc_$TMP54[]_fetch.148" = (%.TempArray17)[0][i1];
;                |   %div.25 = (%.TempArray19)[0][i1];
;                |   %"(float)calc_$C1_fetch.171$" = sitofp.i32.float((%div.22 * %"calc_$TMP54[]_fetch.148"));
;                |   %mul.39 = %"(float)mul.30$"  *  0.000000e+00;
;                |   %"(float)calc_$C3_fetch.176$" = sitofp.i32.float(%div.25);
;                |   %"(float)calc_$DT_fetch.177$" = sitofp.i32.float(%"calc_$DW34[]_fetch.142" + 3 * (%"calc_$P1[]_fetch.120" * %"calc_$P1[]_fetch.120" * %"calc_$P1[]_fetch.120"));
;                |   (null)[0] = 0;
;                |   %div.28 = %"calc_$S142[]_fetch.100"  /  0;
;                |   (null)[0] = %div.28;
;                |   (null)[0] = (null)[0];
;                + END LOOP
; CHECK:   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @calc_() #1 {
alloca_1:
  br label %bb20

bb20:                                             ; preds = %bb20, %alloca_1
  %indvars.iv300 = phi i64 [ %indvars.iv.next301, %bb20 ], [ 1, %alloca_1 ]
  %"calc_$IND[]_fetch.76" = load i32, ptr null, align 1
  %"calc_$T110[]86" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$T110[]_fetch.79" = load i32, ptr %"calc_$T110[]86", align 4
  %"calc_$T110[]_fetch.79.fr" = freeze i32 %"calc_$T110[]_fetch.79"
  %"calc_$RR50[]90" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$RR50[]_fetch.83" = load i32, ptr %"calc_$RR50[]90", align 4
  %"calc_$PP62[]92" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$PP62[]_fetch.85" = load i32, ptr %"calc_$PP62[]92", align 4
  %mul.21 = mul nsw i32 %"calc_$PP62[]_fetch.85", %"calc_$RR50[]_fetch.83"
  %"calc_$TT46[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$V114[]95" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$V114[]_fetch.88" = load i32, ptr %"calc_$V114[]95", align 4
  %div.18 = sdiv i32 %mul.21, 0
  %"calc_$S142[]_fetch.100" = load i32, ptr null, align 4
  %"calc_$WW58[]107" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$WW58[]_fetch.102" = load i32, ptr %"calc_$WW58[]107", align 4
  %div.19 = sdiv i32 %"calc_$WW58[]_fetch.102", 0
  %div.20 = sdiv i32 0, %div.19
  %add.15 = add nsw i32 %div.20, %div.18
  %div.21 = sdiv i32 %"calc_$V114[]_fetch.88", %add.15
  %"calc_$E30[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  store i32 %div.21, ptr %"calc_$E30[]", align 4
  %div.22 = sdiv i32 0, %"calc_$T110[]_fetch.79.fr"
  %"calc_$P1[]_fetch.120" = load i32, ptr null, align 1
  %mul.24 = mul nsw i32 %"calc_$P1[]_fetch.120", %"calc_$P1[]_fetch.120"
  %mul.25 = mul nsw i32 %mul.24, %"calc_$P1[]_fetch.120"
  %"calc_$XX2[]126" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$XX2[]_fetch.128" = load i32, ptr %"calc_$XX2[]126", align 4
  %mul.27 = mul nsw i32 %"calc_$XX2[]_fetch.128", %"calc_$IND[]_fetch.76"
  store i32 %mul.27, ptr %"calc_$TT46[]", align 4
  %add.22 = fadd reassoc ninf nsz arcp contract afn float 0.000000e+00, 0.000000e+00
  %"calc_$ONE38[]132" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$ONE38[]_fetch.140" = load i32, ptr %"calc_$ONE38[]132", align 4
  %mul.30 = mul nsw i32 %"calc_$ONE38[]_fetch.140", %mul.25
  %"(float)mul.30$" = sitofp i32 %mul.30 to float
  %"calc_$DW34[]134" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$DW34[]_fetch.142" = load i32, ptr %"calc_$DW34[]134", align 4
  %mul.32 = mul nsw i32 %mul.25, 3
  %add.23 = add nsw i32 %"calc_$DW34[]_fetch.142", %mul.32
  %"calc_$TMP54[]_fetch.148" = load i32, ptr null, align 4
  %mul.34 = mul nsw i32 %div.22, %"calc_$TMP54[]_fetch.148"
  %"calc_$TG222[]149" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 0, ptr nonnull elementtype(i32) null, i64 0)
  %"calc_$TG222[]_fetch.163" = load i32, ptr %"calc_$TG222[]149", align 4
  %"calc_$TG318[]_fetch.165" = load i32, ptr null, align 4
  %div.25 = sdiv i32 %"calc_$TG222[]_fetch.163", %"calc_$TG318[]_fetch.165"
  %"(float)calc_$C1_fetch.171$" = sitofp i32 %mul.34 to float
  %mul.39 = fmul reassoc ninf nsz arcp contract afn float %"(float)mul.30$", 0.000000e+00
  %"(float)calc_$C3_fetch.176$" = sitofp i32 %div.25 to float
  %"(float)calc_$DT_fetch.177$" = sitofp i32 %add.23 to float
  store i32 0, ptr null, align 4
  %div.28 = sdiv i32 %"calc_$S142[]_fetch.100", 0
  store i32 %div.28, ptr null, align 4
  %"calc_$SS[]_fetch.200" = load i32, ptr null, align 1
  store i32 %"calc_$SS[]_fetch.200", ptr null, align 1
  %indvars.iv.next301 = add nuw nsw i64 %indvars.iv300, 1
  %exitcond302.not = icmp eq i64 %indvars.iv.next301, 64
  br i1 %exitcond302.not, label %bb23, label %bb20, !llvm.loop !0

bb23:                                             ; preds = %bb20
  ret void
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "prefer-vector-width"="512" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.vectorize.ivdep_back"}

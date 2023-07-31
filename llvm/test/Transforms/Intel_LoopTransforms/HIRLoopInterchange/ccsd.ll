
; RUN: opt -passes="hir-ssa-deconstruction,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" -disable-output  < %s 2>&1 | FileCheck %s

; Verify that compiler successfully interchanges this loopnest

; HIR before Interchange
; BEGIN REGION { }
;       + DO i1 = 0, undef + -2, 1   <DO_LOOP>
;       |   + DO i2 = 0, -1 * undef + -1, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, undef + -1 * %"ccsd_idx1_wrk1_$KLO_fetch", 1   <DO_LOOP>
;       |   |   |   + DO i4 = 0, undef + -2, 1   <DO_LOOP>
;       |   |   |   |   %"ccsd_idx1_wrk1_$ERI1[][][][]_fetch" = (%"ccsd_idx1_wrk1_$ERI1")[i2 + undef][i1][i3 + %"ccsd_idx1_wrk1_$KLO_fetch"][0];
;       |   |   |   |   %"ccsd_idx1_wrk1_$SISN[][][][]_fetch" = (%"ccsd_idx1_wrk1_$SISN")[i4][i1][0][i3 + %"ccsd_idx1_wrk1_$KLO_fetch" + -1];
;       |   |   |   |   (%"ccsd_idx1_wrk1_$SISN")[i4][i1][0][i2 + undef + -1] = undef;
;       |   |   |   + END LOOP
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR After Interchange
; CHECK: BEGIN REGION { modified }
;           + DO i1 = 0, undef + -2, 1   <DO_LOOP>
;           |   + DO i2 = 0, -1 * undef + -1, 1   <DO_LOOP>
;           |   |   + DO i3 = 0, undef + -2, 1   <DO_LOOP>
;           |   |   |   + DO i4 = 0, undef + -1 * %"ccsd_idx1_wrk1_$KLO_fetch", 1   <DO_LOOP>
;           |   |   |   |   %"ccsd_idx1_wrk1_$ERI1[][][][]_fetch" = (%"ccsd_idx1_wrk1_$ERI1")[i2 + undef][i3][i4 + %"ccsd_idx1_wrk1_$KLO_fetch"][0];
;           |   |   |   |   %"ccsd_idx1_wrk1_$SISN[][][][]_fetch" = (%"ccsd_idx1_wrk1_$SISN")[i1][i3][0][i4 + %"ccsd_idx1_wrk1_$KLO_fetch" + -1];
;           |   |   |   |   (%"ccsd_idx1_wrk1_$SISN")[i1][i3][0][i2 + undef + -1] = undef;
;           |   |   |   + END LOOP
;           |   |   + END LOOP
;           |   + END LOOP
;           + END LOOP
;     END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind uwtable
define void @ccsd_idx1_wrk1_(ptr noalias nocapture dereferenceable(8) %"ccsd_idx1_wrk1_$SISN", ptr noalias nocapture readonly dereferenceable(8) %"ccsd_idx1_wrk1_$ERI1", ptr noalias nocapture readonly dereferenceable(8) %"ccsd_idx1_wrk1_$KLO") local_unnamed_addr #1 {
alloca_2:
  %"ccsd_idx1_wrk1_$KLO_fetch" = load i64, ptr %"ccsd_idx1_wrk1_$KLO", align 1
  %0 = add nsw i64 undef, 1
  br label %bb487

bb487:                                            ; preds = %alloca_2
  br i1 undef, label %bb492, label %bb491.preheader

bb491.preheader:                                  ; preds = %bb487
  br label %bb491

bb491:                                            ; preds = %bb496, %bb491.preheader
  %"ccsd_idx1_wrk1_$J.0" = phi i64 [ %add260, %bb496 ], [ 1, %bb491.preheader ]
  br label %bb495

bb495:                                            ; preds = %bb500, %bb491
  %"ccsd_idx1_wrk1_$I.0" = phi i64 [ %add250, %bb500 ], [ undef, %bb491 ]
  %"ccsd_idx1_wrk1_$ERI1[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 undef, i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$ERI1", i64 %"ccsd_idx1_wrk1_$I.0")
  %"ccsd_idx1_wrk1_$ERI1[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$ERI1[]", i64 %"ccsd_idx1_wrk1_$J.0")
  br label %bb499

bb499:                                            ; preds = %bb510, %bb495
  %"ccsd_idx1_wrk1_$K.0" = phi i64 [ %add240, %bb510 ], [ %"ccsd_idx1_wrk1_$KLO_fetch", %bb495 ]
  %"ccsd_idx1_wrk1_$ERI1[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"ccsd_idx1_wrk1_$KLO_fetch", i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$ERI1[][]", i64 %"ccsd_idx1_wrk1_$K.0")
  %"ccsd_idx1_wrk1_$ERI1[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$ERI1[][][]", i64 1)
  %"ccsd_idx1_wrk1_$ERI1[][][][]_fetch" = load double, ptr %"ccsd_idx1_wrk1_$ERI1[][][][]", align 1
  br label %bb509

bb509:                                            ; preds = %bb509, %bb499
  %"ccsd_idx1_wrk1_$II.0" = phi i64 [ %add230, %bb509 ], [ 1, %bb499 ]
  %"ccsd_idx1_wrk1_$SISN[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$SISN", i64 %"ccsd_idx1_wrk1_$II.0")
  %"ccsd_idx1_wrk1_$SISN[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$SISN[]", i64 %"ccsd_idx1_wrk1_$J.0")
  %"ccsd_idx1_wrk1_$SISN[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 undef, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$SISN[][]", i64 1)
  %"ccsd_idx1_wrk1_$SISN[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$SISN[][][]", i64 %"ccsd_idx1_wrk1_$K.0")
  %"ccsd_idx1_wrk1_$SISN[][][][]_fetch" = load double, ptr %"ccsd_idx1_wrk1_$SISN[][][][]", align 1
  %"ccsd_idx1_wrk1_$SISN[][][][]203" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"ccsd_idx1_wrk1_$SISN[][][]", i64 %"ccsd_idx1_wrk1_$I.0")
  store double undef, ptr %"ccsd_idx1_wrk1_$SISN[][][][]203", align 1
  %add230 = add nuw nsw i64 %"ccsd_idx1_wrk1_$II.0", 1
  %exitcond = icmp eq i64 %add230, undef
  br i1 %exitcond, label %bb510.loopexit, label %bb509

bb510.loopexit:                                   ; preds = %bb509
  br label %bb510

bb510:                                            ; preds = %bb510.loopexit
  %add240 = add nsw i64 %"ccsd_idx1_wrk1_$K.0", 1
  %exitcond405 = icmp eq i64 %add240, %0
  br i1 %exitcond405, label %bb500.loopexit, label %bb499

bb500.loopexit:                                   ; preds = %bb510
  br label %bb500

bb500:                                            ; preds = %bb500.loopexit
  %add250 = add nsw i64 %"ccsd_idx1_wrk1_$I.0", 1
  %exitcond406 = icmp eq i64 %add250, 0
  br i1 %exitcond406, label %bb496.loopexit, label %bb495

bb496.loopexit:                                   ; preds = %bb500
  br label %bb496

bb496:                                            ; preds = %bb496.loopexit
  %add260 = add nuw nsw i64 %"ccsd_idx1_wrk1_$J.0", 1
  %exitcond407 = icmp eq i64 %add260, undef
  br i1 %exitcond407, label %bb492.loopexit, label %bb491

bb492.loopexit:                                   ; preds = %bb496
  br label %bb492

bb492:                                            ; preds = %bb492.loopexit, %bb487
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!omp_offload.info = !{}

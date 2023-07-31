; REQUIRES: asserts
; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking-profit 2>&1 < %s | FileCheck %s

; Verify the loop is not be affected by inter loop blocking.
; The input is not a profitable candidate because of "C" array, which are defs in both spatial loops.
; Input to hir inter loop blocking
; CHECK: Function: sub1_

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, zext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:               |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   |   %add6 = (%"sub1_$B")[i2][i3]  +  1.000000e+00;
; CHECK:               |   |   |   (%"sub1_$A")[i2][i3] = %add6;
; CHECK:               |   |   |   %add24 = (%"sub1_$C")[i2][i3]  +  2.000000e+00;
; CHECK:               |   |   |   (%"sub1_$C")[i2][i3] = %add24;
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               |
; CHECK:               |
; CHECK:               |   + DO i2 = 0, 1, 1   <DO_LOOP>
; CHECK:               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   |   %"sub1_$A[]50[]_fetch" = (%"sub1_$A")[i2][i3];
; CHECK:               |   |   |   %add52 = %"sub1_$A[]50[]_fetch"  +  2.000000e+00;
; CHECK:               |   |   |   (%"sub1_$B")[i2][i3] = %add52;
; CHECK:               |   |   |   %add77 = %"sub1_$A[]50[]_fetch"  +  (%"sub1_$C")[i2][i3];
; CHECK:               |   |   |   (%"sub1_$C")[i2][i3] = %add77;
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK: Not profitable due to sustained Defs or Uses

;Module Before HIR
; ModuleID = 'neg-non-profit.f90'
source_filename = "neg-non-profit.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub1_(ptr noalias nocapture dereferenceable(8) %"sub1_$A", ptr noalias nocapture dereferenceable(8) %"sub1_$B", ptr noalias nocapture dereferenceable(8) %"sub1_$C", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$N_fetch" = load i32, ptr %"sub1_$N", align 1
  %int_sext = sext i32 %"sub1_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %rel = icmp slt i32 %"sub1_$NTIMES_fetch", 1
  br i1 %rel, label %bb4, label %bb3.preheader

bb3.preheader:                                    ; preds = %alloca_0
  %int_sext1161 = zext i32 %"sub1_$NTIMES_fetch" to i64
  %0 = add nuw nsw i64 %int_sext1161, 1
  br label %bb3

bb3:                                              ; preds = %bb3.preheader, %bb42
  %"sub1_$K.0" = phi i64 [ %add108, %bb42 ], [ 1, %bb3.preheader ]
  br label %bb7

bb7:                                              ; preds = %bb14, %bb3
  %"sub1_$I.0" = phi i64 [ 1, %bb3 ], [ %add45, %bb14 ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.0")
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.0")
  %"sub1_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$C", i64 %"sub1_$I.0")
  br label %bb11

bb11:                                             ; preds = %bb11, %bb7
  %"sub1_$J.0" = phi i64 [ 1, %bb7 ], [ %add37, %bb11 ]
  %"sub1_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]", i64 %"sub1_$J.0")
  %"sub1_$B[][]_fetch" = load double, ptr %"sub1_$B[][]", align 1
  %add6 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[][]_fetch", 1.000000e+00
  %"sub1_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]", i64 %"sub1_$J.0")
  store double %add6, ptr %"sub1_$A[][]", align 1
  %"sub1_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$C[]", i64 %"sub1_$J.0")
  %"sub1_$C[][]_fetch" = load double, ptr %"sub1_$C[][]", align 1
  %add24 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$C[][]_fetch", 2.000000e+00
  store double %add24, ptr %"sub1_$C[][]", align 1
  %add37 = add nuw nsw i64 %"sub1_$J.0", 1
  %exitcond = icmp eq i64 %add37, 4
  br i1 %exitcond, label %bb14, label %bb11

bb14:                                             ; preds = %bb11
  %add45 = add nuw nsw i64 %"sub1_$I.0", 1
  %exitcond158 = icmp eq i64 %add45, 4
  br i1 %exitcond158, label %bb39.preheader, label %bb7

bb39.preheader:                                   ; preds = %bb14
  br label %bb39

bb39:                                             ; preds = %bb39.preheader, %bb46
  %rel106 = phi i1 [ false, %bb46 ], [ true, %bb39.preheader ]
  %"sub1_$I.1" = phi i64 [ 2, %bb46 ], [ 1, %bb39.preheader ]
  %"sub1_$A[]50" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$B[]61" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.1")
  %"sub1_$C[]75" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub1_$C", i64 %"sub1_$I.1")
  br label %bb43

bb43:                                             ; preds = %bb43, %bb39
  %"sub1_$J.1" = phi i64 [ 1, %bb39 ], [ %add94, %bb43 ]
  %"sub1_$A[]50[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A[]50", i64 %"sub1_$J.1")
  %"sub1_$A[]50[]_fetch" = load double, ptr %"sub1_$A[]50[]", align 1
  %add52 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[]50[]_fetch", 2.000000e+00
  %"sub1_$B[]61[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B[]61", i64 %"sub1_$J.1")
  store double %add52, ptr %"sub1_$B[]61[]", align 1
  %"sub1_$C[]75[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$C[]75", i64 %"sub1_$J.1")
  %"sub1_$C[]75[]_fetch" = load double, ptr %"sub1_$C[]75[]", align 1
  %add77 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[]50[]_fetch", %"sub1_$C[]75[]_fetch"
  store double %add77, ptr %"sub1_$C[]75[]", align 1
  %add94 = add nuw nsw i64 %"sub1_$J.1", 1
  %exitcond159 = icmp eq i64 %add94, 4
  br i1 %exitcond159, label %bb46, label %bb43

bb46:                                             ; preds = %bb43
  br i1 %rel106, label %bb39, label %bb42

bb42:                                             ; preds = %bb46
  %add108 = add nuw nsw i64 %"sub1_$K.0", 1
  %exitcond160 = icmp eq i64 %add108, %0
  br i1 %exitcond160, label %bb4.loopexit, label %bb3

bb4.loopexit:                                     ; preds = %bb42
  br label %bb4

bb4:                                              ; preds = %bb4.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

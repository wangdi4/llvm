;
; Test to check that we don't crash when setting constant loop lower bound when
; vectorizing peel.
;
; RUN: opt -enable-new-pm=0 -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-peeling -vplan-enable-vectorized-peel %s -print-after=hir-vplan-vec 2>&1 | FileCheck %s
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec" -vplan-enable-peeling -vplan-enable-vectorized-peel %s -print-after=hir-vplan-vec 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@scratc_ = external unnamed_addr global [524288 x i8], align 32

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

define void @fbuild_(i64 %n, i32 %v) local_unnamed_addr #1 {
; CHECK:  BEGIN REGION { modified }
; Peel
; CHECK:        |   + DO i2 = 0, 6, 8   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-peel> <nounroll> <novectorize> <max_trip_count = 1>
; CHECK:        |   + END LOOP
; Main loop
; CHECK:        |   + DO i2 = 7, [[LOOP_UB0:%.*]], 8   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:        |   + END LOOP
; Remai
; CHECK:        |   + DO i2 = [[LB_TMP0:%.*]], -1 * i1 + [[N0:%.*]] + -3, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 7> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 7>
; CHECK:        |   + END LOOP
; CHECK:  END REGION
;
alloca_14:
  br label %bb373

bb373:                                            ; preds = %bb396, %alloca_14
  %indvars.iv = phi i64 [ 2, %alloca_14 ], [ %indvars.iv.next2, %bb396 ]
  br label %bb377

bb377:                                            ; preds = %bb377, %bb373
  %indvars.iv130 = phi i64 [ %indvars.iv, %bb373 ], [ %indvars.iv.next131, %bb377 ]
  %rel.183 = icmp ugt i64 %indvars.iv, 2
  %sub.70 = select i1 %rel.183, double 1.000000e+00, double 0.000000e+00
  %"val$[]46" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) bitcast ([524288 x i8]* @scratc_ to i32*), i64 %indvars.iv130)
  store i32 %v, i32* %"val$[]46", align 4
  %indvars.iv.next131 = add nuw nsw i64 %indvars.iv130, 1
  %exitcond = icmp eq i64 %indvars.iv.next131, %n
  br i1 %exitcond, label %bb395.preheader, label %bb377

bb395.preheader:                                  ; preds = %bb377
  br label %bb396

bb396:                                            ; preds = %bb395.preheader
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv, 1
  %exitcond139 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond139, label %bb374, label %bb373

bb374:                                            ; preds = %bb396
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }

!omp_offload.info = !{}

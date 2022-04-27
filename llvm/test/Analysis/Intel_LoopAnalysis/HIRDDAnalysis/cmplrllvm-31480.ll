; RUN: opt  -hir-ssa-deconstruction -hir-temp-cleanup -hir-sinking-for-perfect-loopnest -hir-loop-blocking -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that DD does not refine last dimenision of the dependence vectors
; after nodes sinking and loop blocking. The node between 9:29 should be
; (%Z)[64 * i1 + i2] --> (%Z)[64 * i1 + i2] ANTI (= = *)

; Original IR
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64((1 + %N_fetch.3)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;                  |      %"X[]_fetch.7" = (%X)[i1];
;                  |      %add.113 = (%Z)[i1];
;                  |   + DO i2 = 0, zext.i32.i64(%M_fetch.2) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>
;                  |   |   %mul.1 = %"X[]_fetch.7"  *  %add.113;
;                  |   |   %add.113 = %mul.1  +  (%C)[-1 * i2 + %M_fetch.2 + -1];
;                  |   + END LOOP
;                  |      (%Z)[i1] = %add.113;
;                  + END LOOP
;            END REGION

; *** IR Dump After HIR Sinking For Perfect Loopnest (hir-sinking-for-perfect-loopnest) ***
;            BEGIN REGION { }
;                  + DO i1 = 0, zext.i32.i64((1 + %N_fetch.3)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;                  |   + DO i2 = 0, zext.i32.i64(%M_fetch.2) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>
;                  |   |   %"X[]_fetch.7" = (%X)[i1];
;                  |   |   %add.113 = (%Z)[i1];
;                  |   |   %mul.1 = %"X[]_fetch.7"  *  %add.113;
;                  |   |   %add.113 = %mul.1  +  (%C)[-1 * i2 + %M_fetch.2 + -1];
;                  |   |   (%Z)[i1] = %add.113;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; *** IR Dump After HIR Loop Blocking (hir-loop-blocking) ***
;            BEGIN REGION { modified }
;                  + DO i1 = 0, (zext.i32.i64((1 + %N_fetch.3)) + -2)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;                  |   %min = (-64 * i1 + zext.i32.i64((1 + %N_fetch.3)) + -2 <= 63) ? -64 * i1 + zext.i32.i64((1 + %N_fetch.3)) + -2 : 63;
;                  |
;                  |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;                  |   |   + DO i3 = 0, zext.i32.i64(%M_fetch.2) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>
;                  |   |   |   %"X[]_fetch.7" = (%X)[64 * i1 + i2];
; <9>              |   |   |   %add.113 = (%Z)[64 * i1 + i2];
;                  |   |   |   %mul.1 = %"X[]_fetch.7"  *  %add.113;
;                  |   |   |   %add.113 = %mul.1  +  (%C)[-1 * i3 + %M_fetch.2 + -1];
; <29>             |   |   |   (%Z)[64 * i1 + i2] = %add.113;
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;            END REGION


; CHECK-DAG: 29:9 (%Z)[64 * i1 + i2] --> (%Z)[64 * i1 + i2] FLOW (= = *) (0 0 ?)
; CHECK-DAG: 29:29 (%Z)[64 * i1 + i2] --> (%Z)[64 * i1 + i2] OUTPUT (= = *) (0 0 ?)
; CHECK-DAG: 9:29 (%Z)[64 * i1 + i2] --> (%Z)[64 * i1 + i2] ANTI (= = *) (0 0 ?)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @foo_(i32* noalias nocapture readonly dereferenceable(4) %"N", double* noalias nocapture readonly dereferenceable(8) %"X", i32* noalias nocapture readonly dereferenceable(4) %"M", double* noalias nocapture readonly dereferenceable(8) %"C", double* noalias nocapture dereferenceable(8) %"Z") local_unnamed_addr {
alloca_0:
  %"M_fetch.2" = load i32, i32* %"M", align 1
  %"N_fetch.3" = load i32, i32* %"N", align 1
  %rel.1 = icmp slt i32 %"N_fetch.3", 1
  br i1 %rel.1, label %bb5, label %bb4.preheader

bb4.preheader:                                    ; preds = %alloca_0
  %rel.2 = icmp slt i32 %"M_fetch.2", 2
  %0 = zext i32 %"M_fetch.2" to i64
  %1 = add nuw nsw i32 %"N_fetch.3", 1
  %wide.trip.count16 = zext i32 %1 to i64
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb9
  %indvars.iv14 = phi i64 [ 1, %bb4.preheader ], [ %indvars.iv.next15, %bb9 ]
  br i1 %rel.2, label %bb9, label %bb8.preheader

bb8.preheader:                                    ; preds = %bb4
  %"X[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"X", i64 %indvars.iv14)
  %"X[]_fetch.7" = load double, double* %"X[]", align 1
  %"Z[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"Z", i64 %indvars.iv14)
  %"Z[].promoted" = load double, double* %"Z[]", align 1
  br label %bb8

bb8:                                              ; preds = %bb8.preheader, %bb8
  %indvars.iv = phi i64 [ %0, %bb8.preheader ], [ %indvars.iv.next, %bb8 ]
  %add.113 = phi double [ %"Z[].promoted", %bb8.preheader ], [ %add.1, %bb8 ]
  %"J.0.in" = phi i32 [ %"M_fetch.2", %bb8.preheader ], [ %"J.0", %bb8 ]
  %"J.0" = add nsw i32 %"J.0.in", -1
  %mul.1 = fmul reassoc ninf nsz arcp contract afn double %"X[]_fetch.7", %add.113
  %int_sext212 = zext i32 %"J.0" to i64
  %"C[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"C", i64 %int_sext212)
  %"C[]_fetch.11" = load double, double* %"C[]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn double %mul.1, %"C[]_fetch.11"
  %rel.3 = icmp sgt i64 %indvars.iv, 2
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  br i1 %rel.3, label %bb8, label %bb9.loopexit

bb9.loopexit:                                     ; preds = %bb8
  %add.1.lcssa = phi double [ %add.1, %bb8 ]
  store double %add.1.lcssa, double* %"Z[]", align 1
  br label %bb9

bb9:                                              ; preds = %bb9.loopexit, %bb4
  %indvars.iv.next15 = add nuw nsw i64 %indvars.iv14, 1
  %exitcond = icmp eq i64 %indvars.iv.next15, %wide.trip.count16
  br i1 %exitcond, label %bb5.loopexit, label %bb4

bb5.loopexit:                                     ; preds = %bb9
  br label %bb5

bb5:                                              ; preds = %bb5.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64)


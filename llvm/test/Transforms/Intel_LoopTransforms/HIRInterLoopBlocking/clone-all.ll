; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s | FileCheck %s
; RUN: opt -disable-hir-inter-loop-blocking=false -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s | FileCheck %s --check-prefix=NOLIBIRC


; Verify that non-load instructions can be cloned before by-strip loops as needed

; Before

;         BEGIN REGION { }
;               + DO i1 = 0, %"sub1_$NTIMES_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
;               |   |   |   %add21 = (%"sub1_$B.addr_a0$_fetch")[i2 + 1][i3 + 1]  +  1.000000e+00;
;               |   |   |   (%"sub1_$A.addr_a0$_fetch")[i2 + 1][i3 + 1] = %add21;
;               |   |   + END LOOP
;               |   + END LOOP
;               |
;               |
;               |   + DO i2 = 0, 2, 1   <DO_LOOP>
;               |   |   %globalvar_mod_mp_zstop__fetch = (@globalvar_mod_mp_zstop_)[0];
;               |   |   %myrel = %globalvar_mod_mp_zstop__fetch  |  %NNN;
;               |   |   if (%globalvar_mod_mp_zstop__fetch >= (@globalvar_mod_mp_zstart_)[0])
;               |   |   {
;               |   |      + DO i3 = 0, sext.i32.i64(%globalvar_mod_mp_zstop__fetch) + -1 * sext.i32.i64(%myrel), 1   <DO_LOOP>
;               |   |      |   %add111 = (%"sub1_$A.addr_a0$_fetch")[i2 + 1][i3 + sext.i32.i64(%myrel)]  +  2.000000e+00;
;               |   |      |   (%"sub1_$B.addr_a0$_fetch")[i2 + 1][i3 + sext.i32.i64(%myrel)] = %add111;
;               |   |      + END LOOP
;               |   |   }
;               |   + END LOOP
;               + END LOOP
;        END REGION

; After

;         BEGIN REGION { modified }
;               + DO i1 = 0, %"sub1_$NTIMES_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;               |   %clone6 = (@globalvar_mod_mp_zstop_)[0];
;               |   %clone = %globalvar_mod_mp_zstop__fetch  |  %NNN;    // This cloned instruction is NOT a load instruction.
;               |
;               |   + DO i2 = 1, 3, 2   <DO_LOOP>
;               |   |   %tile_e_min = (i2 + 1 <= 3) ? i2 + 1 : 3;
;               |   |
;               |   |   + DO i3 = smin(1, sext.i32.i64(%clone)), smax(3, sext.i32.i64(%clone6)), 2   <DO_LOOP>
;               |   |   |   %tile_e_min7 = (i3 + 1 <= smax(3, sext.i32.i64(%clone6))) ? i3 + 1 : smax(3, sext.i32.i64(%clone6));
;               |   |   |   %lb_max8 = (1 <= i2) ? i2 : 1;
;               |   |   |   %ub_min9 = (3 <= %tile_e_min) ? 3 : %tile_e_min;
;               |   |   |
;               |   |   |   + DO i4 = 0, -1 * %lb_max8 + %ub_min9, 1   <DO_LOOP>
;               |   |   |   |   %lb_max = (1 <= i3) ? i3 : 1;
;               |   |   |   |   %ub_min = (3 <= %tile_e_min7) ? 3 : %tile_e_min7;
;               |   |   |   |
;               |   |   |   |   + DO i5 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
;               |   |   |   |   |   %add21 = (%"sub1_$B.addr_a0$_fetch")[i4 + %lb_max8][i5 + %lb_max]  +  1.000000e+00;
;               |   |   |   |   |   (%"sub1_$A.addr_a0$_fetch")[i4 + %lb_max8][i5 + %lb_max] = %add21;
;               |   |   |   |   + END LOOP
;               |   |   |   + END LOOP
;               |   |   |
;               |   |   |   %lb_max12 = (1 <= i2) ? i2 : 1;
;               |   |   |   %ub_min13 = (3 <= %tile_e_min) ? 3 : %tile_e_min;
;               |   |   |
;               |   |   |   + DO i4 = 0, -1 * %lb_max12 + %ub_min13, 1   <DO_LOOP>
;               |   |   |   |   %globalvar_mod_mp_zstop__fetch = (@globalvar_mod_mp_zstop_)[0];
;               |   |   |   |   %myrel = %globalvar_mod_mp_zstop__fetch  |  %NNN;
;               |   |   |   |   if (%globalvar_mod_mp_zstop__fetch >= (@globalvar_mod_mp_zstart_)[0])
;               |   |   |   |   {
;               |   |   |   |      %lb_max10 = (sext.i32.i64(%myrel) <= i3) ? i3 : sext.i32.i64(%myrel);
;               |   |   |   |      %ub_min11 = (sext.i32.i64(%globalvar_mod_mp_zstop__fetch) <= %tile_e_min7) ? sext.i32.i64(%globalvar_mod_mp_zstop__fetch) : %tile_e_min7;
;               |   |   |   |
;               |   |   |   |      + DO i5 = 0, -1 * %lb_max10 + %ub_min11, 1   <DO_LOOP>
;               |   |   |   |      |   %add111 = (%"sub1_$A.addr_a0$_fetch")[i4 + %lb_max12][i5 + %lb_max10]  +  2.000000e+00;
;               |   |   |   |      |   (%"sub1_$B.addr_a0$_fetch")[i4 + %lb_max12][i5 + %lb_max10] = %add111;
;               |   |   |   |      + END LOOP
;               |   |   |   |   }
;               |   |   |   + END LOOP
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;         END REGION

; CHECK: Function: sub1_

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, %"sub1_$NTIMES_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:               |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   + DO i3 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   |   %add21 = (%"sub1_$B.addr_a0$_fetch")[i2 + 1][i3 + 1]  +  1.000000e+00;
; CHECK:               |   |   |   (%"sub1_$A.addr_a0$_fetch")[i2 + 1][i3 + 1] = %add21;
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               |
; CHECK:               |
; CHECK:               |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   |   %globalvar_mod_mp_zstop__fetch = (@globalvar_mod_mp_zstop_)[0];
; CHECK:               |   |   %myrel = %globalvar_mod_mp_zstop__fetch |  %NNN;
; CHECK:               |   |   if (%globalvar_mod_mp_zstop__fetch >= (@globalvar_mod_mp_zstart_)[0])
; CHECK:               |   |   {
; CHECK:               |   |      + DO i3 = 0, sext.i32.i64(%globalvar_mod_mp_zstop__fetch) + -1 * sext.i32.i64(%myrel), 1   <DO_LOOP>
; CHECK:               |   |      |   %add111 = (%"sub1_$A.addr_a0$_fetch")[i2 + 1][i3 + sext.i32.i64(%myrel)]  +  2.000000e+00;
; CHECK:               |   |      |   (%"sub1_$B.addr_a0$_fetch")[i2 + 1][i3 + sext.i32.i64(%myrel)] = %add111;
; CHECK:               |   |      + END LOOP
; CHECK:               |   |   }
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK: Function: sub1_

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, %"sub1_$NTIMES_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |   [[CLONE_2:%clone[0-9]*]] = (@globalvar_mod_mp_zstop_)[0];
; CHECK:           |   [[CLONE_1:%clone[0-9]*]] = [[CLONE_2]] |  %NNN;
; CHECK:           |
; CHECK:           |   + DO i2 = 1, 3, 2   <DO_LOOP>
; CHECK:           |   |   [[TILE_1:%tile_e_min[0-9]*]] = (i2 + 1 <= 3) ? i2 + 1 : 3;
; CHECK:           |   |
; CHECK:           |   |   + DO i3 = smin(1, sext.i32.i64([[CLONE_1]])), smax(3, sext.i32.i64([[CLONE_2]])), 2   <DO_LOOP>
; CHECK:           |   |   |   [[TILE_2:%tile_e_min[0-9]+]] = (i3 + 1 <= smax(3, sext.i32.i64([[CLONE_2]]))) ? i3 + 1 : smax(3, sext.i32.i64([[CLONE_2]]));
; CHECK:           |   |   |   [[LB_1:%lb_max[0-9]*]] = (1 <= i2) ? i2 : 1;
; CHECK:           |   |   |   [[UB_1:%ub_min[0-9]*]] = (3 <= [[TILE_1]]) ? 3 : [[TILE_1]];
; CHECK:           |   |   |
; CHECK:           |   |   |   + DO i4 = 0, -1 * [[LB_1]] + [[UB_1]], 1   <DO_LOOP>
; CHECK:           |   |   |   |   [[LB_2:%lb_max[0-9]*]] = (1 <= i3) ? i3 : 1;
; CHECK:           |   |   |   |   [[UB_2:%ub_min[0-9]*]] = (3 <= [[TILE_2]]) ? 3 : [[TILE_2]];
; CHECK:           |   |   |   |
; CHECK:           |   |   |   |   + DO i5 = 0, -1 * [[LB_2]] + [[UB_2]], 1   <DO_LOOP>
; CHECK:           |   |   |   |   |   %add21 = (%"sub1_$B.addr_a0$_fetch")[i4 + [[LB_1]]][i5 + [[LB_2]]]  +  1.000000e+00;
; CHECK:           |   |   |   |   |   (%"sub1_$A.addr_a0$_fetch")[i4 + [[LB_1]]][i5 + [[LB_2]]] = %add21;
; CHECK:           |   |   |   |   + END LOOP
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   |
; CHECK:           |   |   |   [[LB_3:%lb_max[0-9]*]] = (1 <= i2) ? i2 : 1;
; CHECK:           |   |   |   [[UB_3:%ub_min[0-9]*]] = (3 <= [[TILE_1]]) ? 3 : [[TILE_1]];
; CHECK:           |   |   |
; CHECK:           |   |   |   + DO i4 = 0, -1 * [[LB_3]] + [[UB_3]], 1   <DO_LOOP>
; CHECK:           |   |   |   |   %globalvar_mod_mp_zstop__fetch = (@globalvar_mod_mp_zstop_)[0];
; CHECK:           |   |   |   |   %myrel = %globalvar_mod_mp_zstop__fetch  |  %NNN;
; CHECK:           |   |   |   |   if (%globalvar_mod_mp_zstop__fetch >= (@globalvar_mod_mp_zstart_)[0])
; CHECK:           |   |   |   |   {
; CHECK:           |   |   |   |      [[LB_4:%lb_max[0-9]*]] = (sext.i32.i64(%myrel) <= i3) ? i3 : sext.i32.i64(%myrel);
; CHECK:           |   |   |   |      [[UB_4:%ub_min[0-9]*]] = (sext.i32.i64(%globalvar_mod_mp_zstop__fetch) <= [[TILE_2]]) ? sext.i32.i64(%globalvar_mod_mp_zstop__fetch) : [[TILE_2]];
; CHECK:           |   |   |   |      + DO i5 = 0, -1 * [[LB_4]] + [[UB_4]], 1   <DO_LOOP>
; CHECK:           |   |   |   |      |   %add111 = (%"sub1_$A.addr_a0$_fetch")[i4 + [[LB_3]]][i5 + [[LB_4]]]  +  2.000000e+00;
; CHECK:           |   |   |   |      |   (%"sub1_$B.addr_a0$_fetch")[i4 + [[LB_3]]][i5 + [[LB_4]]] = %add111;
; CHECK:           |   |   |   |      + END LOOP
; CHECK:           |   |   |   |   }
; CHECK:           |   |   |   + END LOOP
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

; Verify that transformation is not triggered without libIRC.

; NOLIBIRC-NOT: modified

;Module Before HIR
; ModuleID = 'blob-index-inner.f90'
source_filename = "blob-index-inner.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@globalvar_mod_mp_zstop_ = external local_unnamed_addr global i32, align 8
@globalvar_mod_mp_zstart_ = external local_unnamed_addr global i32, align 8

; Function Attrs: nofree nounwind uwtable
define void @sub1_(ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"sub1_$A", ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"sub1_$B", ptr noalias nocapture readnone dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES", ptr noalias nocapture readnone dereferenceable(4) %"sub1_$BLOB", i32 %NNN) local_unnamed_addr #0 {
alloca_0:
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %rel = icmp slt i32 %"sub1_$NTIMES_fetch", 1
  br i1 %rel, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %"sub1_$B.addr_a0$" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$B", i64 0, i32 0
  %"sub1_$B.addr_a0$_fetch" = load ptr, ptr %"sub1_$B.addr_a0$", align 1
  %"sub1_$B.dim_info$.lower_bound$55" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$B", i64 0, i32 6, i64 0, i32 2
  %"sub1_$B.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$B.dim_info$.lower_bound$55", i32 0)
  %"sub1_$B.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"sub1_$B.dim_info$.lower_bound$[]", align 1
  %"sub1_$B.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$B", i64 0, i32 6, i64 0, i32 1
  %"sub1_$B.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$B.dim_info$.spacing$", i32 1)
  %"sub1_$B.dim_info$.spacing$[]_fetch" = load i64, ptr %"sub1_$B.dim_info$.spacing$[]", align 1
  %"sub1_$B.dim_info$.lower_bound$[]3" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$B.dim_info$.lower_bound$55", i32 1)
  %"sub1_$B.dim_info$.lower_bound$[]3_fetch" = load i64, ptr %"sub1_$B.dim_info$.lower_bound$[]3", align 1
  %"sub1_$A.addr_a0$" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$A", i64 0, i32 0
  %"sub1_$A.addr_a0$_fetch" = load ptr, ptr %"sub1_$A.addr_a0$", align 1
  %"sub1_$A.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$A", i64 0, i32 6, i64 0, i32 2
  %"sub1_$A.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$A.dim_info$.lower_bound$", i32 0)
  %"sub1_$A.dim_info$.lower_bound$[]_fetch" = load i64, ptr %"sub1_$A.dim_info$.lower_bound$[]", align 1
  %"sub1_$A.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$ptr$rank2$", ptr %"sub1_$A", i64 0, i32 6, i64 0, i32 1
  %"sub1_$A.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$A.dim_info$.spacing$", i32 1)
  %"sub1_$A.dim_info$.spacing$[]_fetch" = load i64, ptr %"sub1_$A.dim_info$.spacing$[]", align 1
  %"sub1_$A.dim_info$.lower_bound$31[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) nonnull %"sub1_$A.dim_info$.lower_bound$", i32 1)
  %"sub1_$A.dim_info$.lower_bound$31[]_fetch" = load i64, ptr %"sub1_$A.dim_info$.lower_bound$31[]", align 1
  %0 = add nuw nsw i32 %"sub1_$NTIMES_fetch", 1
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb50
  %"sub1_$K.0" = phi i32 [ %add175, %bb50 ], [ 1, %bb2.preheader ]
  br label %bb6

bb6:                                              ; preds = %bb13, %bb2
  %indvars.iv199 = phi i64 [ %indvars.iv.next200, %bb13 ], [ 1, %bb2 ]
  %"sub1_$B.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"sub1_$B.dim_info$.lower_bound$[]3_fetch", i64 %"sub1_$B.dim_info$.spacing$[]_fetch", ptr elementtype(double) %"sub1_$B.addr_a0$_fetch", i64 %indvars.iv199)
  %"sub1_$A.addr_a0$_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"sub1_$A.dim_info$.lower_bound$31[]_fetch", i64 %"sub1_$A.dim_info$.spacing$[]_fetch", ptr elementtype(double) %"sub1_$A.addr_a0$_fetch", i64 %indvars.iv199)
  br label %bb10

bb10:                                             ; preds = %bb10, %bb6
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb10 ], [ 1, %bb6 ]
  %"sub1_$B.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"sub1_$B.dim_info$.lower_bound$[]_fetch", i64 8, ptr elementtype(double) %"sub1_$B.addr_a0$_fetch[]", i64 %indvars.iv)
  %"sub1_$B.addr_a0$_fetch[][]_fetch" = load double, ptr %"sub1_$B.addr_a0$_fetch[][]", align 1
  %add21 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B.addr_a0$_fetch[][]_fetch", 1.000000e+00
  %"sub1_$A.addr_a0$_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"sub1_$A.dim_info$.lower_bound$[]_fetch", i64 8, ptr elementtype(double) %"sub1_$A.addr_a0$_fetch[]", i64 %indvars.iv)
  store double %add21, ptr %"sub1_$A.addr_a0$_fetch[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond, label %bb13, label %bb10

bb13:                                             ; preds = %bb10
  %indvars.iv.next200 = add nuw nsw i64 %indvars.iv199, 1
  %exitcond201 = icmp eq i64 %indvars.iv.next200, 4
  br i1 %exitcond201, label %bb47.preheader, label %bb6

bb47.preheader:                                   ; preds = %bb13
  br label %bb47

bb47:                                             ; preds = %bb47.preheader, %bb52
  %indvars.iv205 = phi i64 [ %indvars.iv.next206, %bb52 ], [ 1, %bb47.preheader ]
  %globalvar_mod_mp_zstart__fetch = load i32, ptr @globalvar_mod_mp_zstart_, align 8
  %globalvar_mod_mp_zstop__fetch = load i32, ptr @globalvar_mod_mp_zstop_, align 8
  %myrel = or i32 %globalvar_mod_mp_zstop__fetch, %NNN
  %rel73 = icmp slt i32 %globalvar_mod_mp_zstop__fetch, %globalvar_mod_mp_zstart__fetch
  br i1 %rel73, label %bb52, label %bb51.preheader

bb51.preheader:                                   ; preds = %bb47
  %"sub1_$A.addr_a0$_fetch[]109" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"sub1_$A.dim_info$.lower_bound$31[]_fetch", i64 %"sub1_$A.dim_info$.spacing$[]_fetch", ptr elementtype(double) %"sub1_$A.addr_a0$_fetch", i64 %indvars.iv205)
  %"sub1_$B.addr_a0$113_fetch[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"sub1_$B.dim_info$.lower_bound$[]3_fetch", i64 %"sub1_$B.dim_info$.spacing$[]_fetch", ptr elementtype(double) %"sub1_$B.addr_a0$_fetch", i64 %indvars.iv205)
  %1 = sext i32 %myrel to i64
  %2 = add nsw i32 %globalvar_mod_mp_zstop__fetch, 1
  %wide.trip.count = sext i32 %2 to i64
  br label %bb51

bb51:                                             ; preds = %bb51.preheader, %bb51
  %indvars.iv202 = phi i64 [ %1, %bb51.preheader ], [ %indvars.iv.next203, %bb51 ]
  %"sub1_$A.addr_a0$_fetch[]109[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"sub1_$A.dim_info$.lower_bound$[]_fetch", i64 8, ptr elementtype(double) %"sub1_$A.addr_a0$_fetch[]109", i64 %indvars.iv202)
  %"sub1_$A.addr_a0$_fetch[]109[]_fetch" = load double, ptr %"sub1_$A.addr_a0$_fetch[]109[]", align 1
  %add111 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A.addr_a0$_fetch[]109[]_fetch", 2.000000e+00
  %"sub1_$B.addr_a0$113_fetch[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"sub1_$B.dim_info$.lower_bound$[]_fetch", i64 8, ptr elementtype(double) %"sub1_$B.addr_a0$113_fetch[]", i64 %indvars.iv202)
  store double %add111, ptr %"sub1_$B.addr_a0$113_fetch[][]", align 1
  %indvars.iv.next203 = add nsw i64 %indvars.iv202, 1
  %exitcond204 = icmp eq i64 %indvars.iv.next203, %wide.trip.count
  br i1 %exitcond204, label %bb52.loopexit, label %bb51

bb52.loopexit:                                    ; preds = %bb51
  br label %bb52

bb52:                                             ; preds = %bb52.loopexit, %bb47
  %indvars.iv.next206 = add nuw nsw i64 %indvars.iv205, 1
  %exitcond207 = icmp eq i64 %indvars.iv.next206, 4
  br i1 %exitcond207, label %bb50, label %bb47

bb50:                                             ; preds = %bb52
  %add175 = add nuw nsw i32 %"sub1_$K.0", 1
  %exitcond208 = icmp eq i32 %add175, %0
  br i1 %exitcond208, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb50
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

; RUN: opt -hir-inter-loop-blocking-force-test -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-inter-loop-blocking-stripmine-size=2 2>&1 < %s | FileCheck %s

; Verify that the second sibling loop (i2-level) was delayed(shifted) and that
; the upper bound of the new stripmine loop is increased by the same amount + 1.
; Here, the shift amount is two: a def at A[i2] in the first sibling loop, and a use at
; A[i2 + 2] at the second sibling loop.


; CHECK:Function: sub1_

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, sext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"sub1_$N_fetch2") + -1, 1   <DO_LOOP>
; CHECK:              |   |   %add = (%"sub1_$B")[i2]  +  1.000000e+00;
; CHECK:              |   |   (%"sub1_$A")[i2] = %add;
; CHECK:              |   + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |   + DO i2 = 0, %3 + -503, 1   <DO_LOOP>  <MAX_TC_EST = 2147483145>
; CHECK:              |   |   %add26 = (%"sub1_$A")[i2 + 2]  +  2.000000e+00;
; CHECK:              |   |   (%"sub1_$B")[i2] = %add26;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION


; CHECK:Function: sub1_

; CHECK:        BEGIN REGION { modified }
; CHECK:              + DO i1 = 0, sext.i32.i64(%"sub1_$NTIMES_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, (3 + smax((-503 + %3), (-1 + sext.i32.i64(%"sub1_$N_fetch2")))), 2   <DO_LOOP>
; CHECK:              |   |   %tile_e_min = (i2 + 1 <= (3 + smax((-503 + %3), (-1 + sext.i32.i64(%"sub1_$N_fetch2"))))) ? i2 + 1 : (3 + smax((-503 + %3), (-1 + sext.i32.i64(%"sub1_$N_fetch2"))));
; CHECK:              |   |   %lb_max = (0 <= i2) ? i2 : 0;
; CHECK:              |   |   %ub_min = (sext.i32.i64(%"sub1_$N_fetch2") + -1 <= %tile_e_min) ? sext.i32.i64(%"sub1_$N_fetch2") + -1 : %tile_e_min;
; CHECK:              |   |
; CHECK:              |   |   + DO i3 = 0, -1 * %lb_max + %ub_min, 1   <DO_LOOP>
; CHECK:              |   |   |   %add = (%"sub1_$B")[i3 + %lb_max]  +  1.000000e+00;
; CHECK:              |   |   |   (%"sub1_$A")[i3 + %lb_max] = %add;
; CHECK:              |   |   + END LOOP
; CHECK:              |   |
; CHECK:              |   |   %lb_max4 = (0 <= i2 + -2) ? i2 + -2 : 0;
; CHECK:              |   |   %sub = %tile_e_min  -  2;
; CHECK:              |   |   %ub_min5 = (%3 + -503 <= %sub) ? %3 + -503 : %sub;
; CHECK:              |   |
; CHECK:              |   |   + DO i3 = 0, -1 * %lb_max4 + %ub_min5, 1   <DO_LOOP>  <MAX_TC_EST = 2147483145>
; CHECK:              |   |   |   %add26 = (%"sub1_$A")[i3 + %lb_max4 + 2]  +  2.000000e+00;
; CHECK:              |   |   |   (%"sub1_$B")[i3 + %lb_max4] = %add26;
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION


;Module Before HIR
; ModuleID = 'shift-loop.f90'
source_filename = "shift-loop.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub1_(ptr noalias nocapture dereferenceable(8) %"sub1_$A", ptr noalias nocapture dereferenceable(8) %"sub1_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$N", ptr noalias nocapture readonly dereferenceable(4) %"sub1_$NTIMES") local_unnamed_addr #0 {
alloca_0:
  %"sub1_$NTIMES_fetch" = load i32, ptr %"sub1_$NTIMES", align 1
  %int_sext = sext i32 %"sub1_$NTIMES_fetch" to i64
  %rel = icmp slt i32 %"sub1_$NTIMES_fetch", 1
  br i1 %rel, label %bb4, label %bb3.preheader

bb3.preheader:                                    ; preds = %alloca_0
  %"sub1_$N_fetch2" = load i32, ptr %"sub1_$N", align 1
  %int_sext4 = sext i32 %"sub1_$N_fetch2" to i64
  %rel6 = icmp slt i32 %"sub1_$N_fetch2", 1
  br i1 %rel6, label %bb4, label %bb3.preheader.split

bb3.preheader.split:                              ; preds = %bb3.preheader
  %rel23 = icmp slt i32 %"sub1_$N_fetch2", 503
  br i1 %rel23, label %bb3.us66.preheader, label %bb3.preheader93

bb3.preheader93:                                  ; preds = %bb3.preheader.split
  %0 = add nsw i64 %int_sext4, 1
  %1 = zext i32 %"sub1_$N_fetch2" to i64
  %2 = icmp ugt i64 %1, 503
  %3 = select i1 %2, i64 %1, i64 503
  %smax = add nsw i64 %3, -499
  %4 = add nsw i64 %int_sext, 1
  br label %bb3

bb3.us66.preheader:                               ; preds = %bb3.preheader.split
  %5 = add nsw i64 %int_sext4, 1
  %6 = add nsw i64 %int_sext, 1
  br label %bb3.us66

bb3.us66:                                         ; preds = %bb3.us66.preheader, %bb13.us86
  %"sub1_$J.0.us67" = phi i64 [ %add44.us87, %bb13.us86 ], [ 1, %bb3.us66.preheader ]
  br label %bb7.us68

bb7.us68:                                         ; preds = %bb3.us66, %bb7.us68
  %"sub1_$I.0.us69" = phi i64 [ %add11.us74, %bb7.us68 ], [ 1, %bb3.us66 ]
  %"sub1_$B[].us70" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.0.us69")
  %"sub1_$B[]_fetch.us71" = load double, ptr %"sub1_$B[].us70", align 1
  %add.us72 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[]_fetch.us71", 1.000000e+00
  %"sub1_$A[].us73" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.0.us69")
  store double %add.us72, ptr %"sub1_$A[].us73", align 1
  %add11.us74 = add nuw nsw i64 %"sub1_$I.0.us69", 1
  %exitcond = icmp eq i64 %add11.us74, %5
  br i1 %exitcond, label %bb13.us86, label %bb7.us68

bb13.us86:                                        ; preds = %bb7.us68
  %add44.us87 = add nuw nsw i64 %"sub1_$J.0.us67", 1
  %exitcond95 = icmp eq i64 %add44.us87, %6
  br i1 %exitcond95, label %bb4.loopexit, label %bb3.us66

bb3:                                              ; preds = %bb3.preheader93, %bb13
  %"sub1_$J.0" = phi i64 [ %add44, %bb13 ], [ 1, %bb3.preheader93 ]
  br label %bb7

bb7:                                              ; preds = %bb3, %bb7
  %"sub1_$I.0" = phi i64 [ %add11, %bb7 ], [ 1, %bb3 ]
  %"sub1_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B", i64 %"sub1_$I.0")
  %"sub1_$B[]_fetch" = load double, ptr %"sub1_$B[]", align 1
  %add = fadd reassoc ninf nsz arcp contract afn double %"sub1_$B[]_fetch", 1.000000e+00
  %"sub1_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.0")
  store double %add, ptr %"sub1_$A[]", align 1
  %add11 = add nuw nsw i64 %"sub1_$I.0", 1
  %exitcond96 = icmp eq i64 %add11, %0
  br i1 %exitcond96, label %bb12.preheader, label %bb7

bb12.preheader:                                   ; preds = %bb7
  br label %bb12

bb12:                                             ; preds = %bb12.preheader, %bb12
  %"sub1_$I.1" = phi i64 [ %add36, %bb12 ], [ 3, %bb12.preheader ]
  %"sub1_$A[]24" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$A", i64 %"sub1_$I.1")
  %"sub1_$A[]24_fetch" = load double, ptr %"sub1_$A[]24", align 1
  %add26 = fadd reassoc ninf nsz arcp contract afn double %"sub1_$A[]24_fetch", 2.000000e+00
  %sub30 = add nsw i64 %"sub1_$I.1", -2
  %"sub1_$B[]31" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub1_$B", i64 %sub30)
  store double %add26, ptr %"sub1_$B[]31", align 1
  %add36 = add nuw nsw i64 %"sub1_$I.1", 1
  %exitcond97 = icmp eq i64 %add36, %smax
  br i1 %exitcond97, label %bb13, label %bb12

bb13:                                             ; preds = %bb12
  %add44 = add nuw nsw i64 %"sub1_$J.0", 1
  %exitcond98 = icmp eq i64 %add44, %4
  br i1 %exitcond98, label %bb4.loopexit100, label %bb3

bb4.loopexit:                                     ; preds = %bb13.us86
  br label %bb4

bb4.loopexit100:                                  ; preds = %bb13
  br label %bb4

bb4:                                              ; preds = %bb4.loopexit100, %bb4.loopexit, %bb3.preheader, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

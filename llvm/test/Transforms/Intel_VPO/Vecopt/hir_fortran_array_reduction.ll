; Check HIR vectorizer support for handling Fortran array accesses participating in reductions.

; Incoming HIR for Case 1
;    %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;    + DO i2 = 0, zext.i32.i64((1 + %"interp_$N_fetch4")) + -1, 1   <DO_LOOP>
;    |   %red.phi = %red.phi  +  (%"interp_$ARR")[i2][i1]; <Safe Reduction>
;    + END LOOP
;
;    @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]

; Incoming HIR for Case 2
;    %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
;
;    + DO i2 = 0, zext.i32.i64((1 + %"interp_$N_fetch4")) + -1, 1   <DO_LOOP>
;    |   %2 = (%"interp_$ARR")[i2][i1];
;    |   %red.phi = %red.phi  +  %2; <Safe Reduction>
;    + END LOOP
;
;    @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]


; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=2 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s


; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=2 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s

; Check that loop was vectorized.
; CHECK:      + DO i2 = 0, [[UB:.*]], 2 <DO_LOOP> <MAX_TC_EST = 1073741824>   <LEGAL_MAX_TC = 1073741824> <auto-vectorized> <nounroll> <novectorize>
; CHECK:      |   [[LD:%.*]] = (<2 x double>*)(%"interp_$ARR")[i2 + <i64 0, i64 1>][i1];
; CHECK:      |   [[VEC2:%.*]] = [[RED_PHI:%.*]]  +  [[LD]];
; CHECK:      |   [[RED_PHI]]  =  [[VEC2]];
; CHECK:      + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind
define void @interp_(double* noalias nocapture %"interp_$ARR", i32* noalias nocapture readonly %"interp_$M", i32* noalias nocapture readonly %"interp_$N") local_unnamed_addr {
alloca:
  %"interp_$M_fetch" = load i32, i32* %"interp_$M", align 4
  %int_sext = sext i32 %"interp_$M_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %rel = icmp slt i32 %"interp_$M_fetch", 0
  br i1 %rel, label %bb3, label %bb7.preheader

bb7.preheader:                                    ; preds = %alloca
  %"interp_$N_fetch4" = load i32, i32* %"interp_$N", align 4
  %rel6 = icmp slt i32 %"interp_$N_fetch4", 0
  %0 = add i32 %"interp_$N_fetch4", 1
  %1 = add nuw i32 %"interp_$M_fetch", 1
  %wide.trip.count41 = zext i32 %1 to i64
  %wide.trip.count = zext i32 %0 to i64
  br label %bb7

bb7:                                              ; preds = %bb1, %bb7.preheader
  %indvars.iv39 = phi i64 [ 0, %bb7.preheader ], [ %indvars.iv.next40, %bb1 ]
  br i1 %rel6, label %bb1, label %bb11.preheader

bb11.preheader:                                   ; preds = %bb7
  br label %bb11

bb11:                                             ; preds = %bb11.preheader, %bb11
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb11 ], [ 0, %bb11.preheader ]
  %red.phi = phi double [ 0.000000e+00, %bb11.preheader ], [ %red.add, %bb11 ]
  %"interp_$ARR[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 0, i64 %mul, double* elementtype(double) %"interp_$ARR", i64 %indvars.iv)
  %"interp_$ARR[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 0, i64 8, double* elementtype(double) %"interp_$ARR[]", i64 %indvars.iv39)
  %2 = load double, double* %"interp_$ARR[][]", align 8
  %red.add = fadd fast double %red.phi, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb1.loopexit, label %bb11

bb1.loopexit:                                     ; preds = %bb11
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %bb7
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond42 = icmp eq i64 %indvars.iv.next40, %wide.trip.count41
  br i1 %exitcond42, label %bb3.loopexit, label %bb7

bb3.loopexit:                                     ; preds = %bb1
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca
  ret void
}
; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1


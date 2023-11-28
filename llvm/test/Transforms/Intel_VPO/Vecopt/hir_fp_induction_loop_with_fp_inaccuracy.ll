; Test to verify that ParVecAnalysis bails out from identifying FP IV
; for a loop that is known to cause FP computation inaccuracies upon
; vectorization.

; Incoming HIR -
; BEGIN REGION { }
;       + DO i1 = 0, zext.i32.i64(%NOFREQ) + -1, 1   <DO_LOOP>
;       |   (%FREQUENCY)[%IPOWER + -1][i1] = %add.28;
;       |   %add.28 = %FREQSTEP  +  %add.28;
;       + END LOOP
; END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>' -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-cg,simplifycfg,intel-ir-optreport-emitter -disable-output -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTREPORT

; CHECK-LABEL: BEGIN REGION { }
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%NOFREQ) + -1, 1   <DO_LOOP>
; CHECK-NEXT:        |   (%FREQUENCY)[%IPOWER + -1][i1] = %add.28;
; CHECK-NEXT:        |   %add.28 = %FREQSTEP  +  %add.28;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:  END REGION

; OPTREPORT: remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; OPTREPORT: remark #15346: vector dependence: assumed FLOW dependence
; OPTREPORT: remark #15346: vector dependence: assumed FLOW dependence

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define double @myinit_mod_mp_freq_init_(i64 %IPOWER, i32 %NOFREQ, i64 %MAXNOFREQ, ptr noalias nocapture writeonly dereferenceable(8) %FREQUENCY, double %FREQ, double %FREQSTEP) local_unnamed_addr {
bb2.preheader:
  %mul.1 = shl nsw i64 %MAXNOFREQ, 3
  %"FREQUENCY[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(double) %FREQUENCY, i64 %IPOWER)
  %0 = add nuw nsw i32 %NOFREQ, 1
  %wide.trip.count = zext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb2
  %indvars.iv = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next, %bb2 ]
  %add.28 = phi double [ %FREQ, %bb2.preheader ], [ %add.2, %bb2 ]
  %"FREQUENCY[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"FREQUENCY[]", i64 %indvars.iv)
  store double %add.28, ptr %"FREQUENCY[][]", align 1
  %add.2 = fadd reassoc ninf nsz arcp contract afn double %FREQSTEP, %add.28
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb2
  %add.2.lcssa = phi double [ %add.2, %bb2 ]
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit
  ret double %add.2.lcssa
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

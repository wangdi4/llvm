; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that DD analysis recognizes zext blob as non-negative during DD.

; CHECK-NOT: (%A)[i1] --> (%A)[i1 + zext.i32.i64(%div.1)] FLOW (*)

;<0>          BEGIN REGION { }
;<14>               + DO i1 = 0, sext.i32.i64(%div.1) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 1073741823>
;<5>                |   %add.2 = (%"A")[i1 + zext.i32.i64(%div.1)]  +  %"K_fetch.7";
;<7>                |   (%"A")[i1] = %add.2;
;<14>               + END LOOP
;<0>          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind uwtable
define void @s174_(i32* noalias nocapture readonly dereferenceable(4) %"N", float* noalias nocapture dereferenceable(4) %"A", float* noalias nocapture readnone dereferenceable(4) %"B", float* noalias nocapture readonly
dereferenceable(4) %"K") local_unnamed_addr {
entry:
  %"N_fetch.2" = load i32, i32* %"N", align 1
  %div.1 = sdiv i32 %"N_fetch.2", 2
  %rel.1 = icmp slt i32 %div.1, 1
  br i1 %rel.1, label %exit, label %preheader

preheader:                                    ; preds = %entry
  %"K_fetch.7" = load float, float* %"K", align 1
  %0 = zext i32 %div.1 to i64
  %1 = add nuw nsw i32 %div.1, 1
  %wide.trip.count8 = zext i32 %1 to i64
  br label %loop

loop:                                              ; preds = %preheader, %loop
  %indvars.iv = phi i64 [ 1, %preheader ], [ %indvars.iv.next, %loop ]
  %2 = add nuw nsw i64 %indvars.iv, %0
  %"A[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"A", i64 %2)
  %"A[]_fetch.6" = load float, float* %"A[]", align 1
  %add.2 = fadd reassoc ninf nsz arcp contract afn float %"A[]_fetch.6", %"K_fetch.7"
  %"A[]2" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) nonnull %"A", i64 %indvars.iv)
  store float %add.2, float* %"A[]2", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count8
  br i1 %exitcond, label %loopexit, label %loop

loopexit:                                     ; preds = %loop
  br label %exit

exit:                                              ; preds = %loopexit, %entry
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

!omp_offload.info = !{}


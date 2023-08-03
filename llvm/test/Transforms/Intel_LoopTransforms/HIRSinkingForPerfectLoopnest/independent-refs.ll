; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; Check that sinking does not occur for near perfect loopnest where there is no dependence
; between a ref inside the innerloop vs outside refs. In this case we would prefer loop
; distribution to create independent loops.


; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:        |   |   %add.1 = (%"sub_$C1")[i1][i2]  +  1.000000e+00;
; CHECK:        |   |   (%"sub_$A1")[i1][i2] = %add.1;
; CHECK:        |   |   %add.2 = (%"sub_$C2")[i1][i2]  +  1.000000e+00;
; CHECK:        |   |   (%"sub_$A2")[i1][i2] = %add.2;
; CHECK:        |   + END LOOP
; CHECK:        |   (%"sub_$B1")[i1] = 1.000000e+00;
; CHECK:        |   (%"sub_$B2")[i1] = 2.000000e+00;
; CHECK:        + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture writeonly dereferenceable(8) %"sub_$A1", ptr noalias nocapture writeonly dereferenceable(8) %"sub_$A2", ptr noalias nocapture writeonly dereferenceable(8) %"sub_$B1", ptr noalias nocapture writeonly dereferenceable(8) %"sub_$B2", ptr noalias nocapture readonly dereferenceable(8) %"sub_$C1", ptr noalias nocapture readonly dereferenceable(8) %"sub_$C2", ptr noalias nocapture readnone dereferenceable(4) %"sub_$N2") local_unnamed_addr #0 {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog9, %alloca_0
  %indvars.iv22 = phi i64 [ %indvars.iv.next23, %do.epilog9 ], [ 1, %alloca_0 ]
  %"sub_$C1[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr nonnull elementtype(double) %"sub_$C1", i64 %indvars.iv22)
  %"sub_$A1[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr nonnull elementtype(double) %"sub_$A1", i64 %indvars.iv22)
  %"sub_$C2[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr nonnull elementtype(double) %"sub_$C2", i64 %indvars.iv22)
  %"sub_$A2[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 800, ptr nonnull elementtype(double) %"sub_$A2", i64 %indvars.iv22)
  %"sub_$B1[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$B1", i64 %indvars.iv22)
  %"sub_$B2[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$B2", i64 %indvars.iv22)
  br label %do.body6

do.body6:                                         ; preds = %do.body6, %do.body2
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body6 ], [ 1, %do.body2 ]
  %"sub_$C1[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$C1[]", i64 %indvars.iv)
  %"sub_$C1[][]_fetch.3" = load double, ptr %"sub_$C1[][]", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn double %"sub_$C1[][]_fetch.3", 1.000000e+00
  %"sub_$A1[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$A1[]", i64 %indvars.iv)
  store double %add.1, ptr %"sub_$A1[][]", align 1
  %"sub_$C2[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$C2[]", i64 %indvars.iv)
  %"sub_$C2[][]_fetch.8" = load double, ptr %"sub_$C2[][]", align 1
  %add.2 = fadd reassoc ninf nsz arcp contract afn double %"sub_$C2[][]_fetch.8", 1.000000e+00
  %"sub_$A2[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$A2[]", i64 %indvars.iv)
  store double %add.2, ptr %"sub_$A2[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 101
  br i1 %exitcond.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.body6
  %"sub_$B1[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$B1[]", i64 1)
  %"sub_$B2[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$B2[]", i64 1)
  store double 1.000000e+00, ptr %"sub_$B1[][]", align 1
  store double 2.000000e+00, ptr %"sub_$B2[][]", align 1
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24.not = icmp eq i64 %indvars.iv.next23, 101
  br i1 %exitcond24.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %do.epilog9
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

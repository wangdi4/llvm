; RUN: opt < %s -hir-details-dims -hir-ssa-deconstruction -hir-framework -analyze | FileCheck %s
; RUN: opt < %s -hir-details-dims -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Check that load of (%"sub_$W")[0:i1:8(%complex_64bit*:0)].1 has LB of 0 as incoming LB was merged into the index.

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:       |   %.unpack1415 = (i32*)(%"sub_$W")[0:i1:8(%complex_64bit*:0)].1;
; CHECK:       |   (i32*)(%"sub_$A")[0:i1:4(float*:0)] = %.unpack1415;
; CHECK:       + END LOOP
; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_64bit = type { float, float }

define void @sub_(%complex_64bit* noalias nocapture readonly %"sub_$W", float* noalias nocapture %"sub_$A") {
alloca:
  br label %bb4

bb4:                                              ; preds = %bb4, %alloca
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb4 ], [ 1, %alloca ]
  %0 = tail call %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8 0, i64 1, i64 8, %complex_64bit* %"sub_$W", i64 %indvars.iv)
  %.elt13 = getelementptr inbounds %complex_64bit, %complex_64bit* %0, i64 0, i32 1
  %1 = bitcast float* %.elt13 to i32*
  %.unpack1415 = load i32, i32* %1, align 4
  %2 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* %"sub_$A", i64 %indvars.iv)
  %3 = bitcast float* %2 to i32*
  store i32 %.unpack1415, i32* %3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %bb2, label %bb4

bb2:                                              ; preds = %bb4
  ret void
}

declare %complex_64bit* @llvm.intel.subscript.p0s_complex_64bits.i64.i64.p0s_complex_64bits.i64(i8, i64, i64, %complex_64bit*, i64) #1
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #0 = { nounwind readnone speculatable }

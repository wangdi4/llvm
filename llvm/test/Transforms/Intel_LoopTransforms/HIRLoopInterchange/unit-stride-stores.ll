; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-interchange" -aa-pipeline="basic-aa" -debug-only=hir-loop-interchange,hir-locality-analysis -disable-output < %s 2>&1 | FileCheck %s

; TODO: Interchanged: ( 1 2 3 ) --> ( 2 3 1 )

; Verify that unit stride stores/lvals are prioritized for the interchange cost model.
; The key intuition here is that Loops for lvl 1 and 3 both share the same cache locality
; for one ref, with each being the innermost dimension. Previously, the cost model
; prioritized i3 since its in dim 2 ( (%"sub2_$A")[i2][i3][i1]) as opposed to dim 3 for
; i1 ((%"sub2_$B")[i1][i2][i3]). However, since the stride is large, the updated costmodel
; assigns equal numcachelines and the determining factor becomes the lval stride value.

; Details from HIRLocalityAnalysis showing the cost breakdown:
;Group 0 contains:
;        (%"sub2_$B")[i1][i2][i3] {sb:22}
;Group 1 contains:
;        (%"sub2_$A")[i2][i3][i1] {sb:23}
;(%"sub2_$B")[i1][i2][i3] Adding Stride of 80000
;Added NumCacheLines = 125000, ExtraCacheLines = 0
;(%"sub2_$A")[i2][i3][i1] Adding Stride of 8
;Added NumCacheLines = 13, ExtraCacheLines = 0
; CHECK: Locality Info for Loop level: 1     NumCacheLines: 125013   SpatialCacheLines: 125013 TempInvCacheLines: 0    AvgLvalStride: 8         AvgStride: 40004
;(%"sub2_$B")[i1][i2][i3] Adding Stride of 800
;Added NumCacheLines = 1250, ExtraCacheLines = 0
;(%"sub2_$A")[i2][i3][i1] Adding Stride of 80000
;Added NumCacheLines = 125000, ExtraCacheLines = 0
; CHECK: Locality Info for Loop level: 2     NumCacheLines: 126250   SpatialCacheLines: 126250 TempInvCacheLines: 0    AvgLvalStride: 80000     AvgStride: 40400
;(%"sub2_$B")[i1][i2][i3] Adding Stride of 8
;Added NumCacheLines = 13, ExtraCacheLines = 0
;(%"sub2_$A")[i2][i3][i1] Adding Stride of 800
;Added NumCacheLines = 1250, ExtraCacheLines = 0
; CHECK: Locality Info for Loop level: 3     NumCacheLines: 1263     SpatialCacheLines: 1263  TempInvCacheLines: 0     AvgLvalStride: 800       AvgStride: 404

; CandidateLoop:
;      + DO i1 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   + DO i2 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   |   + DO i3 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   |   |   %"sub2_$B[][][]_fetch" = (%"sub2_$B")[i1][i2][i3];
;      |   |   |   (%"sub2_$A")[i2][i3][i1] = %"sub2_$B[][][]_fetch";
;      |   |   + END LOOP
;      |   + END LOOP
;      + END LOOP


;  Resulting loop
;      + DO i1 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   + DO i2 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   |   + DO i3 = 0, sext.i32.i64(%"sub2_$N_fetch") + -1, 1   <DO_LOOP>
;      |   |   |   %"sub2_$B[][][]_fetch" = (%"sub2_$B")[i3][i1][i2];
;      |   |   |   (%"sub2_$A")[i1][i2][i3] = %"sub2_$B[][][]_fetch";
;      |   |   + END LOOP
;      |   + END LOOP
;      + END LOOP

;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub2_(ptr noalias nocapture dereferenceable(8) %"sub2_$A", ptr noalias nocapture readonly dereferenceable(8) %"sub2_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub2_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub2_$N_fetch" = load i32, ptr %"sub2_$N", align 1
  %int_sext = sext i32 %"sub2_$N_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %mul13 = mul nsw i64 %mul, %int_sext
  %rel = icmp slt i32 %"sub2_$N_fetch", 1
  br i1 %rel, label %bb4, label %bb3.preheader

bb3.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub2_$N_fetch", 1
  %wide.trip.count105 = sext i32 %0 to i64
  br label %bb11.preheader.preheader

bb11.preheader.preheader:                         ; preds = %bb8, %bb3.preheader
  %indvars.iv103 = phi i64 [ 1, %bb3.preheader ], [ %indvars.iv.next104, %bb8 ]
  %"sub2_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(double) nonnull %"sub2_$B", i64 %indvars.iv103)
  br label %bb11.preheader

bb11.preheader:                                   ; preds = %bb11.preheader.preheader, %bb12
  %indvars.iv99 = phi i64 [ 1, %bb11.preheader.preheader ], [ %indvars.iv.next100, %bb12 ]
  %"sub2_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub2_$B[]", i64 %indvars.iv99)
  %"sub2_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul13, ptr elementtype(double) nonnull %"sub2_$A", i64 %indvars.iv99)
  br label %bb11

bb11:                                             ; preds = %bb11.preheader, %bb11
  %indvars.iv = phi i64 [ 1, %bb11.preheader ], [ %indvars.iv.next, %bb11 ]
  %"sub2_$B[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub2_$B[][]", i64 %indvars.iv)
  %"sub2_$B[][][]_fetch" = load double, ptr %"sub2_$B[][][]", align 1
  %"sub2_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) nonnull %"sub2_$A[]", i64 %indvars.iv)
  %"sub2_$A[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %"sub2_$A[][]", i64 %indvars.iv103)
  store double %"sub2_$B[][][]_fetch", ptr %"sub2_$A[][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count105
  br i1 %exitcond, label %bb12, label %bb11

bb12:                                             ; preds = %bb11
  %indvars.iv.next100 = add nuw nsw i64 %indvars.iv99, 1
  %exitcond102 = icmp eq i64 %indvars.iv.next100, %wide.trip.count105
  br i1 %exitcond102, label %bb8, label %bb11.preheader

bb8:                                              ; preds = %bb12
  %indvars.iv.next104 = add nuw nsw i64 %indvars.iv103, 1
  %exitcond106 = icmp eq i64 %indvars.iv.next104, %wide.trip.count105
  br i1 %exitcond106, label %bb4.loopexit, label %bb11.preheader.preheader

bb4.loopexit:                                     ; preds = %bb8
  br label %bb4

bb4:                                              ; preds = %bb4.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

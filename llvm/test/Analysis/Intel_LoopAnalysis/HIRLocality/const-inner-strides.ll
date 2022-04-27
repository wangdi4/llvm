; RUN: opt %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-locality-analysis>" -hir-spatial-locality -disable-output 2>&1 | FileCheck %s

; Check that the inner stride for i2 is 80 which is computed from the const stride value
; of 10 multiplied by the type size of 8.

; subroutine sub2  (A,B, N)
;   real*8 A(10,n,n), b(10,n,n)
;
;   do k=1,n
;      do i=1,n
;         do j=1,n
;            b(j,i,k) = 1.0
;         enddo
;      enddo
;   enddo
;   end


; CHECK: Locality Info for Loop level: 1     NumCacheLines: 12500    SpatialCacheLines: 12500 TempInvCacheLines: 0     AvgLvalStride: 8000      AvgStride: 8000
; CHECK: Locality Info for Loop level: 2     NumCacheLines: 125      SpatialCacheLines: 125   TempInvCacheLines: 0     AvgLvalStride: 80        AvgStride: 80
; CHECK: Locality Info for Loop level: 3     NumCacheLines: 2        SpatialCacheLines: 2     TempInvCacheLines: 0     AvgLvalStride: 8         AvgStride: 8

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @sub2_(double* noalias nocapture readnone dereferenceable(8) %"sub2_$A", double* noalias nocapture dereferenceable(8) %"sub2_$B", i32* noalias nocapture readonly dereferenceable(4) %"sub2_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub2_$N_fetch" = load i32, i32* %"sub2_$N", align 1
  %int_sext = sext i32 %"sub2_$N_fetch" to i64
  %mul = mul nsw i64 %int_sext, 80
  %rel = icmp slt i32 %"sub2_$N_fetch", 1
  br i1 %rel, label %bb4, label %bb3.preheader

bb3.preheader:                                    ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub2_$N_fetch", 1
  %wide.trip.count65 = sext i32 %0 to i64
  br label %bb11.preheader.preheader

bb11.preheader.preheader:                         ; preds = %bb8, %bb3.preheader
  %indvars.iv63 = phi i64 [ 1, %bb3.preheader ], [ %indvars.iv.next64, %bb8 ]
  %"sub2_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul, double* elementtype(double) nonnull %"sub2_$B", i64 %indvars.iv63)
  br label %bb11.preheader

bb11.preheader:                                   ; preds = %bb11.preheader.preheader, %bb12
  %indvars.iv59 = phi i64 [ 1, %bb11.preheader.preheader ], [ %indvars.iv.next60, %bb12 ]
  %"sub2_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 80, double* elementtype(double) nonnull %"sub2_$B[]", i64 %indvars.iv59)
  br label %bb11

bb11:                                             ; preds = %bb11.preheader, %bb11
  %indvars.iv = phi i64 [ 1, %bb11.preheader ], [ %indvars.iv.next, %bb11 ]
  %"sub2_$B[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub2_$B[][]", i64 %indvars.iv)
  store double 1.000000e+00, double* %"sub2_$B[][][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count65
  br i1 %exitcond, label %bb12, label %bb11

bb12:                                             ; preds = %bb11
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond62 = icmp eq i64 %indvars.iv.next60, %wide.trip.count65
  br i1 %exitcond62, label %bb8, label %bb11.preheader

bb8:                                              ; preds = %bb12
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond66 = icmp eq i64 %indvars.iv.next64, %wide.trip.count65
  br i1 %exitcond66, label %bb4.loopexit, label %bb11.preheader.preheader

bb4.loopexit:                                     ; preds = %bb8
  br label %bb4

bb4:                                              ; preds = %bb4.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

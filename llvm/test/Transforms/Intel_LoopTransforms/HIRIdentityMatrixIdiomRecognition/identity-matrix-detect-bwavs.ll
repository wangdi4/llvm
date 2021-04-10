; REQUIRES: asserts

; RUN: opt -debug -hir-ssa-deconstruction -hir-identity-matrix-idiom-recognition -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -S %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-identity-matrix-idiom-recognition,print<hir>" -enable-alt-identity-matrix-detection -debug-only=hir-hlnode-utils -aa-pipeline=basic-aa -S %s 2>&1 | FileCheck %s

; Check that identity matrix detection works for 503.bwaves loop structure

;  BEGIN REGION { }
;        + DO i1 = 0, sext.i32.i64(%"shell_$NZL_fetch") + -1, 1   <DO_LOOP>
;        |   + DO i2 = 0, sext.i32.i64(%"shell_$NY_fetch") + -1, 1   <DO_LOOP>
;        |   |   + DO i3 = 0, sext.i32.i64(%"shell_$NX_fetch") + -1, 1   <DO_LOOP>
;        |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;        |   |   |   |   (%"shell_$RHS6")[i1][i2][i3][i4] = 0.000000e+00;
;        |   |   |   |
;        |   |   |   |   + DO i5 = 0, 4, 1   <DO_LOOP>
;        |   |   |   |   |   (%"shell_$A")[i1][i2][i3][i4][i5] = 0.000000e+00;
;        |   |   |   |   |   (%"shell_$IDENT20")[i1][i2][i3][i4][i5] = 0.000000e+00;
;        |   |   |   |   + END LOOP
;        |   |   |   |
;        |   |   |   |   (%"shell_$IDENT20")[i1][i2][i3][i4][i4] = 1.000000e+00;
;        |   |   |   + END LOOP
;        |   |   + END LOOP
;        |   + END LOOP
;        + END LOOP
;  END REGION

;Found Diag Inst in OuterLp: <47>         (%"shell_$IDENT20")[i1][i2][i3][i4][i4] = 1.000000e+00;

;Found Zero Instruction: <38>         (%"shell_$IDENT20")[i1][i2][i3][i4][i5] = 0.000000e+00;

;Found Zero Instruction: <36>         (%"shell_$A")[i1][i2][i3][i4][i5] = 0.000000e+00;

; CHECK: Found Ident Matrix, DiagInst:
; CHECK-SAME: (%"shell_$IDENT20")[i1][i2][i3][i4][i4] = 1.000000e+00;

; CHECK: Found Identity Matrix for Loop



target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @shell_(i32* noalias nocapture readonly dereferenceable(4) %"shell_$NX", i32* noalias nocapture readonly dereferenceable(4) %"shell_$NY", i32* noalias nocapture readnone dereferenceable(4) %"shell_$NZ", i32* noalias nocapture readonly dereferenceable(4) %"shell_$NZL", double* noalias nocapture dereferenceable(8) %"shell_$A", double* noalias nocapture readonly dereferenceable(8) %"shell_$B") local_unnamed_addr #0 {
alloca_0:
  %"shell_$NX_fetch" = load i32, i32* %"shell_$NX", align 1
  %"shell_$NY_fetch" = load i32, i32* %"shell_$NY", align 1
  %"shell_$NZL_fetch" = load i32, i32* %"shell_$NZL", align 1
  %int_sext = sext i32 %"shell_$NX_fetch" to i64
  %rel.1 = icmp sgt i64 %int_sext, 0
  %slct.1 = select i1 %rel.1, i64 %int_sext, i64 0
  %int_sext2 = sext i32 %"shell_$NY_fetch" to i64
  %rel.2 = icmp sgt i64 %int_sext2, 0
  %slct.2 = select i1 %rel.2, i64 %int_sext2, i64 0
  %int_sext4 = sext i32 %"shell_$NZL_fetch" to i64
  %rel.3 = icmp sgt i64 %int_sext4, 0
  %slct.3 = select i1 %rel.3, i64 %int_sext4, i64 0
  %mul.2 = mul nuw nsw i64 %slct.2, %slct.1
  %mul.3 = mul i64 %mul.2, 40
  %mul.4 = mul i64 %mul.3, %slct.3
  %div.1 = lshr exact i64 %mul.4, 3
  %"shell_$RHS6" = alloca double, i64 %div.1, align 1
  %mul.8 = mul i64 %mul.2, 200
  %mul.9 = mul i64 %mul.8, %slct.3
  %div.2 = lshr exact i64 %mul.9, 3
  %"shell_$IDENT20" = alloca double, i64 %div.2, align 1
  %mul.12 = mul nsw i64 %int_sext, 40
  %mul.13 = mul nsw i64 %mul.12, %int_sext2
  %mul.23 = mul nsw i64 %int_sext, 200
  %mul.24 = mul nsw i64 %mul.23, %int_sext2
  %rel.7 = icmp slt i32 %"shell_$NZL_fetch", 1
  br i1 %rel.7, label %bb35, label %bb4.preheader

bb4.preheader:                                    ; preds = %alloca_0
  %rel.8 = icmp slt i32 %"shell_$NY_fetch", 1
  %rel.9 = icmp slt i32 %"shell_$NX_fetch", 1
  %0 = add nuw nsw i32 %"shell_$NX_fetch", 1
  %1 = add nuw nsw i32 %"shell_$NY_fetch", 1
  %2 = add nuw nsw i32 %"shell_$NZL_fetch", 1
  %wide.trip.count325 = sext i32 %2 to i64
  %wide.trip.count321 = sext i32 %1 to i64
  %wide.trip.count317 = sext i32 %0 to i64
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb9
  %indvars.iv323 = phi i64 [ 1, %bb4.preheader ], [ %indvars.iv.next324, %bb9 ]
  br i1 %rel.8, label %bb9, label %bb8.preheader

bb8.preheader:                                    ; preds = %bb4
  %"shell_$IDENT20[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 4, i64 1, i64 %mul.24, double* nonnull %"shell_$IDENT20", i64 %indvars.iv323)
  %"shell_$RHS6[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.13, double* nonnull %"shell_$RHS6", i64 %indvars.iv323)
  %"shell_$A[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 4, i64 1, i64 %mul.24, double* nonnull %"shell_$A", i64 %indvars.iv323)
  br label %bb8

bb8:                                              ; preds = %bb8.preheader, %bb13
  %indvars.iv319 = phi i64 [ 1, %bb8.preheader ], [ %indvars.iv.next320, %bb13 ]
  br i1 %rel.9, label %bb13, label %bb12.preheader

bb12.preheader:                                   ; preds = %bb8
  %"shell_$IDENT20[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.23, double* nonnull %"shell_$IDENT20[]", i64 %indvars.iv319)
  %"shell_$RHS6[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.12, double* nonnull %"shell_$RHS6[]", i64 %indvars.iv319)
  %"shell_$A[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.23, double* nonnull %"shell_$A[]", i64 %indvars.iv319)
  br label %bb12

bb12:                                             ; preds = %bb12.preheader, %bb19
  %indvars.iv315 = phi i64 [ 1, %bb12.preheader ], [ %indvars.iv.next316, %bb19 ]
  %"shell_$IDENT20[][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 200, double* nonnull %"shell_$IDENT20[][]", i64 %indvars.iv315)
  %"shell_$RHS6[][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$RHS6[][]", i64 %indvars.iv315)
  %"shell_$A[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 200, double* nonnull %"shell_$A[][]", i64 %indvars.iv315)
  br label %bb16

bb16:                                             ; preds = %bb26, %bb12
  %indvars.iv312 = phi i64 [ %indvars.iv.next313, %bb26 ], [ 1, %bb12 ]
  %"shell_$RHS6[][][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$RHS6[][][]", i64 %indvars.iv312)
  store double 0.000000e+00, double* %"shell_$RHS6[][][][]", align 1
  %"shell_$A[][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$A[][][]", i64 %indvars.iv312)
  %"shell_$IDENT20[][][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$IDENT20[][][]", i64 %indvars.iv312)
  br label %bb23

bb23:                                             ; preds = %bb23, %bb16
  %indvars.iv309 = phi i64 [ %indvars.iv.next310, %bb23 ], [ 1, %bb16 ]
  %"shell_$A[][][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$A[][][][]", i64 %indvars.iv309)
  store double 0.000000e+00, double* %"shell_$A[][][][][]", align 1
  %"shell_$IDENT20[][][][][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$IDENT20[][][][]", i64 %indvars.iv309)
  store double 0.000000e+00, double* %"shell_$IDENT20[][][][][]", align 1
  %indvars.iv.next310 = add nuw nsw i64 %indvars.iv309, 1
  %exitcond311.not = icmp eq i64 %indvars.iv.next310, 6
  br i1 %exitcond311.not, label %bb26, label %bb23

bb26:                                             ; preds = %bb23
  %"shell_$IDENT20[][][][][]95" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$IDENT20[][][][]", i64 %indvars.iv312)
  store double 1.000000e+00, double* %"shell_$IDENT20[][][][][]95", align 1
  %indvars.iv.next313 = add nuw nsw i64 %indvars.iv312, 1
  %exitcond314.not = icmp eq i64 %indvars.iv.next313, 6
  br i1 %exitcond314.not, label %bb19, label %bb16

bb19:                                             ; preds = %bb26
  %indvars.iv.next316 = add nuw nsw i64 %indvars.iv315, 1
  %exitcond318 = icmp eq i64 %indvars.iv.next316, %wide.trip.count317
  br i1 %exitcond318, label %bb13.loopexit, label %bb12

bb13.loopexit:                                    ; preds = %bb19
  br label %bb13

bb13:                                             ; preds = %bb13.loopexit, %bb8
  %indvars.iv.next320 = add nuw nsw i64 %indvars.iv319, 1
  %exitcond322 = icmp eq i64 %indvars.iv.next320, %wide.trip.count321
  br i1 %exitcond322, label %bb9.loopexit, label %bb8

bb9.loopexit:                                     ; preds = %bb13
  br label %bb9

bb9:                                              ; preds = %bb9.loopexit, %bb4
  %indvars.iv.next324 = add nuw nsw i64 %indvars.iv323, 1
  %exitcond326 = icmp eq i64 %indvars.iv.next324, %wide.trip.count325
  br i1 %exitcond326, label %bb34.preheader, label %bb4

bb34.preheader:                                   ; preds = %bb9
  br label %bb34

bb34:                                             ; preds = %bb34.preheader, %bb39
  %indvars.iv305 = phi i64 [ 1, %bb34.preheader ], [ %indvars.iv.next306, %bb39 ]
  br i1 %rel.8, label %bb39, label %bb38.preheader

bb38.preheader:                                   ; preds = %bb34
  %"shell_$IDENT20[]137" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 4, i64 1, i64 %mul.24, double* nonnull %"shell_$IDENT20", i64 %indvars.iv305)
  %"shell_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 4, i64 1, i64 %mul.24, double* nonnull %"shell_$B", i64 %indvars.iv305)
  %"shell_$A[]175" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 4, i64 1, i64 %mul.24, double* nonnull %"shell_$A", i64 %indvars.iv305)
  br label %bb38

bb38:                                             ; preds = %bb38.preheader, %bb43
  %indvars.iv301 = phi i64 [ 1, %bb38.preheader ], [ %indvars.iv.next302, %bb43 ]
  br i1 %rel.9, label %bb43, label %bb42.preheader

bb42.preheader:                                   ; preds = %bb38
  %"shell_$IDENT20[][]138" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.23, double* nonnull %"shell_$IDENT20[]137", i64 %indvars.iv301)
  %"shell_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.23, double* nonnull %"shell_$B[]", i64 %indvars.iv301)
  %"shell_$A[][]176" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul.23, double* nonnull %"shell_$A[]175", i64 %indvars.iv301)
  br label %bb42

bb42:                                             ; preds = %bb42.preheader, %bb49
  %indvars.iv298 = phi i64 [ 1, %bb42.preheader ], [ %indvars.iv.next299, %bb49 ]
  %"shell_$IDENT20[][][]139" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 200, double* nonnull %"shell_$IDENT20[][]138", i64 %indvars.iv298)
  %"shell_$B[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 200, double* nonnull %"shell_$B[][]", i64 %indvars.iv298)
  %"shell_$A[][][]177" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 200, double* nonnull %"shell_$A[][]176", i64 %indvars.iv298)
  br label %bb46

bb46:                                             ; preds = %bb53, %bb42
  %indvars.iv295 = phi i64 [ %indvars.iv.next296, %bb53 ], [ 1, %bb42 ]
  %"shell_$IDENT20[][][][]140" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$IDENT20[][][]139", i64 %indvars.iv295)
  %"shell_$B[][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$B[][][]", i64 %indvars.iv295)
  %"shell_$A[][][][]178" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull %"shell_$A[][][]177", i64 %indvars.iv295)
  br label %bb50

bb50:                                             ; preds = %bb50, %bb46
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb50 ], [ 1, %bb46 ]
  %"shell_$IDENT20[][][][][]141" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$IDENT20[][][][]140", i64 %indvars.iv)
  %"shell_$IDENT20[][][][][]_fetch" = load double, double* %"shell_$IDENT20[][][][][]141", align 1
  %"(float)shell_$IDENT20[][][][][]_fetch$" = fptrunc double %"shell_$IDENT20[][][][][]_fetch" to float
  %"(double)shell_$T1_fetch$" = fpext float %"(float)shell_$IDENT20[][][][][]_fetch$" to double
  %"shell_$B[][][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$B[][][][]", i64 %indvars.iv)
  %"shell_$B[][][][][]_fetch" = load double, double* %"shell_$B[][][][][]", align 1
  %mul.61 = fmul fast double %"shell_$B[][][][][]_fetch", 5.000000e-01
  %mul.62 = fmul fast double %mul.61, %"(double)shell_$T1_fetch$"
  %sub.1 = fsub fast double %"(double)shell_$T1_fetch$", %mul.62
  %"shell_$A[][][][][]179" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"shell_$A[][][][]178", i64 %indvars.iv)
  store double %sub.1, double* %"shell_$A[][][][][]179", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %bb53, label %bb50

bb53:                                             ; preds = %bb50
  %indvars.iv.next296 = add nuw nsw i64 %indvars.iv295, 1
  %exitcond297.not = icmp eq i64 %indvars.iv.next296, 6
  br i1 %exitcond297.not, label %bb49, label %bb46

bb49:                                             ; preds = %bb53
  %indvars.iv.next299 = add nuw nsw i64 %indvars.iv298, 1
  %exitcond300 = icmp eq i64 %indvars.iv.next299, %wide.trip.count317
  br i1 %exitcond300, label %bb43.loopexit, label %bb42

bb43.loopexit:                                    ; preds = %bb49
  br label %bb43

bb43:                                             ; preds = %bb43.loopexit, %bb38
  %indvars.iv.next302 = add nuw nsw i64 %indvars.iv301, 1
  %exitcond304 = icmp eq i64 %indvars.iv.next302, %wide.trip.count321
  br i1 %exitcond304, label %bb39.loopexit, label %bb38

bb39.loopexit:                                    ; preds = %bb43
  br label %bb39

bb39:                                             ; preds = %bb39.loopexit, %bb34
  %indvars.iv.next306 = add nuw nsw i64 %indvars.iv305, 1
  %exitcond308 = icmp eq i64 %indvars.iv.next306, %wide.trip.count325
  br i1 %exitcond308, label %bb35.loopexit, label %bb34

bb35.loopexit:                                    ; preds = %bb39
  br label %bb35

bb35:                                             ; preds = %bb35.loopexit, %alloca_0
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

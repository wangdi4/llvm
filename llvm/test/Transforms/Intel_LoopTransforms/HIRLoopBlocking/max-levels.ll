; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -hir-loop-blocking-algo=kandr 2>&1 < %s | FileCheck %s

; Make sure blocking works at boundary condition, generating upto the maximum number of loops after blocking.

; Function: sd_t_d1_1_
;
;        BEGIN REGION { }
;              + DO i1 = 0, sext.i32.i64(%"sd_t_d1_1_$P4D_fetch") + -1, 1   <DO_LOOP>
;              |   + DO i2 = 0, sext.i32.i64(%"sd_t_d1_1_$P5D_fetch") + -1, 1   <DO_LOOP>
;              |   |   + DO i3 = 0, sext.i32.i64(%"sd_t_d1_1_$P6D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   + DO i4 = 0, sext.i32.i64(%"sd_t_d1_1_$H1D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   + DO i5 = 0, sext.i32.i64(%"sd_t_d1_1_$H2D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   + DO i6 = 0, sext.i32.i64(%"sd_t_d1_1_$H3D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   |   + DO i7 = 0, sext.i32.i64(%"sd_t_d1_1_$H7D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   |   |   %sub298 = (%"sd_t_d1_1_$TRIPLESX")[i1][i2][i3][i4][i5][i6];
;              |   |   |   |   |   |   |   %mul113 = (%"sd_t_d1_1_$T2SUB")[i4][i2][i1][i7]  *  (%"sd_t_d1_1_$V2SUB")[i7][i3][i5][i6];
;              |   |   |   |   |   |   |   %sub298 = %sub298  -  %mul113;
;              |   |   |   |   |   |   |   (%"sd_t_d1_1_$TRIPLESX")[i1][i2][i3][i4][i5][i6] = %sub298;
;              |   |   |   |   |   |   + END LOOP
;              |   |   |   |   |   + END LOOP
;              |   |   |   |   + END LOOP
;              |   |   |   + END LOOP
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;        END REGION
;
; Function: sd_t_d1_1_
;
;        BEGIN REGION { modified }
;              + DO i1 = 0, (sext.i32.i64(%"sd_t_d1_1_$H2D_fetch") + -1)/u64, 1   <DO_LOOP>
;              |   %min = (-64 * i1 + sext.i32.i64(%"sd_t_d1_1_$H2D_fetch") + -1 <= 63) ? -64 * i1 + sext.i32.i64(%"sd_t_d1_1_$H2D_fetch") + -1 : 63;
;              |
;              |   + DO i2 = 0, (sext.i32.i64(%"sd_t_d1_1_$H3D_fetch") + -1)/u64, 1   <DO_LOOP>
;              |   |   %min7 = (-64 * i2 + sext.i32.i64(%"sd_t_d1_1_$H3D_fetch") + -1 <= 63) ? -64 * i2 + sext.i32.i64(%"sd_t_d1_1_$H3D_fetch") + -1 : 63;
;              |   |
;              |   |   + DO i3 = 0, sext.i32.i64(%"sd_t_d1_1_$P4D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   + DO i4 = 0, sext.i32.i64(%"sd_t_d1_1_$P5D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   + DO i5 = 0, sext.i32.i64(%"sd_t_d1_1_$P6D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   + DO i6 = 0, sext.i32.i64(%"sd_t_d1_1_$H1D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   |   + DO i7 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;              |   |   |   |   |   |   |   + DO i8 = 0, %min7, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;              |   |   |   |   |   |   |   |   + DO i9 = 0, sext.i32.i64(%"sd_t_d1_1_$H7D_fetch") + -1, 1   <DO_LOOP>
;              |   |   |   |   |   |   |   |   |   %sub298 = (%"sd_t_d1_1_$TRIPLESX")[i3][i4][i5][i6][64 * i1 + i7][64 * i2 + i8];
;              |   |   |   |   |   |   |   |   |   %mul113 = (%"sd_t_d1_1_$T2SUB")[i6][i4][i3][i9]  *  (%"sd_t_d1_1_$V2SUB")[i9][i5][64 * i1 + i7][64 * i2 + i8];
;              |   |   |   |   |   |   |   |   |   %sub298 = %sub298  -  %mul113;
;              |   |   |   |   |   |   |   |   |   (%"sd_t_d1_1_$TRIPLESX")[i3][i4][i5][i6][64 * i1 + i7][64 * i2 + i8] = %sub298;
;              |   |   |   |   |   |   |   |   + END LOOP
;              |   |   |   |   |   |   |   + END LOOP
;              |   |   |   |   |   |   + END LOOP
;              |   |   |   |   |   + END LOOP
;              |   |   |   |   + END LOOP
;              |   |   |   + END LOOP
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;        END REGION

; CHECK: %min
; CHECK: {{%min[0-9]+}}
; CHECK: DO i9

;
; ModuleID = 'input.ll'
source_filename = "ccsd_t_doubles_l.i"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree nounwind uwtable
define void @sd_t_d1_1_(ptr noalias nocapture readonly %"sd_t_d1_1_$H3D", ptr noalias nocapture readonly %"sd_t_d1_1_$H2D", ptr noalias nocapture readonly %"sd_t_d1_1_$H1D", ptr noalias nocapture readonly %"sd_t_d1_1_$P6D", ptr noalias nocapture readonly %"sd_t_d1_1_$P5D", ptr noalias nocapture readonly %"sd_t_d1_1_$P4D", ptr noalias nocapture readonly %"sd_t_d1_1_$H7D", ptr noalias nocapture %"sd_t_d1_1_$TRIPLESX", ptr noalias nocapture readonly %"sd_t_d1_1_$T2SUB", ptr noalias nocapture readonly %"sd_t_d1_1_$V2SUB") local_unnamed_addr #1 {
alloca_3:
  %"sd_t_d1_1_$H3D_fetch" = load i32, ptr %"sd_t_d1_1_$H3D", align 1
  %"sd_t_d1_1_$H2D_fetch" = load i32, ptr %"sd_t_d1_1_$H2D", align 1
  %"sd_t_d1_1_$H1D_fetch" = load i32, ptr %"sd_t_d1_1_$H1D", align 1
  %"sd_t_d1_1_$P6D_fetch" = load i32, ptr %"sd_t_d1_1_$P6D", align 1
  %"sd_t_d1_1_$P5D_fetch" = load i32, ptr %"sd_t_d1_1_$P5D", align 1
  %"sd_t_d1_1_$P4D_fetch" = load i32, ptr %"sd_t_d1_1_$P4D", align 1
  %"sd_t_d1_1_$H7D_fetch" = load i32, ptr %"sd_t_d1_1_$H7D", align 1
  %int_sext = sext i32 %"sd_t_d1_1_$H3D_fetch" to i64
  %mul = shl nsw i64 %int_sext, 3
  %int_sext27 = sext i32 %"sd_t_d1_1_$H2D_fetch" to i64
  %mul28 = mul nsw i64 %mul, %int_sext27
  %int_sext29 = sext i32 %"sd_t_d1_1_$H1D_fetch" to i64
  %mul30 = mul nsw i64 %mul28, %int_sext29
  %int_sext31 = sext i32 %"sd_t_d1_1_$P6D_fetch" to i64
  %mul32 = mul nsw i64 %mul30, %int_sext31
  %int_sext33 = sext i32 %"sd_t_d1_1_$P5D_fetch" to i64
  %mul34 = mul nsw i64 %mul32, %int_sext33
  %int_sext50 = sext i32 %"sd_t_d1_1_$H7D_fetch" to i64
  %mul51 = shl nsw i64 %int_sext50, 3
  %int_sext52 = sext i32 %"sd_t_d1_1_$P4D_fetch" to i64
  %mul53 = mul nsw i64 %mul51, %int_sext52
  %mul56 = mul nsw i64 %mul53, %int_sext33
  %mul87 = mul nsw i64 %mul28, %int_sext31
  %rel = icmp slt i32 %"sd_t_d1_1_$P4D_fetch", 1
  br i1 %rel, label %bb2019, label %bb2018.preheader

bb2018.preheader:                                 ; preds = %alloca_3
  %rel6 = icmp slt i32 %"sd_t_d1_1_$P5D_fetch", 1
  %rel10 = icmp slt i32 %"sd_t_d1_1_$P6D_fetch", 1
  %rel14 = icmp slt i32 %"sd_t_d1_1_$H1D_fetch", 1
  %rel18 = icmp slt i32 %"sd_t_d1_1_$H2D_fetch", 1
  %rel22 = icmp slt i32 %"sd_t_d1_1_$H3D_fetch", 1
  %rel26 = icmp slt i32 %"sd_t_d1_1_$H7D_fetch", 1
  %0 = add nuw nsw i32 %"sd_t_d1_1_$H7D_fetch", 1
  %1 = add nuw nsw i32 %"sd_t_d1_1_$H3D_fetch", 1
  %2 = add nuw nsw i32 %"sd_t_d1_1_$H2D_fetch", 1
  %3 = add nuw nsw i32 %"sd_t_d1_1_$H1D_fetch", 1
  %4 = add nuw nsw i32 %"sd_t_d1_1_$P6D_fetch", 1
  %5 = add nuw nsw i32 %"sd_t_d1_1_$P5D_fetch", 1
  %6 = add nuw nsw i32 %"sd_t_d1_1_$P4D_fetch", 1
  %wide.trip.count321 = sext i32 %6 to i64
  %wide.trip.count317 = sext i32 %5 to i64
  %wide.trip.count313 = sext i32 %4 to i64
  %wide.trip.count309 = sext i32 %3 to i64
  %wide.trip.count305 = sext i32 %2 to i64
  %wide.trip.count301 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb2018

bb2018:                                           ; preds = %bb2023, %bb2018.preheader
  %indvars.iv319 = phi i64 [ 1, %bb2018.preheader ], [ %indvars.iv.next320, %bb2023 ]
  br i1 %rel6, label %bb2023, label %bb2022.preheader

bb2022.preheader:                                 ; preds = %bb2018
  %"sd_t_d1_1_$TRIPLESX[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 5, i64 1, i64 %mul34, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX", i64 %indvars.iv319)
  br label %bb2022

bb2022:                                           ; preds = %bb2027, %bb2022.preheader
  %indvars.iv315 = phi i64 [ 1, %bb2022.preheader ], [ %indvars.iv.next316, %bb2027 ]
  br i1 %rel10, label %bb2027, label %bb2026.preheader

bb2026.preheader:                                 ; preds = %bb2022
  %"sd_t_d1_1_$TRIPLESX[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 4, i64 1, i64 %mul32, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX[]", i64 %indvars.iv315)
  br label %bb2026

bb2026:                                           ; preds = %bb2031, %bb2026.preheader
  %indvars.iv311 = phi i64 [ 1, %bb2026.preheader ], [ %indvars.iv.next312, %bb2031 ]
  br i1 %rel14, label %bb2031, label %bb2030.preheader

bb2030.preheader:                                 ; preds = %bb2026
  %"sd_t_d1_1_$TRIPLESX[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul30, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX[][]", i64 %indvars.iv311)
  br label %bb2030

bb2030:                                           ; preds = %bb2035, %bb2030.preheader
  %indvars.iv307 = phi i64 [ 1, %bb2030.preheader ], [ %indvars.iv.next308, %bb2035 ]
  br i1 %rel18, label %bb2035, label %bb2034.preheader

bb2034.preheader:                                 ; preds = %bb2030
  %"sd_t_d1_1_$TRIPLESX[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul28, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX[][][]", i64 %indvars.iv307)
  %"sd_t_d1_1_$T2SUB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul56, ptr elementtype(double) %"sd_t_d1_1_$T2SUB", i64 %indvars.iv307)
  %"sd_t_d1_1_$T2SUB[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul53, ptr elementtype(double) %"sd_t_d1_1_$T2SUB[]", i64 %indvars.iv315)
  %"sd_t_d1_1_$T2SUB[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul51, ptr elementtype(double) %"sd_t_d1_1_$T2SUB[][]", i64 %indvars.iv319)
  br label %bb2034

bb2034:                                           ; preds = %bb2039, %bb2034.preheader
  %indvars.iv303 = phi i64 [ 1, %bb2034.preheader ], [ %indvars.iv.next304, %bb2039 ]
  br i1 %rel22, label %bb2039, label %bb2038.preheader

bb2038.preheader:                                 ; preds = %bb2034
  %"sd_t_d1_1_$TRIPLESX[][][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX[][][][]", i64 %indvars.iv303)
  br label %bb2038

bb2038:                                           ; preds = %bb2043, %bb2038.preheader
  %indvars.iv299 = phi i64 [ 1, %bb2038.preheader ], [ %indvars.iv.next300, %bb2043 ]
  br i1 %rel26, label %bb2043, label %bb2042.preheader

bb2042.preheader:                                 ; preds = %bb2038
  %"sd_t_d1_1_$TRIPLESX[][][][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sd_t_d1_1_$TRIPLESX[][][][][]", i64 %indvars.iv299)
  %"sd_t_d1_1_$TRIPLESX[][][][][][].promoted" = load double, ptr %"sd_t_d1_1_$TRIPLESX[][][][][][]", align 1
  br label %bb2042

bb2042:                                           ; preds = %bb2042, %bb2042.preheader
  %indvars.iv = phi i64 [ 1, %bb2042.preheader ], [ %indvars.iv.next, %bb2042 ]
  %sub298 = phi double [ %"sd_t_d1_1_$TRIPLESX[][][][][][].promoted", %bb2042.preheader ], [ %sub, %bb2042 ]
  %"sd_t_d1_1_$T2SUB[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sd_t_d1_1_$T2SUB[][][]", i64 %indvars.iv)
  %"sd_t_d1_1_$T2SUB[][][][]_fetch" = load double, ptr %"sd_t_d1_1_$T2SUB[][][][]", align 1
  %"sd_t_d1_1_$V2SUB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul87, ptr elementtype(double) %"sd_t_d1_1_$V2SUB", i64 %indvars.iv)
  %"sd_t_d1_1_$V2SUB[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul28, ptr elementtype(double) %"sd_t_d1_1_$V2SUB[]", i64 %indvars.iv311)
  %"sd_t_d1_1_$V2SUB[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul, ptr elementtype(double) %"sd_t_d1_1_$V2SUB[][]", i64 %indvars.iv303)
  %"sd_t_d1_1_$V2SUB[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"sd_t_d1_1_$V2SUB[][][]", i64 %indvars.iv299)
  %"sd_t_d1_1_$V2SUB[][][][]_fetch" = load double, ptr %"sd_t_d1_1_$V2SUB[][][][]", align 1
  %mul113 = fmul double %"sd_t_d1_1_$T2SUB[][][][]_fetch", %"sd_t_d1_1_$V2SUB[][][][]_fetch"
  %sub = fsub double %sub298, %mul113
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb2043.loopexit, label %bb2042

bb2043.loopexit:                                  ; preds = %bb2042
  %sub.lcssa = phi double [ %sub, %bb2042 ]
  store double %sub.lcssa, ptr %"sd_t_d1_1_$TRIPLESX[][][][][][]", align 1
  br label %bb2043

bb2043:                                           ; preds = %bb2043.loopexit, %bb2038
  %indvars.iv.next300 = add nuw nsw i64 %indvars.iv299, 1
  %exitcond302 = icmp eq i64 %indvars.iv.next300, %wide.trip.count301
  br i1 %exitcond302, label %bb2039.loopexit, label %bb2038

bb2039.loopexit:                                  ; preds = %bb2043
  br label %bb2039

bb2039:                                           ; preds = %bb2039.loopexit, %bb2034
  %indvars.iv.next304 = add nuw nsw i64 %indvars.iv303, 1
  %exitcond306 = icmp eq i64 %indvars.iv.next304, %wide.trip.count305
  br i1 %exitcond306, label %bb2035.loopexit, label %bb2034

bb2035.loopexit:                                  ; preds = %bb2039
  br label %bb2035

bb2035:                                           ; preds = %bb2035.loopexit, %bb2030
  %indvars.iv.next308 = add nuw nsw i64 %indvars.iv307, 1
  %exitcond310 = icmp eq i64 %indvars.iv.next308, %wide.trip.count309
  br i1 %exitcond310, label %bb2031.loopexit, label %bb2030

bb2031.loopexit:                                  ; preds = %bb2035
  br label %bb2031

bb2031:                                           ; preds = %bb2031.loopexit, %bb2026
  %indvars.iv.next312 = add nuw nsw i64 %indvars.iv311, 1
  %exitcond314 = icmp eq i64 %indvars.iv.next312, %wide.trip.count313
  br i1 %exitcond314, label %bb2027.loopexit, label %bb2026

bb2027.loopexit:                                  ; preds = %bb2031
  br label %bb2027

bb2027:                                           ; preds = %bb2027.loopexit, %bb2022
  %indvars.iv.next316 = add nuw nsw i64 %indvars.iv315, 1
  %exitcond318 = icmp eq i64 %indvars.iv.next316, %wide.trip.count317
  br i1 %exitcond318, label %bb2023.loopexit, label %bb2022

bb2023.loopexit:                                  ; preds = %bb2027
  br label %bb2023

bb2023:                                           ; preds = %bb2023.loopexit, %bb2018
  %indvars.iv.next320 = add nuw nsw i64 %indvars.iv319, 1
  %exitcond322 = icmp eq i64 %indvars.iv.next320, %wide.trip.count321
  br i1 %exitcond322, label %bb2019.loopexit, label %bb2018

bb2019.loopexit:                                  ; preds = %bb2023
  br label %bb2019

bb2019:                                           ; preds = %bb2019.loopexit, %alloca_3
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }

!omp_offload.info = !{}

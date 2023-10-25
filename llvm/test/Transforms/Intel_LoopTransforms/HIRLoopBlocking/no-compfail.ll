; REQUIRES: asserts
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-loop-blocking,print<hir>" -debug-only=hir-loop-blocking -disable-output 2>&1 < %s | FileCheck %s
;
; Verify that loop-blocking doesn't compfail.
; Previously, it was compfailing by proceeding even when the number of total loops after blocking exceeds max loop levels.

; Before loop blocking transformation
;
; Function: bspline_mp_dbs3gd_
;
;         BEGIN REGION { }
;               + DO i1 = 0, zext.i32.i64(%NZVEC_fetch.36) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   + DO i2 = 0, sext.i32.i64(%NYVEC_fetch.35) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   + DO i3 = 0, sext.i32.i64(%KZ_fetch.42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   + DO i4 = 0, sext.i32.i64(%KY_fetch.40) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   |   + DO i5 = 0, sext.i32.i64(%KX_fetch.38) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   |   |   + DO i6 = 0, sext.i32.i64(%NXVEC_fetch.34) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;               |   |   |   |   |   |   %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83" = (%"bspline_mp_dbs3gd_$BIATY30")[i4][i2];
;               |   |   |   |   |   |   %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90" = (%"bspline_mp_dbs3gd_$BIATZ24")[i3][i1];
;               |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100" = (%"bspline_mp_dbs3gd_$LEFTY44")[i2];
;               |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105" = (%"bspline_mp_dbs3gd_$LEFTZ40")[i1];
;               |   |   |   |   |   |   %mul.24 = %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83"  *  (%"bspline_mp_dbs3gd_$BIATX36")[i5][i6];
;               |   |   |   |   |   |   %mul.27 = %mul.24  *  %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90";
;               |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95" = (%"bspline_mp_dbs3gd_$LEFTX48")[i6];
;               |   |   |   |   |   |   %mul.32 = %mul.27  *  (%BCOEF)[i3 + -1 * %KZ_fetch.42 + %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105" + 1][i4 + -1 * %KY_fetch.40 + %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100" + 1][i5 + -1 * %KX_fetch.38 + %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95" + 1];
;               |   |   |   |   |   |   %add.11 = %mul.32  +  (%VAL)[i1][i2][i6];
;               |   |   |   |   |   |   (%VAL)[i1][i2][i6] = %add.11;
;               |   |   |   |   |   + END LOOP
;               |   |   |   |   + END LOOP
;               |   |   |   + END LOOP
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;         END REGION

; After loop blocking transformation
;
; Function: bspline_mp_dbs3gd_
;
; CHECK: Loop at Level 5 will be stripmined by missing IV.
; CHECK: Loop at Level 4 will be stripmined by missing IV.
; CHECK: Loop at Level 3 will be stripmined by missing IV.
; CHECK: Total number of loops reached the limit. No more blocking opportunity will be considered
; CHECK-DAG: At Level 3, stripmine can not be done
; CHECK-DAG: At Level 4, stripmine can not be done
; CHECK-DAG: At Level 5, stripmine can not be done

; Later on none of the loops are stripmined due to legality of stripmining.

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, zext.i32.i64(%NZVEC_fetch.36) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%NYVEC_fetch.35) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   + DO i3 = 0, sext.i32.i64(%KZ_fetch.42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   |   + DO i4 = 0, sext.i32.i64(%KY_fetch.40) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   |   |   + DO i5 = 0, sext.i32.i64(%KX_fetch.38) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   |   |   |   + DO i6 = 0, sext.i32.i64(%NXVEC_fetch.34) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   |   |   |   |   |   %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83" = (%"bspline_mp_dbs3gd_$BIATY30")[i4][i2];
; CHECK:              |   |   |   |   |   |   %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90" = (%"bspline_mp_dbs3gd_$BIATZ24")[i3][i1];
; CHECK:              |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100" = (%"bspline_mp_dbs3gd_$LEFTY44")[i2];
; CHECK:              |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105" = (%"bspline_mp_dbs3gd_$LEFTZ40")[i1];
; CHECK:              |   |   |   |   |   |   %mul.24 = %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83"  *  (%"bspline_mp_dbs3gd_$BIATX36")[i5][i6];
; CHECK:              |   |   |   |   |   |   %mul.27 = %mul.24  *  %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90";
; CHECK:              |   |   |   |   |   |   %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95" = (%"bspline_mp_dbs3gd_$LEFTX48")[i6];
; CHECK:              |   |   |   |   |   |   %mul.32 = %mul.27  *  (%BCOEF)[i3 + -1 * %KZ_fetch.42 + %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105" + 1][i4 + -1 * %KY_fetch.40 + %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100" + 1][i5 + -1 * %KX_fetch.38 + %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95" + 1];
; CHECK:              |   |   |   |   |   |   %add.11 = %mul.32  +  (%VAL)[i1][i2][i6];
; CHECK:              |   |   |   |   |   |   (%VAL)[i1][i2][i6] = %add.11;
; CHECK:              |   |   |   |   |   + END LOOP
; CHECK:              |   |   |   |   + END LOOP
; CHECK:              |   |   |   + END LOOP
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; ModuleID = 'module.ll'
source_filename = "/tmp/ifx1994870584fmI41z/ifx0qwlus.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit = external hidden unnamed_addr constant [6 x i8]
@bspline_mp_routine_ = external local_unnamed_addr global [80 x i8], align 8

; Function Attrs: nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define void @bspline_mp_dbs3gd_(ptr noalias nocapture readonly dereferenceable(4) %IDERX, ptr noalias nocapture readonly dereferenceable(4) %IDERY, ptr noalias nocapture readonly dereferenceable(4) %IDERZ, ptr noalias nocapture readonly dereferenceable(4) %NXVEC, ptr noalias nocapture readnone dereferenceable(8) %XVEC, ptr noalias nocapture readonly dereferenceable(4) %NYVEC, ptr noalias nocapture readnone dereferenceable(8) %YVEC, ptr noalias nocapture readonly dereferenceable(4) %NZVEC, ptr noalias nocapture readnone dereferenceable(8) %ZVEC, ptr noalias nocapture readonly dereferenceable(4) %KX, ptr noalias nocapture readonly dereferenceable(4) %KY, ptr noalias nocapture readonly dereferenceable(4) %KZ, ptr noalias nocapture readnone dereferenceable(8) %XKNOT, ptr noalias nocapture readnone dereferenceable(8) %YKNOT, ptr noalias nocapture readnone dereferenceable(8) %ZKNOT, ptr noalias nocapture readonly dereferenceable(4) %NX, ptr noalias nocapture readonly dereferenceable(4) %NY, ptr noalias nocapture readonly dereferenceable(4) %NZ, ptr noalias nocapture readonly dereferenceable(8) %BCOEF, ptr noalias nocapture dereferenceable(8) %VAL, ptr noalias nocapture readonly dereferenceable(4) %LDF, ptr noalias nocapture readonly dereferenceable(4) %MDF, ptr noalias nocapture writeonly dereferenceable(4) %IERR) local_unnamed_addr #0 {
alloca_1:
  %NXVEC_fetch.34 = load i32, ptr %NXVEC, align 1, !tbaa !1
  %NYVEC_fetch.35 = load i32, ptr %NYVEC, align 1, !tbaa !6
  %NZVEC_fetch.36 = load i32, ptr %NZVEC, align 1, !tbaa !8
  %NX_fetch.37 = load i32, ptr %NX, align 1, !tbaa !10, !llfort.type_idx !12
  %KX_fetch.38 = load i32, ptr %KX, align 1, !tbaa !13
  %NY_fetch.39 = load i32, ptr %NY, align 1, !tbaa !15, !llfort.type_idx !17
  %KY_fetch.40 = load i32, ptr %KY, align 1, !tbaa !18
  %KZ_fetch.42 = load i32, ptr %KZ, align 1, !tbaa !20
  %LDF_fetch.43 = load i32, ptr %LDF, align 1, !tbaa !22, !llfort.type_idx !24
  %MDF_fetch.44 = load i32, ptr %MDF, align 1, !tbaa !25, !llfort.type_idx !27
  %int_sext20 = sext i32 %NZVEC_fetch.36 to i64, !llfort.type_idx !28
  %rel.19 = icmp sgt i64 %int_sext20, 0
  %slct.19 = select i1 %rel.19, i64 %int_sext20, i64 0
  %mul.7 = shl nuw nsw i64 %slct.19, 3
  %int_sext22 = sext i32 %KZ_fetch.42 to i64, !llfort.type_idx !28
  %rel.20 = icmp sgt i64 %int_sext22, 0
  %slct.20 = select i1 %rel.20, i64 %int_sext22, i64 0
  %mul.8 = mul nsw i64 %mul.7, %slct.20
  %div.5 = lshr exact i64 %mul.8, 3
  %"bspline_mp_dbs3gd_$BIATZ24" = alloca double, i64 %div.5, align 8, !llfort.type_idx !29
  %int_sext26 = sext i32 %NYVEC_fetch.35 to i64, !llfort.type_idx !28
  %rel.21 = icmp sgt i64 %int_sext26, 0
  %slct.21 = select i1 %rel.21, i64 %int_sext26, i64 0
  %mul.9 = shl nuw nsw i64 %slct.21, 3
  %int_sext28 = sext i32 %KY_fetch.40 to i64, !llfort.type_idx !28
  %rel.22 = icmp sgt i64 %int_sext28, 0
  %slct.22 = select i1 %rel.22, i64 %int_sext28, i64 0
  %mul.10 = mul nsw i64 %mul.9, %slct.22
  %div.6 = lshr exact i64 %mul.10, 3
  %"bspline_mp_dbs3gd_$BIATY30" = alloca double, i64 %div.6, align 8, !llfort.type_idx !30
  %int_sext32 = sext i32 %NXVEC_fetch.34 to i64, !llfort.type_idx !28
  %rel.23 = icmp sgt i64 %int_sext32, 0
  %slct.23 = select i1 %rel.23, i64 %int_sext32, i64 0
  %mul.11 = shl nuw nsw i64 %slct.23, 3
  %int_sext34 = sext i32 %KX_fetch.38 to i64, !llfort.type_idx !28
  %rel.24 = icmp sgt i64 %int_sext34, 0
  %slct.24 = select i1 %rel.24, i64 %int_sext34, i64 0
  %mul.12 = mul nsw i64 %mul.11, %slct.24
  %div.7 = lshr exact i64 %mul.12, 3
  %"bspline_mp_dbs3gd_$BIATX36" = alloca double, i64 %div.7, align 8, !llfort.type_idx !31
  %"bspline_mp_dbs3gd_$LEFTZ40" = alloca i32, i64 %slct.19, align 4, !llfort.type_idx !32
  %"bspline_mp_dbs3gd_$LEFTY44" = alloca i32, i64 %slct.21, align 4, !llfort.type_idx !33
  %"bspline_mp_dbs3gd_$LEFTX48" = alloca i32, i64 %slct.23, align 4, !llfort.type_idx !34
  %int_sext54 = sext i32 %LDF_fetch.43 to i64, !llfort.type_idx !28
  %mul.16 = shl nsw i64 %int_sext54, 3
  %int_sext55 = sext i32 %MDF_fetch.44 to i64, !llfort.type_idx !28
  %mul.17 = mul nsw i64 %mul.16, %int_sext55
  %mul.20 = shl nsw i64 %int_sext32, 3
  %mul.22 = shl nsw i64 %int_sext26, 3
  %mul.25 = shl nsw i64 %int_sext20, 3
  %int_sext63 = sext i32 %NX_fetch.37 to i64, !llfort.type_idx !28
  %mul.28 = shl nsw i64 %int_sext63, 3
  %int_sext64 = sext i32 %NY_fetch.39 to i64, !llfort.type_idx !28
  %mul.29 = mul nsw i64 %mul.28, %int_sext64
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(6) @bspline_mp_routine_, ptr noundef nonnull align 1 dereferenceable(6) @strlit, i64 6, i1 false)
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 2 dereferenceable(74) getelementptr inbounds ([80 x i8], ptr @bspline_mp_routine_, i64 0, i64 6), i8 32, i64 74, i1 false)
  store i32 0, ptr %IERR, align 1, !tbaa !35
  %rel.28 = icmp slt i32 %NZVEC_fetch.36, 1
  br i1 %rel.28, label %do.end_do19, label %do.body18.preheader

do.body18.preheader:                              ; preds = %alloca_1
  %rel.29 = icmp slt i32 %NYVEC_fetch.35, 1
  %rel.30 = icmp slt i32 %KZ_fetch.42, 1
  %rel.31 = icmp slt i32 %KY_fetch.40, 1
  %rel.32 = icmp slt i32 %KX_fetch.38, 1
  %rel.33 = icmp slt i32 %NXVEC_fetch.34, 1
  %0 = add nuw nsw i32 %NXVEC_fetch.34, 1
  %1 = add nuw nsw i32 %KX_fetch.38, 1
  %2 = add nuw nsw i32 %KY_fetch.40, 1
  %3 = add nuw nsw i32 %KZ_fetch.42, 1
  %4 = add nuw nsw i32 %NYVEC_fetch.35, 1
  %5 = add nuw nsw i32 %NZVEC_fetch.36, 1
  %wide.trip.count171 = zext i32 %5 to i64
  %wide.trip.count167 = sext i32 %4 to i64
  %wide.trip.count163 = sext i32 %3 to i64
  %wide.trip.count159 = sext i32 %2 to i64
  %wide.trip.count155 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body18

do.body18:                                        ; preds = %do.end_do23, %do.body18.preheader
  %indvars.iv169 = phi i64 [ 1, %do.body18.preheader ], [ %indvars.iv.next170, %do.end_do23 ]
  br i1 %rel.29, label %do.end_do23, label %do.body22.preheader

do.body22.preheader:                              ; preds = %do.body18
  %"VAL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.17, ptr nonnull elementtype(double) %VAL, i64 %indvars.iv169)
  %"bspline_mp_dbs3gd_$LEFTZ40[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"bspline_mp_dbs3gd_$LEFTZ40", i64 %indvars.iv169)
  br label %do.body22

do.body22:                                        ; preds = %do.end_do27, %do.body22.preheader
  %indvars.iv165 = phi i64 [ 1, %do.body22.preheader ], [ %indvars.iv.next166, %do.end_do27 ]
  br i1 %rel.30, label %do.end_do27, label %do.body26.preheader

do.body26.preheader:                              ; preds = %do.body22
  %"VAL[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.16, ptr nonnull elementtype(double) %"VAL[]", i64 %indvars.iv165)
  %"bspline_mp_dbs3gd_$LEFTY44[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"bspline_mp_dbs3gd_$LEFTY44", i64 %indvars.iv165)
  br label %do.body26

do.body26:                                        ; preds = %do.end_do31, %do.body26.preheader
  %indvars.iv161 = phi i64 [ 1, %do.body26.preheader ], [ %indvars.iv.next162, %do.end_do31 ]
  br i1 %rel.31, label %do.end_do31, label %do.body30.preheader

do.body30.preheader:                              ; preds = %do.body26
  %"bspline_mp_dbs3gd_$BIATZ24[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.25, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATZ24", i64 %indvars.iv161)
  %"bspline_mp_dbs3gd_$BIATZ24[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATZ24[]", i64 %indvars.iv169)
  %6 = trunc i64 %indvars.iv161 to i32
  %sub.3 = sub i32 %6, %KZ_fetch.42
  br label %do.body30

do.body30:                                        ; preds = %do.end_do35, %do.body30.preheader
  %indvars.iv157 = phi i64 [ 1, %do.body30.preheader ], [ %indvars.iv.next158, %do.end_do35 ]
  br i1 %rel.32, label %do.end_do35, label %do.body34.preheader

do.body34.preheader:                              ; preds = %do.body30
  %"bspline_mp_dbs3gd_$BIATY30[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.22, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATY30", i64 %indvars.iv157)
  %"bspline_mp_dbs3gd_$BIATY30[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATY30[]", i64 %indvars.iv165)
  %7 = trunc i64 %indvars.iv157 to i32
  %sub.2 = sub i32 %7, %KY_fetch.40
  br label %do.body34

do.body34:                                        ; preds = %do.end_do39, %do.body34.preheader
  %indvars.iv153 = phi i64 [ 1, %do.body34.preheader ], [ %indvars.iv.next154, %do.end_do39 ]
  br i1 %rel.33, label %do.end_do39, label %do.body38.preheader

do.body38.preheader:                              ; preds = %do.body34
  %"bspline_mp_dbs3gd_$BIATX36[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.20, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATX36", i64 %indvars.iv153), !llfort.type_idx !31
  %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83" = load double, ptr %"bspline_mp_dbs3gd_$BIATY30[][]", align 8, !tbaa !37, !llfort.type_idx !30
  %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90" = load double, ptr %"bspline_mp_dbs3gd_$BIATZ24[][]", align 8, !tbaa !39, !llfort.type_idx !29
  %8 = trunc i64 %indvars.iv153 to i32
  %sub.1 = sub i32 %8, %KX_fetch.38
  %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100" = load i32, ptr %"bspline_mp_dbs3gd_$LEFTY44[]", align 4, !tbaa !41, !llfort.type_idx !33
  %add.7 = add i32 %sub.2, %"bspline_mp_dbs3gd_$LEFTY44[]_fetch.100"
  %int_sext78 = sext i32 %add.7 to i64, !llfort.type_idx !28
  %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105" = load i32, ptr %"bspline_mp_dbs3gd_$LEFTZ40[]", align 4, !tbaa !43, !llfort.type_idx !32
  %add.8 = add i32 %sub.3, %"bspline_mp_dbs3gd_$LEFTZ40[]_fetch.105"
  %int_sext80 = sext i32 %add.8 to i64, !llfort.type_idx !28
  %"BCOEF[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.29, ptr nonnull elementtype(double) %BCOEF, i64 %int_sext80), !llfort.type_idx !45
  %"BCOEF[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.28, ptr nonnull elementtype(double) %"BCOEF[]", i64 %int_sext78), !llfort.type_idx !45
  br label %do.body38

do.body38:                                        ; preds = %do.body38, %do.body38.preheader
  %indvars.iv = phi i64 [ 1, %do.body38.preheader ], [ %indvars.iv.next, %do.body38 ]
  %"VAL[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"VAL[][]", i64 %indvars.iv), !llfort.type_idx !46
  %"VAL[][][]_fetch.69" = load double, ptr %"VAL[][][]", align 1, !tbaa !47, !llfort.type_idx !49
  %"bspline_mp_dbs3gd_$BIATX36[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"bspline_mp_dbs3gd_$BIATX36[]", i64 %indvars.iv), !llfort.type_idx !31
  %"bspline_mp_dbs3gd_$BIATX36[][]_fetch.76" = load double, ptr %"bspline_mp_dbs3gd_$BIATX36[][]", align 8, !tbaa !50, !llfort.type_idx !31
  %mul.24 = fmul reassoc ninf nsz arcp contract afn double %"bspline_mp_dbs3gd_$BIATY30[][]_fetch.83", %"bspline_mp_dbs3gd_$BIATX36[][]_fetch.76"
  %mul.27 = fmul reassoc ninf nsz arcp contract afn double %mul.24, %"bspline_mp_dbs3gd_$BIATZ24[][]_fetch.90"
  %"bspline_mp_dbs3gd_$LEFTX48[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %"bspline_mp_dbs3gd_$LEFTX48", i64 %indvars.iv), !llfort.type_idx !34
  %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95" = load i32, ptr %"bspline_mp_dbs3gd_$LEFTX48[]", align 4, !tbaa !52, !llfort.type_idx !34
  %add.6 = add i32 %sub.1, %"bspline_mp_dbs3gd_$LEFTX48[]_fetch.95"
  %int_sext76 = sext i32 %add.6 to i64, !llfort.type_idx !28
  %"BCOEF[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"BCOEF[][]", i64 %int_sext76), !llfort.type_idx !45
  %"BCOEF[][][]_fetch.111" = load double, ptr %"BCOEF[][][]", align 1, !tbaa !54, !llfort.type_idx !56
  %mul.32 = fmul reassoc ninf nsz arcp contract afn double %mul.27, %"BCOEF[][][]_fetch.111"
  %add.11 = fadd reassoc ninf nsz arcp contract afn double %mul.32, %"VAL[][][]_fetch.69"
  store double %add.11, ptr %"VAL[][][]", align 1, !tbaa !47
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do39.loopexit, label %do.body38

do.end_do39.loopexit:                             ; preds = %do.body38
  br label %do.end_do39

do.end_do39:                                      ; preds = %do.end_do39.loopexit, %do.body34
  %indvars.iv.next154 = add nuw nsw i64 %indvars.iv153, 1
  %exitcond156 = icmp eq i64 %indvars.iv.next154, %wide.trip.count155
  br i1 %exitcond156, label %do.end_do35.loopexit, label %do.body34

do.end_do35.loopexit:                             ; preds = %do.end_do39
  br label %do.end_do35

do.end_do35:                                      ; preds = %do.end_do35.loopexit, %do.body30
  %indvars.iv.next158 = add nuw nsw i64 %indvars.iv157, 1
  %exitcond160 = icmp eq i64 %indvars.iv.next158, %wide.trip.count159
  br i1 %exitcond160, label %do.end_do31.loopexit, label %do.body30

do.end_do31.loopexit:                             ; preds = %do.end_do35
  br label %do.end_do31

do.end_do31:                                      ; preds = %do.end_do31.loopexit, %do.body26
  %indvars.iv.next162 = add nuw nsw i64 %indvars.iv161, 1
  %exitcond164 = icmp eq i64 %indvars.iv.next162, %wide.trip.count163
  br i1 %exitcond164, label %do.end_do27.loopexit, label %do.body26

do.end_do27.loopexit:                             ; preds = %do.end_do31
  br label %do.end_do27

do.end_do27:                                      ; preds = %do.end_do27.loopexit, %do.body22
  %indvars.iv.next166 = add nuw nsw i64 %indvars.iv165, 1
  %exitcond168 = icmp eq i64 %indvars.iv.next166, %wide.trip.count167
  br i1 %exitcond168, label %do.end_do23.loopexit, label %do.body22

do.end_do23.loopexit:                             ; preds = %do.end_do27
  br label %do.end_do23

do.end_do23:                                      ; preds = %do.end_do23.loopexit, %do.body18
  %indvars.iv.next170 = add nuw nsw i64 %indvars.iv169, 1
  %exitcond172 = icmp eq i64 %indvars.iv.next170, %wide.trip.count171
  br i1 %exitcond172, label %do.end_do19.loopexit, label %do.body18

do.end_do19.loopexit:                             ; preds = %do.end_do23
  br label %do.end_do19

do.end_do19:                                      ; preds = %do.end_do19.loopexit, %alloca_1
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #3 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$1", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$bspline_mp_dbs3gd_"}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$2", !3, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$3", !3, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$4", !3, i64 0}
!12 = !{i64 244}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$5", !3, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$6", !3, i64 0}
!17 = !{i64 246}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$7", !3, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$9", !3, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$10", !3, i64 0}
!24 = !{i64 250}
!25 = !{!26, !26, i64 0}
!26 = !{!"ifx$unique_sym$11", !3, i64 0}
!27 = !{i64 251}
!28 = !{i64 3}
!29 = !{i64 235}
!30 = !{i64 236}
!31 = !{i64 237}
!32 = !{i64 238}
!33 = !{i64 239}
!34 = !{i64 240}
!35 = !{!36, !36, i64 0}
!36 = !{!"ifx$unique_sym$13", !3, i64 0}
!37 = !{!38, !38, i64 0}
!38 = !{!"ifx$unique_sym$22", !3, i64 0}
!39 = !{!40, !40, i64 0}
!40 = !{!"ifx$unique_sym$23", !3, i64 0}
!41 = !{!42, !42, i64 0}
!42 = !{!"ifx$unique_sym$25", !3, i64 0}
!43 = !{!44, !44, i64 0}
!44 = !{!"ifx$unique_sym$26", !3, i64 0}
!45 = !{i64 64}
!46 = !{i64 66}
!47 = !{!48, !48, i64 0}
!48 = !{!"ifx$unique_sym$20", !3, i64 0}
!49 = !{i64 259}
!50 = !{!51, !51, i64 0}
!51 = !{!"ifx$unique_sym$21", !3, i64 0}
!52 = !{!53, !53, i64 0}
!53 = !{!"ifx$unique_sym$24", !3, i64 0}
!54 = !{!55, !55, i64 0}
!55 = !{!"ifx$unique_sym$27", !3, i64 0}
!56 = !{i64 263}

; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-loop-distribute-memrec" -print-before=hir-loop-distribute-memrec -print-after=hir-loop-distribute-memrec -disable-output < %s 2>&1 | FileCheck %s

; Check that distribution happens for multiple chunks where multiple def/uses of the same temps occur in different chunks.
; In this case %"(float)div.1$" is stored into TempArray and reused across 3 distributed loops. Note that only the
; last definite of the temp is stored into the TempArray for each chunk.

; Before Unrolling

;       BEGIN REGION { }
;             + DO i1 = 0, 9999, 1   <DO_LOOP>
;             |   %0 = trunc.i64.i32(i1 + 1);
;             |
;             |   + DO i2 = 0, 9, 1   <DO_LOOP>
;             |   |   %"(float)mul.1$" = sitofp.i32.float(i1 + %indvars.iv35 * i2 + 1);
;             |   |   (@"noswpipe_d_$C")[0][i2][i1] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(i2 + 1);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][i2][i1] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][i2][i1] = %add.1;
;             |   + END LOOP
;             |
;             |   %indvars.iv35 = i1 + 2;
;             + END LOOP
;       END REGION

; After 8x Unrolling and Distribution

; CHECK: BEGIN REGION { modified }
; CHECK:      + DO i1 = 0, 156, 1   <DO_LOOP>
;             |   %min = (-64 * i1 + 9999 <= 63) ? -64 * i1 + 9999 : 63;
;             |
; CHECK:      |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;             |   |   %0 = trunc.i64.i32(64 * i1 + i2 + 1);
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 1);
;             |   |   (@"noswpipe_d_$C")[0][0][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(1);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][0][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][0][64 * i1 + i2] = %add.1;
;             |   |   %3 = trunc.i64.i32(2);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
; CHECK:      |   |   (%.TempArray)[0][i2] = %"(float)div.1$";
;             |   |   (@"noswpipe_d_$B")[0][1][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %3 = trunc.i64.i32(3);
; CHECK:      |   + END LOOP
; CHECK:      |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;             |   |   %0 = trunc.i64.i32(64 * i1 + i2 + 1);
; CHECK:      |   |   %"(float)div.1$" = (%.TempArray)[0][i2];
;             |   |   %3 = trunc.i64.i32(3);
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][1][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][1][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 2 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][2][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][2][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][2][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 3 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][3][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(4);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][3][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][3][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 4 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][4][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(5);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][4][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][4][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 5 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][5][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(6);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][5][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][5][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 6 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][6][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(7);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][6][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][6][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 7 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][7][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(8);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][7][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][7][64 * i1 + i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 8 * %indvars.iv35 + 1);
;             |   |   (@"noswpipe_d_$C")[0][8][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(9);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
; CHECK:      |   |   (%.TempArray)[0][i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
; CHECK:      |   |   (%.TempArray3)[0][i2] = %add.1;
;             |   |   %"(float)mul.1$" = sitofp.i32.float(64 * i1 + i2 + 9 * %indvars.iv35 + 1);
; CHECK:      |   |   (%.TempArray5)[0][i2] = %"(float)mul.1$";
;             |   |   %indvars.iv35 = 64 * i1 + i2 + 2;
; CHECK:      |   + END LOOP
; CHECK:      |   + DO i2 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
;             |   |   %0 = trunc.i64.i32(64 * i1 + i2 + 1);
; CHECK:      |   |   %"(float)div.1$" = (%.TempArray)[0][i2];
; CHECK:      |   |   %add.1 = (%.TempArray3)[0][i2];
; CHECK:      |   |   %"(float)mul.1$" = (%.TempArray5)[0][i2];
;             |   |   (@"noswpipe_d_$B")[0][8][64 * i1 + i2] = %"(float)div.1$";
;             |   |   (@"noswpipe_d_$A")[0][8][64 * i1 + i2] = %add.1;
;             |   |   (@"noswpipe_d_$C")[0][9][64 * i1 + i2] = %"(float)mul.1$";
;             |   |   %3 = trunc.i64.i32(10);
;             |   |   %"(float)div.1$" = sitofp.i32.float((%0 /u %3));
;             |   |   (@"noswpipe_d_$B")[0][9][64 * i1 + i2] = %"(float)div.1$";
;             |   |   %add.1 = %"(float)div.1$"  +  %"(float)mul.1$";
;             |   |   (@"noswpipe_d_$A")[0][9][64 * i1 + i2] = %add.1;
; CHECK:      |   + END LOOP
; CHECK:      + END LOOP
;       END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"noswpipe_d_$C" = internal unnamed_addr global [10 x [10000 x float]] zeroinitializer, align 16
@"noswpipe_d_$B" = internal unnamed_addr global [10 x [10000 x float]] zeroinitializer, align 16
@"noswpipe_d_$A" = internal unnamed_addr global [10 x [10000 x float]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 65536, align 4
@1 = internal unnamed_addr constant i32 2, align 4
@strlit = internal unnamed_addr constant [0 x i8] zeroinitializer

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %func_result = tail call i32 @for_set_fpe_(ptr nonnull @0) #5, !llfort.type_idx !0
  %func_result2 = tail call i32 @for_set_reentrancy(ptr nonnull @1) #5, !llfort.type_idx !0
  br label %do.body6

do.body6:                                         ; preds = %do.epilog13, %alloca_0
  %indvars.iv35 = phi i64 [ %indvars.iv.next36, %do.epilog13 ], [ 1, %alloca_0 ]
  %0 = trunc i64 %indvars.iv35 to i32
  br label %do.body10

do.body10:                                        ; preds = %do.body10, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body10 ], [ 1, %do.body6 ]
  %1 = mul nuw nsw i64 %indvars.iv, %indvars.iv35
  %2 = trunc i64 %1 to i32
  %"(float)mul.1$" = sitofp i32 %2 to float, !llfort.type_idx !1
  %"noswpipe_d_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) @"noswpipe_d_$C", i64 %indvars.iv), !llfort.type_idx !1, !ifx.array_extent !2
  %"noswpipe_d_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"noswpipe_d_$C[]", i64 %indvars.iv35), !llfort.type_idx !1
  store float %"(float)mul.1$", ptr %"noswpipe_d_$C[][]", align 4, !tbaa !3
  %3 = trunc i64 %indvars.iv to i32
  %div.1 = udiv i32 %0, %3
  %"(float)div.1$" = sitofp i32 %div.1 to float, !llfort.type_idx !1
  %"noswpipe_d_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) @"noswpipe_d_$B", i64 %indvars.iv), !llfort.type_idx !1, !ifx.array_extent !2
  %"noswpipe_d_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"noswpipe_d_$B[]", i64 %indvars.iv35), !llfort.type_idx !1
  store float %"(float)div.1$", ptr %"noswpipe_d_$B[][]", align 4, !tbaa !8
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"(float)div.1$", %"(float)mul.1$"
  %"noswpipe_d_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) @"noswpipe_d_$A", i64 %indvars.iv), !llfort.type_idx !1, !ifx.array_extent !2
  %"noswpipe_d_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"noswpipe_d_$A[]", i64 %indvars.iv35), !llfort.type_idx !1
  store float %add.1, ptr %"noswpipe_d_$A[][]", align 4, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond.not, label %do.epilog13, label %do.body10

do.epilog13:                                      ; preds = %do.body10
  %indvars.iv.next36 = add nuw nsw i64 %indvars.iv35, 1
  %exitcond37.not = icmp eq i64 %indvars.iv.next36, 10001
  br i1 %exitcond37.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.epilog13
  tail call void @llvm.experimental.noalias.scope.decl(metadata !12)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !15)
  tail call void @llvm.experimental.noalias.scope.decl(metadata !17)
  ret void
}

declare !llfort.intrin_id !39 i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !40 i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nofree
declare !llfort.intrin_id !41 i32 @for_stop_core_quiet(ptr nocapture readonly, i32, i32, i64, i32, i32, ...) local_unnamed_addr #1

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$B", ptr noalias nocapture dereferenceable(4) %"sub_$C", ptr noalias nocapture readonly dereferenceable(4) %"sub_$SIZE") local_unnamed_addr #3 {
alloca_1:
  %"sub_$SIZE_fetch.22" = load i32, ptr %"sub_$SIZE", align 1, !tbaa !42
  %rel.3 = icmp slt i32 %"sub_$SIZE_fetch.22", 1
  br i1 %rel.3, label %do.end_do36, label %do.body26.preheader

do.body26.preheader:                              ; preds = %alloca_1
  %0 = add nuw nsw i32 %"sub_$SIZE_fetch.22", 1
  %wide.trip.count47 = zext nneg i32 %0 to i64
  br label %do.body26

do.body26:                                        ; preds = %do.body26.preheader, %do.end_do31.loopexit
  %indvars.iv45 = phi i64 [ 1, %do.body26.preheader ], [ %indvars.iv.next46, %do.end_do31.loopexit ]
  br label %do.body30

do.body30:                                        ; preds = %do.body26, %do.body30
  %indvars.iv = phi i64 [ 1, %do.body26 ], [ %indvars.iv.next, %do.body30 ]
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv), !llfort.type_idx !19
  %"sub_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B[]", i64 %indvars.iv45), !llfort.type_idx !19
  %"sub_$B[][]_fetch.28" = load float, ptr %"sub_$B[][]", align 1, !tbaa !47, !llfort.type_idx !27
  %"sub_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$C", i64 %indvars.iv), !llfort.type_idx !28
  %"sub_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$C[]", i64 %indvars.iv45), !llfort.type_idx !28
  %"sub_$C[][]_fetch.31" = load float, ptr %"sub_$C[][]", align 1, !tbaa !49, !llfort.type_idx !32
  %add.4 = fadd reassoc ninf nsz arcp contract afn float %"sub_$C[][]_fetch.31", %"sub_$B[][]_fetch.28"
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv), !llfort.type_idx !33
  %"sub_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A[]", i64 %indvars.iv45), !llfort.type_idx !33
  store float %add.4, ptr %"sub_$A[][]", align 1, !tbaa !51
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count47
  br i1 %exitcond, label %do.end_do31.loopexit, label %do.body30

do.end_do31.loopexit:                             ; preds = %do.body30
  %indvars.iv.next46 = add nuw nsw i64 %indvars.iv45, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next46, %wide.trip.count47
  br i1 %exitcond48, label %do.body35.preheader, label %do.body26

do.body35.preheader:                              ; preds = %do.end_do31.loopexit
  br label %do.body35

do.body35:                                        ; preds = %do.body35.preheader, %do.end_do40.loopexit
  %indvars.iv53 = phi i64 [ %indvars.iv.next54, %do.end_do40.loopexit ], [ 1, %do.body35.preheader ]
  br label %do.body39

do.body39:                                        ; preds = %do.body35, %do.body39
  %indvars.iv49 = phi i64 [ 1, %do.body35 ], [ %indvars.iv.next50, %do.body39 ]
  %"sub_$A[]11" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv49), !llfort.type_idx !33
  %"sub_$A[][]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A[]11", i64 %indvars.iv53), !llfort.type_idx !33
  %"sub_$A[][]_fetch.46" = load float, ptr %"sub_$A[][]12", align 1, !tbaa !51, !llfort.type_idx !37
  %"sub_$B[]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv49), !llfort.type_idx !19
  %"sub_$B[][]16" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B[]15", i64 %indvars.iv53), !llfort.type_idx !19
  %"sub_$B[][]_fetch.49" = load float, ptr %"sub_$B[][]16", align 1, !tbaa !47, !llfort.type_idx !38
  %sub.1 = fsub reassoc ninf nsz arcp contract afn float %"sub_$A[][]_fetch.46", %"sub_$B[][]_fetch.49"
  %"sub_$C[]19" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40000, ptr nonnull elementtype(float) %"sub_$C", i64 %indvars.iv49), !llfort.type_idx !28
  %"sub_$C[][]20" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$C[]19", i64 %indvars.iv53), !llfort.type_idx !28
  store float %sub.1, ptr %"sub_$C[][]20", align 1, !tbaa !49
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next50, %wide.trip.count47
  br i1 %exitcond52, label %do.end_do40.loopexit, label %do.body39

do.end_do40.loopexit:                             ; preds = %do.body39
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next54, %wide.trip.count47
  br i1 %exitcond56, label %do.end_do36.loopexit, label %do.body35

do.end_do36.loopexit:                             ; preds = %do.end_do40.loopexit
  br label %do.end_do36

do.end_do36:                                      ; preds = %do.end_do36.loopexit, %alloca_1
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #4

!0 = !{i64 2}
!1 = !{i64 5}
!2 = !{i64 10}
!3 = !{!4, !4, i64 0}
!4 = !{!"ifx$unique_sym$3", !5, i64 0}
!5 = !{!"Fortran Data Symbol", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$MAIN__"}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$4", !5, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$5", !5, i64 0}
!12 = !{!13}
!13 = distinct !{!13, !14, !"sub_: %sub_$A"}
!14 = distinct !{!14, !"sub_"}
!15 = !{!16}
!16 = distinct !{!16, !14, !"sub_: %sub_$B"}
!17 = !{!18}
!18 = distinct !{!18, !14, !"sub_: %sub_$C"}
!19 = !{i64 69}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$10$0$1", !22, i64 0}
!22 = !{!"Fortran Data Symbol", !23, i64 0}
!23 = !{!"Generic Fortran Symbol", !24, i64 0}
!24 = !{!"ifx$root$2$sub_$0$1"}
!25 = !{!13, !18, !26}
!26 = distinct !{!26, !14, !"sub_: %sub_$SIZE"}
!27 = !{i64 91}
!28 = !{i64 71}
!29 = !{!30, !30, i64 0}
!30 = !{!"ifx$unique_sym$11$0$1", !22, i64 0}
!31 = !{!13, !16, !26}
!32 = !{i64 92}
!33 = !{i64 67}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$12$0$1", !22, i64 0}
!36 = !{!16, !18, !26}
!37 = !{i64 95}
!38 = !{i64 96}
!39 = !{i32 97}
!40 = !{i32 98}
!41 = !{i32 81}
!42 = !{!43, !43, i64 0}
!43 = !{!"ifx$unique_sym$7", !44, i64 0}
!44 = !{!"Fortran Data Symbol", !45, i64 0}
!45 = !{!"Generic Fortran Symbol", !46, i64 0}
!46 = !{!"ifx$root$2$sub_"}
!47 = !{!48, !48, i64 0}
!48 = !{!"ifx$unique_sym$10", !44, i64 0}
!49 = !{!50, !50, i64 0}
!50 = !{!"ifx$unique_sym$11", !44, i64 0}
!51 = !{!52, !52, i64 0}
!52 = !{!"ifx$unique_sym$12", !44, i64 0}

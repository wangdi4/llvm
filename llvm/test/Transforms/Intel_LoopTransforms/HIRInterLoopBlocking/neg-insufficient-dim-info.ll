; RUN: opt -passes="hir-opt-predicate,hir-inter-loop-blocking" -print-after=hir-inter-loop-blocking < %s -disable-output 2>&1 | FileCheck %s

; Verify that HIRInterLoopBlocking is not excersized.
; Only i1, i2, i3 loops are present. No further loops i4, i5 and so on.
; A representative def ref does not have dimension information for all spatial loop levels.
; In this example, spatial loop levels are i1, i2, and i3.
; The representative def ref has only two dimensions, which is less than 3.

; |   |   |   (%"rprj3_$Y1.priv")[0][2 * i3 + -1 * trunc.i64.i32(%"rprj3_$D1.0.val") + 3] =

; CHECK: Function: rprj3_.DIR.OMP.PARALLEL.LOOP.2200.split205
;
; CHECK:  DO i1 = 0,
; CHECK:     DO i2 = 0,
; CHECK:        DO i3 = 0,
; CHECK-NOT:      DO i4 =
; CHECK:          (%"rprj3_$Y1.priv")[0][2 * i3 + -1 * trunc.i64.i32(%"rprj3_$D1.0.val") + 3] =

; ModuleID = 'test.ll'
source_filename = "test.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, ptr }

@.kmpc_loc.0.0 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.13 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_init_4(ptr nocapture readonly, i32, i32, ptr nocapture, ptr nocapture, ptr nocapture, ptr nocapture, i32, i32) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(ptr nocapture readonly, i32) local_unnamed_addr #1

; Function Attrs: nofree nounwind uwtable
define hidden void @rprj3_.DIR.OMP.PARALLEL.LOOP.2200.split205(ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr noalias nocapture writeonly %"rprj3_$S", i64 %"rprj3_$S.array.elements_fetch.17", ptr noalias nocapture readonly %"rprj3_$R", i64 %"rprj3_$R.array.elements_fetch.18", i64 %"rprj3_$D1.0.val", i64 %"rprj3_$M1J.0.val", i64 %"rprj3_$D2.0.val", i64 %"rprj3_$M2J.0.val", i64 %"rprj3_$D3.0.val", i64 %omp.pdo.norm.lb.val.zext, i64 %"var$11.val", i64 %"var$12.val", i64 %"var$15.val", i64 %"var$16.val", i64 %omp.pdo.norm.ub.0.val) #2 {
DIR.OMP.PARALLEL.LOOP.2:
  %0 = trunc i64 %"rprj3_$M2J.0.val" to i32
  %1 = trunc i64 %"rprj3_$M1J.0.val" to i32
  %2 = trunc i64 %omp.pdo.norm.ub.0.val to i32
  %3 = trunc i64 %"rprj3_$D3.0.val" to i32
  %4 = trunc i64 %"rprj3_$D2.0.val" to i32
  %5 = trunc i64 %"rprj3_$D1.0.val" to i32
  %"rprj3_$X1.priv" = alloca [4099 x double], align 64
  %"rprj3_$Y1.priv" = alloca [4099 x double], align 64
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 0, ptr %is.last, align 4
  %rel.23.not203 = icmp sgt i32 0, %2
  br i1 %rel.23.not203, label %DIR.OMP.END.PARALLEL.LOOP.5.loopexit, label %omp.pdo.body29.lr.ph

omp.pdo.body29:                                   ; preds = %omp.pdo.body29.preheader1, %bb8.loopexit
  %indvars.iv14 = phi i64 [ %13, %omp.pdo.body29.preheader1 ], [ %indvars.iv.next15, %bb8.loopexit ]
  %6 = trunc i64 %indvars.iv14 to i32
  %7 = add i32 %6, 2
  %mul.5 = shl nuw nsw i32 %7, 1
  %sub.10 = sub nsw i32 %mul.5, %3
  %int_sext18 = sext i32 %sub.10 to i64
  %"rprj3_$R[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %"var$12.val", ptr elementtype(double) %"rprj3_$R", i64 %int_sext18)
  %sub.18 = add nsw i32 %sub.10, -1
  %int_sext27 = sext i32 %sub.18 to i64
  %"rprj3_$R[]28" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %"var$12.val", ptr elementtype(double) %"rprj3_$R", i64 %int_sext27)
  %add.14 = add nsw i32 %sub.10, 1
  %int_sext33 = sext i32 %add.14 to i64
  %"rprj3_$R[]34" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %"var$12.val", ptr elementtype(double) %"rprj3_$R", i64 %int_sext33)
  %int_sext146 = zext i32 %7 to i64
  %"rprj3_$S[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %"var$16.val", ptr elementtype(double) %"rprj3_$S", i64 %int_sext146)
  br label %bb7

bb7:                                              ; preds = %bb16, %omp.pdo.body29
  %indvars.iv9 = phi i64 [ 2, %omp.pdo.body29 ], [ %indvars.iv.next10, %bb16 ]
  %indvars.iv9.tr = trunc i64 %indvars.iv9 to i32
  %8 = shl i32 %indvars.iv9.tr, 1
  %sub.12 = sub nsw i32 %8, %4
  br i1 %rel.25, label %bb16, label %bb11.preheader

bb11.preheader:                                   ; preds = %bb7
  %sub.15 = add nsw i32 %sub.12, -1
  %int_sext17 = sext i32 %sub.15 to i64, !llfort.type_idx !1
  %"rprj3_$R[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]", i64 %int_sext17), !llfort.type_idx !2
  %add.11 = add nsw i32 %sub.12, 1
  %int_sext20 = sext i32 %add.11 to i64, !llfort.type_idx !1
  %"rprj3_$R[][]23" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]", i64 %int_sext20), !llfort.type_idx !3
  %int_sext26 = sext i32 %sub.12 to i64, !llfort.type_idx !1
  %"rprj3_$R[][]29" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]28", i64 %int_sext26), !llfort.type_idx !4
  %"rprj3_$R[][]35" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]34", i64 %int_sext26), !llfort.type_idx !5
  %"rprj3_$R[][]42" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]28", i64 %int_sext17), !llfort.type_idx !6
  %"rprj3_$R[][]48" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]34", i64 %int_sext17), !llfort.type_idx !7
  %"rprj3_$R[][]54" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]28", i64 %int_sext20), !llfort.type_idx !8
  %"rprj3_$R[][]60" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr elementtype(double) %"rprj3_$R[]34", i64 %int_sext20), !llfort.type_idx !9
  br label %bb11

bb11:                                             ; preds = %bb11, %bb11.preheader
  %indvars.iv = phi i64 [ 2, %bb11.preheader ], [ %indvars.iv.next, %bb11 ]
  %indvars.iv.tr = trunc i64 %indvars.iv to i32
  %9 = shl i32 %indvars.iv.tr, 1
  %sub.14 = add i32 %9, %11
  %int_sext16 = sext i32 %sub.14 to i64, !llfort.type_idx !1
  %"rprj3_$R[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]", i64 %int_sext16), !llfort.type_idx !10
  %"rprj3_$R[][][]_fetch.46" = load double, ptr %"rprj3_$R[][][]", align 1, !tbaa !11, !alias.scope !16, !noalias !19, !llvm.access.group !61, !llfort.type_idx !10
  %"rprj3_$R[][][]24" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]23", i64 %int_sext16), !llfort.type_idx !62
  %"rprj3_$R[][][]_fetch.53" = load double, ptr %"rprj3_$R[][][]24", align 1, !tbaa !11, !alias.scope !63, !noalias !19, !llvm.access.group !61, !llfort.type_idx !62
  %add.12 = fadd fast double %"rprj3_$R[][][]_fetch.53", %"rprj3_$R[][][]_fetch.46"
  %"rprj3_$R[][][]30" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]29", i64 %int_sext16), !llfort.type_idx !65
  %"rprj3_$R[][][]_fetch.60" = load double, ptr %"rprj3_$R[][][]30", align 1, !tbaa !11, !alias.scope !66, !noalias !19, !llvm.access.group !61, !llfort.type_idx !65
  %add.13 = fadd fast double %add.12, %"rprj3_$R[][][]_fetch.60"
  %"rprj3_$R[][][]36" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]35", i64 %int_sext16), !llfort.type_idx !68
  %"rprj3_$R[][][]_fetch.67" = load double, ptr %"rprj3_$R[][][]36", align 1, !tbaa !11, !alias.scope !69, !noalias !19, !llvm.access.group !61, !llfort.type_idx !68
  %add.15 = fadd fast double %add.13, %"rprj3_$R[][][]_fetch.67"
  %"rprj3_$X1[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$X1$", i64 %int_sext16), !llfort.type_idx !71
  store double %add.15, ptr %"rprj3_$X1[]", align 8, !tbaa !72, !alias.scope !74, !noalias !75, !llvm.access.group !61
  %"rprj3_$R[][][]43" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]42", i64 %int_sext16), !llfort.type_idx !111
  %"rprj3_$R[][][]_fetch.75" = load double, ptr %"rprj3_$R[][][]43", align 1, !tbaa !11, !alias.scope !112, !noalias !19, !llvm.access.group !61, !llfort.type_idx !111
  %"rprj3_$R[][][]49" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]48", i64 %int_sext16), !llfort.type_idx !113
  %"rprj3_$R[][][]_fetch.82" = load double, ptr %"rprj3_$R[][][]49", align 1, !tbaa !11, !alias.scope !114, !noalias !19, !llvm.access.group !61, !llfort.type_idx !113
  %add.17 = fadd fast double %"rprj3_$R[][][]_fetch.82", %"rprj3_$R[][][]_fetch.75"
  %"rprj3_$R[][][]55" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]54", i64 %int_sext16), !llfort.type_idx !115
  %"rprj3_$R[][][]_fetch.89" = load double, ptr %"rprj3_$R[][][]55", align 1, !tbaa !11, !alias.scope !116, !noalias !19, !llvm.access.group !61, !llfort.type_idx !115
  %add.19 = fadd fast double %add.17, %"rprj3_$R[][][]_fetch.89"
  %"rprj3_$R[][][]61" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]60", i64 %int_sext16), !llfort.type_idx !117
  %"rprj3_$R[][][]_fetch.96" = load double, ptr %"rprj3_$R[][][]61", align 1, !tbaa !11, !alias.scope !118, !noalias !19, !llvm.access.group !61, !llfort.type_idx !117
  %add.22 = fadd fast double %add.19, %"rprj3_$R[][][]_fetch.96"
  %"rprj3_$Y1[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$Y1$", i64 %int_sext16), !llfort.type_idx !119
  store double %add.22, ptr %"rprj3_$Y1[]", align 8, !tbaa !120, !alias.scope !122, !noalias !123, !llvm.access.group !61
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %bb12, label %bb11

bb12:                                             ; preds = %bb11
  br i1 %rel.27, label %bb16, label %bb15.preheader

bb15.preheader:                                   ; preds = %bb12
  %"rprj3_$R[][]67" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]28", i64 %int_sext17), !llfort.type_idx !124
  %"rprj3_$R[][]73" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]34", i64 %int_sext17), !llfort.type_idx !125
  %"rprj3_$R[][]79" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]28", i64 %int_sext20), !llfort.type_idx !126
  %"rprj3_$R[][]85" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]34", i64 %int_sext20), !llfort.type_idx !127
  %"rprj3_$R[][]91" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]", i64 %int_sext17), !llfort.type_idx !128
  %"rprj3_$R[][]97" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]", i64 %int_sext20), !llfort.type_idx !129
  %"rprj3_$R[][]103" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]28", i64 %int_sext26), !llfort.type_idx !130
  %"rprj3_$R[][]109" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]34", i64 %int_sext26), !llfort.type_idx !131
  %"rprj3_$R[][]118" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$11.val", ptr nonnull elementtype(double) %"rprj3_$R[]", i64 %int_sext26), !llfort.type_idx !132
  %"rprj3_$S[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$15.val", ptr elementtype(double) %"rprj3_$S[]", i64 %indvars.iv9), !llfort.type_idx !133
  br label %bb15

bb15:                                             ; preds = %bb15, %bb15.preheader
  %indvars.iv4 = phi i64 [ 2, %bb15.preheader ], [ %indvars.iv.next5, %bb15 ]
  %indvars.iv4.tr = trunc i64 %indvars.iv4 to i32
  %10 = shl i32 %indvars.iv4.tr, 1
  %sub.31 = sub nsw i32 %10, %5
  %int_sext63 = sext i32 %sub.31 to i64, !llfort.type_idx !1
  %"rprj3_$R[][][]68" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]67", i64 %int_sext63), !llfort.type_idx !134
  %"rprj3_$R[][][]_fetch.111" = load double, ptr %"rprj3_$R[][][]68", align 1, !tbaa !11, !alias.scope !135, !noalias !19, !llvm.access.group !61, !llfort.type_idx !134
  %"rprj3_$R[][][]74" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]73", i64 %int_sext63), !llfort.type_idx !136
  %"rprj3_$R[][][]_fetch.118" = load double, ptr %"rprj3_$R[][][]74", align 1, !tbaa !11, !alias.scope !137, !noalias !19, !llvm.access.group !61, !llfort.type_idx !136
  %"rprj3_$R[][][]80" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]79", i64 %int_sext63), !llfort.type_idx !138
  %"rprj3_$R[][][]_fetch.125" = load double, ptr %"rprj3_$R[][][]80", align 1, !tbaa !11, !alias.scope !139, !noalias !19, !llvm.access.group !61, !llfort.type_idx !138
  %"rprj3_$R[][][]86" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]85", i64 %int_sext63), !llfort.type_idx !140
  %"rprj3_$R[][][]_fetch.132" = load double, ptr %"rprj3_$R[][][]86", align 1, !tbaa !11, !alias.scope !141, !noalias !19, !llvm.access.group !61, !llfort.type_idx !140
  %"rprj3_$R[][][]92" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]91", i64 %int_sext63), !llfort.type_idx !142
  %"rprj3_$R[][][]_fetch.139" = load double, ptr %"rprj3_$R[][][]92", align 1, !tbaa !11, !alias.scope !143, !noalias !19, !llvm.access.group !61, !llfort.type_idx !142
  %"rprj3_$R[][][]98" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]97", i64 %int_sext63), !llfort.type_idx !144
  %"rprj3_$R[][][]_fetch.146" = load double, ptr %"rprj3_$R[][][]98", align 1, !tbaa !11, !alias.scope !145, !noalias !19, !llvm.access.group !61, !llfort.type_idx !144
  %"rprj3_$R[][][]104" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]103", i64 %int_sext63), !llfort.type_idx !146
  %"rprj3_$R[][][]_fetch.153" = load double, ptr %"rprj3_$R[][][]104", align 1, !tbaa !11, !alias.scope !147, !noalias !19, !llvm.access.group !61, !llfort.type_idx !146
  %"rprj3_$R[][][]110" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]109", i64 %int_sext63), !llfort.type_idx !148
  %"rprj3_$R[][][]_fetch.160" = load double, ptr %"rprj3_$R[][][]110", align 1, !tbaa !11, !alias.scope !149, !noalias !19, !llvm.access.group !61, !llfort.type_idx !148
  %"rprj3_$R[][][]119" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]118", i64 %int_sext63), !llfort.type_idx !150
  %"rprj3_$R[][][]_fetch.167" = load double, ptr %"rprj3_$R[][][]119", align 1, !tbaa !11, !alias.scope !151, !noalias !19, !llvm.access.group !61, !llfort.type_idx !150
  %mul.13 = fmul fast double %"rprj3_$R[][][]_fetch.167", 5.000000e-01
  %sub.38 = add nsw i32 %sub.31, -1
  %int_sext120 = sext i32 %sub.38 to i64, !llfort.type_idx !1
  %"rprj3_$R[][][]125" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]118", i64 %int_sext120), !llfort.type_idx !152
  %"rprj3_$R[][][]_fetch.174" = load double, ptr %"rprj3_$R[][][]125", align 1, !tbaa !11, !alias.scope !153, !noalias !19, !llvm.access.group !61, !llfort.type_idx !152
  %add.36 = add nsw i32 %sub.31, 1
  %int_sext126 = sext i32 %add.36 to i64, !llfort.type_idx !1
  %"rprj3_$R[][][]131" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$R[][]118", i64 %int_sext126), !llfort.type_idx !154
  %"rprj3_$R[][][]_fetch.181" = load double, ptr %"rprj3_$R[][][]131", align 1, !tbaa !11, !alias.scope !155, !noalias !19, !llvm.access.group !61, !llfort.type_idx !154
  %add.37 = fadd fast double %"rprj3_$R[][][]_fetch.146", %"rprj3_$R[][][]_fetch.139"
  %add.32 = fadd fast double %add.37, %"rprj3_$R[][][]_fetch.153"
  %add.33 = fadd fast double %add.32, %"rprj3_$R[][][]_fetch.160"
  %add.35 = fadd fast double %add.33, %"rprj3_$R[][][]_fetch.174"
  %add.38 = fadd fast double %add.35, %"rprj3_$R[][][]_fetch.181"
  %mul.14 = fmul fast double %add.38, 2.500000e-01
  %add.39 = fadd fast double %mul.14, %mul.13
  %"rprj3_$X1[]134" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$X1$", i64 %int_sext120), !llfort.type_idx !156
  %"rprj3_$X1[]_fetch.184" = load double, ptr %"rprj3_$X1[]134", align 8, !tbaa !72, !alias.scope !157, !noalias !158, !llvm.access.group !61, !llfort.type_idx !159
  %"rprj3_$X1[]137" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$X1$", i64 %int_sext126), !llfort.type_idx !160
  %"rprj3_$X1[]_fetch.186" = load double, ptr %"rprj3_$X1[]137", align 8, !tbaa !72, !alias.scope !161, !noalias !158, !llvm.access.group !61, !llfort.type_idx !162
  %add.41 = fadd fast double %"rprj3_$R[][][]_fetch.118", %"rprj3_$R[][][]_fetch.111"
  %add.25 = fadd fast double %add.41, %"rprj3_$R[][][]_fetch.125"
  %add.27 = fadd fast double %add.25, %"rprj3_$R[][][]_fetch.132"
  %add.30 = fadd fast double %add.27, %"rprj3_$X1[]_fetch.184"
  %add.42 = fadd fast double %add.30, %"rprj3_$X1[]_fetch.186"
  %mul.15 = fmul fast double %add.42, 1.250000e-01
  %add.43 = fadd fast double %add.39, %mul.15
  %"rprj3_$Y1[]140" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$Y1$", i64 %int_sext120), !llfort.type_idx !163
  %"rprj3_$Y1[]_fetch.189" = load double, ptr %"rprj3_$Y1[]140", align 8, !tbaa !120, !alias.scope !164, !noalias !165, !llvm.access.group !61, !llfort.type_idx !166
  %"rprj3_$Y1[]143" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"(ptr)rprj3_$Y1$", i64 %int_sext126), !llfort.type_idx !167
  %"rprj3_$Y1[]_fetch.191" = load double, ptr %"rprj3_$Y1[]143", align 8, !tbaa !120, !alias.scope !168, !noalias !165, !llvm.access.group !61, !llfort.type_idx !169
  %add.45 = fadd fast double %"rprj3_$Y1[]_fetch.191", %"rprj3_$Y1[]_fetch.189"
  %mul.16 = fmul fast double %add.45, 6.250000e-02
  %add.46 = fadd fast double %add.43, %mul.16
  %"rprj3_$S[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"rprj3_$S[][]", i64 %indvars.iv4), !llfort.type_idx !170
  store double %add.46, ptr %"rprj3_$S[][][]", align 1, !tbaa !171, !alias.scope !173, !noalias !174, !llvm.access.group !61
  %indvars.iv.next5 = add nuw nsw i64 %indvars.iv4, 1
  %exitcond8.not = icmp eq i64 %indvars.iv.next5, %wide.trip.count7
  br i1 %exitcond8.not, label %bb16.loopexit, label %bb15

bb16.loopexit:                                    ; preds = %bb15
  br label %bb16

bb16:                                             ; preds = %bb16.loopexit, %bb12, %bb7
  %indvars.iv.next10 = add nuw nsw i64 %indvars.iv9, 1
  %exitcond13.not = icmp eq i64 %indvars.iv.next10, %wide.trip.count12
  br i1 %exitcond13.not, label %bb8.loopexit, label %bb7

bb8.loopexit:                                     ; preds = %bb16
  %indvars.iv.next15 = add nuw nsw i64 %indvars.iv14, 1
  %exitcond18 = icmp eq i64 %indvars.iv.next15, %wide.trip.count17
  br i1 %exitcond18, label %loop.region.exit.loopexit, label %omp.pdo.body29, !llvm.loop !175

omp.pdo.body29.lr.ph:                             ; preds = %DIR.OMP.PARALLEL.LOOP.2
  %my.tid = load i32, ptr %tid, align 4
  store i32 0, ptr %lower.bnd, align 4
  store i32 %2, ptr %upper.bnd, align 4
  store i32 1, ptr %stride, align 4
  call void @__kmpc_for_static_init_4(ptr nonnull @.kmpc_loc.0.0, i32 %my.tid, i32 34, ptr nonnull %is.last, ptr nonnull %lower.bnd, ptr nonnull %upper.bnd, ptr nonnull %stride, i32 1, i32 1)
  %lb.new = load i32, ptr %lower.bnd, align 4, !range !182
  %ub.new = load i32, ptr %upper.bnd, align 4, !range !182
  %omp.ztt.not = icmp ugt i32 %lb.new, %ub.new
  br i1 %omp.ztt.not, label %loop.region.exit, label %omp.pdo.body29.preheader

omp.pdo.body29.preheader:                         ; preds = %omp.pdo.body29.lr.ph
  %rel.24 = icmp slt i32 %0, 3
  %rel.25 = icmp slt i32 %1, 2
  %rel.27 = icmp slt i32 %1, 3
  %11 = xor i32 %5, -1
  %"(ptr)rprj3_$X1$" = getelementptr inbounds [4099 x double], ptr %"rprj3_$X1.priv", i64 0, i64 0
  %"(ptr)rprj3_$Y1$" = getelementptr inbounds [4099 x double], ptr %"rprj3_$Y1.priv", i64 0, i64 0
  br i1 %rel.24, label %loop.region.exit, label %omp.pdo.body29.preheader1

omp.pdo.body29.preheader1:                        ; preds = %omp.pdo.body29.preheader
  %12 = add i64 %"rprj3_$M1J.0.val", 1
  %13 = zext i32 %lb.new to i64
  %14 = add nuw nsw i32 %ub.new, 1
  %wide.trip.count17 = zext i32 %14 to i64
  %wide.trip.count12 = and i64 %"rprj3_$M2J.0.val", 4294967295
  %wide.trip.count = and i64 %12, 4294967295
  %wide.trip.count7 = and i64 %"rprj3_$M1J.0.val", 4294967295
  br label %omp.pdo.body29

loop.region.exit.loopexit:                        ; preds = %bb8.loopexit
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %loop.region.exit.loopexit, %omp.pdo.body29.preheader, %omp.pdo.body29.lr.ph
  tail call void @__kmpc_for_static_fini(ptr nonnull @.kmpc_loc.0.0.13, i32 %my.tid)
  br label %DIR.OMP.END.PARALLEL.LOOP.5.loopexit

DIR.OMP.END.PARALLEL.LOOP.5.loopexit:             ; preds = %loop.region.exit, %DIR.OMP.PARALLEL.LOOP.2
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nounwind }
attributes #2 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "processed-by-vpo" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}
!nvvm.annotations = !{}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{i64 3}
!2 = !{i64 110}
!3 = !{i64 113}
!4 = !{i64 116}
!5 = !{i64 119}
!6 = !{i64 125}
!7 = !{i64 128}
!8 = !{i64 131}
!9 = !{i64 134}
!10 = !{i64 111}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$17", !13, i64 0}
!13 = !{!"Fortran Data Symbol", !14, i64 0}
!14 = !{!"Generic Fortran Symbol", !15, i64 0}
!15 = !{!"ifx$root$1$rprj3_"}
!16 = !{!17}
!17 = distinct !{!17, !18, !"OMPAliasScope"}
!18 = distinct !{!18, !"OMPDomain"}
!19 = !{!20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60}
!20 = distinct !{!20, !18, !"OMPAliasScope"}
!21 = distinct !{!21, !18, !"OMPAliasScope"}
!22 = distinct !{!22, !18, !"OMPAliasScope"}
!23 = distinct !{!23, !18, !"OMPAliasScope"}
!24 = distinct !{!24, !18, !"OMPAliasScope"}
!25 = distinct !{!25, !18, !"OMPAliasScope"}
!26 = distinct !{!26, !18, !"OMPAliasScope"}
!27 = distinct !{!27, !18, !"OMPAliasScope"}
!28 = distinct !{!28, !18, !"OMPAliasScope"}
!29 = distinct !{!29, !18, !"OMPAliasScope"}
!30 = distinct !{!30, !18, !"OMPAliasScope"}
!31 = distinct !{!31, !18, !"OMPAliasScope"}
!32 = distinct !{!32, !18, !"OMPAliasScope"}
!33 = distinct !{!33, !18, !"OMPAliasScope"}
!34 = distinct !{!34, !18, !"OMPAliasScope"}
!35 = distinct !{!35, !18, !"OMPAliasScope"}
!36 = distinct !{!36, !18, !"OMPAliasScope"}
!37 = distinct !{!37, !18, !"OMPAliasScope"}
!38 = distinct !{!38, !18, !"OMPAliasScope"}
!39 = distinct !{!39, !18, !"OMPAliasScope"}
!40 = distinct !{!40, !18, !"OMPAliasScope"}
!41 = distinct !{!41, !18, !"OMPAliasScope"}
!42 = distinct !{!42, !18, !"OMPAliasScope"}
!43 = distinct !{!43, !18, !"OMPAliasScope"}
!44 = distinct !{!44, !18, !"OMPAliasScope"}
!45 = distinct !{!45, !18, !"OMPAliasScope"}
!46 = distinct !{!46, !18, !"OMPAliasScope"}
!47 = distinct !{!47, !18, !"OMPAliasScope"}
!48 = distinct !{!48, !18, !"OMPAliasScope"}
!49 = distinct !{!49, !18, !"OMPAliasScope"}
!50 = distinct !{!50, !18, !"OMPAliasScope"}
!51 = distinct !{!51, !18, !"OMPAliasScope"}
!52 = distinct !{!52, !18, !"OMPAliasScope"}
!53 = distinct !{!53, !18, !"OMPAliasScope"}
!54 = distinct !{!54, !18, !"OMPAliasScope"}
!55 = distinct !{!55, !18, !"OMPAliasScope"}
!56 = distinct !{!56, !18, !"OMPAliasScope"}
!57 = distinct !{!57, !18, !"OMPAliasScope"}
!58 = distinct !{!58, !18, !"OMPAliasScope"}
!59 = distinct !{!59, !18, !"OMPAliasScope"}
!60 = distinct !{!60, !18, !"OMPAliasScope"}
!61 = distinct !{}
!62 = !{i64 114}
!63 = !{!64}
!64 = distinct !{!64, !18, !"OMPAliasScope"}
!65 = !{i64 117}
!66 = !{!67}
!67 = distinct !{!67, !18, !"OMPAliasScope"}
!68 = !{i64 120}
!69 = !{!70}
!70 = distinct !{!70, !18, !"OMPAliasScope"}
!71 = !{i64 122}
!72 = !{!73, !73, i64 0}
!73 = !{!"ifx$unique_sym$18", !13, i64 0}
!74 = !{!54, !55}
!75 = !{!76, !77, !20, !21, !78, !22, !79, !23, !80, !24, !25, !26, !27, !28, !29, !81, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !82, !40, !41, !42, !43, !44, !45, !46, !47, !48, !83, !49, !50, !51, !52, !53, !84, !85, !17, !64, !67, !70, !86, !87, !88, !89, !90, !91, !56, !57, !92, !93, !94, !95, !96, !97, !58, !98, !99, !100, !101, !102, !103, !59, !104, !105, !106, !107, !108, !109, !110, !60}
!76 = distinct !{!76, !18, !"OMPAliasScope"}
!77 = distinct !{!77, !18, !"OMPAliasScope"}
!78 = distinct !{!78, !18, !"OMPAliasScope"}
!79 = distinct !{!79, !18, !"OMPAliasScope"}
!80 = distinct !{!80, !18, !"OMPAliasScope"}
!81 = distinct !{!81, !18, !"OMPAliasScope"}
!82 = distinct !{!82, !18, !"OMPAliasScope"}
!83 = distinct !{!83, !18, !"OMPAliasScope"}
!84 = distinct !{!84, !18, !"OMPAliasScope"}
!85 = distinct !{!85, !18, !"OMPAliasScope"}
!86 = distinct !{!86, !18, !"OMPAliasScope"}
!87 = distinct !{!87, !18, !"OMPAliasScope"}
!88 = distinct !{!88, !18, !"OMPAliasScope"}
!89 = distinct !{!89, !18, !"OMPAliasScope"}
!90 = distinct !{!90, !18, !"OMPAliasScope"}
!91 = distinct !{!91, !18, !"OMPAliasScope"}
!92 = distinct !{!92, !18, !"OMPAliasScope"}
!93 = distinct !{!93, !18, !"OMPAliasScope"}
!94 = distinct !{!94, !18, !"OMPAliasScope"}
!95 = distinct !{!95, !18, !"OMPAliasScope"}
!96 = distinct !{!96, !18, !"OMPAliasScope"}
!97 = distinct !{!97, !18, !"OMPAliasScope"}
!98 = distinct !{!98, !18, !"OMPAliasScope"}
!99 = distinct !{!99, !18, !"OMPAliasScope"}
!100 = distinct !{!100, !18, !"OMPAliasScope"}
!101 = distinct !{!101, !18, !"OMPAliasScope"}
!102 = distinct !{!102, !18, !"OMPAliasScope"}
!103 = distinct !{!103, !18, !"OMPAliasScope"}
!104 = distinct !{!104, !18, !"OMPAliasScope"}
!105 = distinct !{!105, !18, !"OMPAliasScope"}
!106 = distinct !{!106, !18, !"OMPAliasScope"}
!107 = distinct !{!107, !18, !"OMPAliasScope"}
!108 = distinct !{!108, !18, !"OMPAliasScope"}
!109 = distinct !{!109, !18, !"OMPAliasScope"}
!110 = distinct !{!110, !18, !"OMPAliasScope"}
!111 = !{i64 126}
!112 = !{!88}
!113 = !{i64 129}
!114 = !{!89}
!115 = !{i64 132}
!116 = !{!90}
!117 = !{i64 135}
!118 = !{!91}
!119 = !{i64 137}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$19", !13, i64 0}
!122 = !{!56, !57}
!123 = !{!76, !77, !20, !21, !78, !22, !79, !23, !80, !24, !25, !26, !27, !28, !29, !81, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !82, !40, !41, !42, !43, !44, !45, !46, !47, !48, !83, !49, !50, !51, !52, !53, !84, !85, !17, !64, !67, !70, !54, !55, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !58, !98, !99, !100, !101, !102, !103, !59, !104, !105, !106, !107, !108, !109, !110, !60}
!124 = !{i64 140}
!125 = !{i64 143}
!126 = !{i64 146}
!127 = !{i64 149}
!128 = !{i64 152}
!129 = !{i64 155}
!130 = !{i64 158}
!131 = !{i64 161}
!132 = !{i64 164}
!133 = !{i64 185}
!134 = !{i64 141}
!135 = !{!94}
!136 = !{i64 144}
!137 = !{!95}
!138 = !{i64 147}
!139 = !{!96}
!140 = !{i64 150}
!141 = !{!97}
!142 = !{i64 153}
!143 = !{!100}
!144 = !{i64 156}
!145 = !{!101}
!146 = !{i64 159}
!147 = !{!102}
!148 = !{i64 162}
!149 = !{!103}
!150 = !{i64 165}
!151 = !{!106}
!152 = !{i64 168}
!153 = !{!107}
!154 = !{i64 171}
!155 = !{!108}
!156 = !{i64 173}
!157 = !{!55}
!158 = !{!20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !56, !57, !58, !59, !60}
!159 = !{i64 174}
!160 = !{i64 176}
!161 = !{!54}
!162 = !{i64 177}
!163 = !{i64 179}
!164 = !{!57}
!165 = !{!20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !58, !59, !60}
!166 = !{i64 180}
!167 = !{i64 182}
!168 = !{!56}
!169 = !{i64 183}
!170 = !{i64 186}
!171 = !{!172, !172, i64 0}
!172 = !{!"ifx$unique_sym$22", !13, i64 0}
!173 = !{!60}
!174 = !{!76, !77, !20, !21, !78, !22, !79, !23, !80, !24, !25, !26, !27, !28, !29, !81, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !82, !40, !41, !42, !43, !44, !45, !46, !47, !48, !83, !49, !50, !51, !52, !53, !84, !85, !17, !64, !67, !70, !54, !55, !86, !87, !88, !89, !90, !91, !56, !57, !92, !93, !94, !95, !96, !97, !58, !98, !99, !100, !101, !102, !103, !59, !104, !105, !106, !107, !108, !109, !110}
!175 = distinct !{!175, !176, !180, !181}
!176 = distinct !{!"intel.optreport", !178}
!178 = !{!"intel.optreport.remarks", !179}
!179 = !{!"intel.optreport.remark", i32 0, !"OpenMP: Outlined parallel loop"}
!180 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!181 = !{!"llvm.loop.parallel_accesses", !61}
!182 = !{i32 0, i32 2147483647}

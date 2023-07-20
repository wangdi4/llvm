 
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>,hir-cg" -hir-details-dims -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

;  For certain SPREAD function, FE may swap the strides. 
;  The array is not contiguous. 
;  HIR - input and output are the same.  No memcpy is generated
;

; CHECK:        + DO i1 = 0, 22, 1   <DO_LOOP>
; CHECK:        |   + DO i2 = 0, 16, 1   <DO_LOOP>
; CHECK:        |   |   @llvm.for.cpystr.i64.i64.i64(&((%temp_array1.sub)[0:i1:7(i8:0)][0:i2:161(i8:0)]),  7,  &((%"rt$VECTOR$_142.sub")[0:i2:7(i8:0)]),  7,  0,  0);
; CHECK:        |   + END LOOP
; CHECK:        + END LOOP


;Module Before HIR
; ModuleID = 'spread.f90'
source_filename = "spread.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@0 = internal unnamed_addr constant i32 65536, align 4
@1 = internal unnamed_addr constant i32 2, align 4

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %func_result = tail call i32 @for_set_fpe_(ptr nonnull @0) #6, !llfort.type_idx !0
  %func_result2 = tail call i32 @for_set_reentrancy(ptr nonnull @1) #6, !llfort.type_idx !0
  tail call fastcc void @i2c_IP_rt_.1()
  ret void
}

; Function Attrs: nounwind uwtable
define void @i2c_IP_rt_(ptr noalias nocapture readonly dereferenceable(4) %J1, ptr noalias nocapture readonly dereferenceable(4) %J2, ptr noalias nocapture readonly dereferenceable(4) %J3, ptr noalias nocapture readonly dereferenceable(4) %J4, ptr noalias nocapture readonly dereferenceable(4) %J5, ptr noalias nocapture readonly dereferenceable(4) %J6, ptr noalias nocapture readonly dereferenceable(4) %J7, ptr noalias nocapture readonly dereferenceable(4) %J8, ptr noalias nocapture readonly dereferenceable(4) %J9, ptr noalias nocapture readonly dereferenceable(4) %J10, ptr noalias nocapture readonly dereferenceable(4) %J11, ptr noalias nocapture readonly dereferenceable(4) %J13, ptr noalias nocapture readonly dereferenceable(4) %J17, ptr noalias nocapture readonly dereferenceable(4) %J23) local_unnamed_addr #0 {
alloca_1:
  %"var$1924156169" = alloca [0 x i8], align 1
  %"var$29" = alloca [2 x i64], align 16, !llfort.type_idx !1
  %"var$31" = alloca [2 x i64], align 16, !llfort.type_idx !2
  %"var$36" = alloca [1 x i64], align 16, !llfort.type_idx !3
  %J17_fetch.6 = load i32, ptr %J17, align 1, !tbaa !4, !llfort.type_idx !9
  %J7_fetch.7 = load i32, ptr %J7, align 1, !tbaa !10, !llfort.type_idx !12
  %J23_fetch.8 = load i32, ptr %J23, align 1, !tbaa !13, !llfort.type_idx !15
  %J8_fetch.9 = load i32, ptr %J8, align 1, !tbaa !16, !llfort.type_idx !18
  %slct.1 = tail call i32 @llvm.smax.i32(i32 %J8_fetch.9, i32 0)
  %slct.2 = tail call i32 @llvm.smax.i32(i32 %J23_fetch.8, i32 0)
  %slct.3 = tail call i32 @llvm.smax.i32(i32 %J17_fetch.6, i32 0)
  %mul.1 = mul i32 %slct.2, %slct.3
  %mul.2 = mul i32 %mul.1, %slct.1
  %0 = zext i32 %mul.2 to i64
  %"rt$MANSWER1$_12" = alloca i8, i64 %0, align 1, !llfort.type_idx !19
  %slct.4 = tail call i32 @llvm.smax.i32(i32 %J7_fetch.7, i32 0)
  %mul.3 = mul nsw i32 %slct.4, %slct.3
  %1 = zext i32 %mul.3 to i64
  %"rt$VECTOR$_14" = alloca i8, i64 %1, align 1, !llfort.type_idx !20
  %slct.15 = zext i32 %slct.1 to i64
  %int_sext39 = sext i32 %J23_fetch.8 to i64, !llfort.type_idx !21
  %mul.8 = mul nsw i64 %slct.15, %int_sext39
  %"$stacksave30" = tail call ptr @llvm.stacksave(), !llfort.type_idx !22
  %slct.6 = zext i32 %slct.4 to i64
  %int_sext32 = sext i32 %J17_fetch.6 to i64, !llfort.type_idx !21
  %slct.9 = tail call i64 @llvm.smax.i64(i64 %int_sext32, i64 0)
  %int_sext34 = trunc i64 %slct.9 to i32, !llfort.type_idx !0
  %mul.4 = shl nuw nsw i64 %slct.9, 1
  %int_sext35 = and i64 %mul.4, 4294967294
  %"var$11" = alloca ptr, i64 %int_sext35, align 8, !llfort.type_idx !22
  %rel.11 = icmp eq i32 %int_sext34, 0
  br i1 %rel.11, label %do.end_do36.thread, label %do.body21.preheader

do.body21.preheader:                              ; preds = %alloca_1
  %J11_fetch.18 = load i32, ptr %J11, align 1, !tbaa !23, !llfort.type_idx !25
  %int_sext6 = sext i32 %J11_fetch.18 to i64, !llfort.type_idx !21
  %rel.12.not170 = icmp slt i32 %J11_fetch.18, 1
  %"(ptr)slct.11$" = inttoptr i64 %int_sext6 to ptr, !llfort.type_idx !22
  %2 = add nsw i64 %int_sext6, 1
  br label %do.body21

do.end_do36.thread:                               ; preds = %alloca_1
  %"var$1924156169.sub" = getelementptr inbounds [0 x i8], ptr %"var$1924156169", i64 0, i64 0
  br label %do.end_do48

do.body21:                                        ; preds = %do.body21.preheader, %loop_exit30
  %indvars.iv = phi i64 [ 2, %do.body21.preheader ], [ %indvars.iv.next, %loop_exit30 ]
  %"var$12.0" = phi i64 [ 0, %do.body21.preheader ], [ %add.2, %loop_exit30 ]
  %"var$9.0" = phi i32 [ 1, %do.body21.preheader ], [ %add.5, %loop_exit30 ]
  %"var$8.0" = phi i64 [ 1, %do.body21.preheader ], [ %add.4, %loop_exit30 ]
  %"var$7.0" = phi i64 [ 0, %do.body21.preheader ], [ %slct.13, %loop_exit30 ]
  %int_zext = trunc i32 %"var$9.0" to i8, !llfort.type_idx !26
  %"var$148" = alloca i8, i64 %int_sext6, align 1, !llfort.type_idx !27
  br i1 %rel.12.not170, label %loop_exit30, label %loop_body29.preheader

loop_body29.preheader:                            ; preds = %do.body21
  br label %loop_body29

loop_body29:                                      ; preds = %loop_body29.preheader, %loop_body29
  %"$loop_ctr9.0171" = phi i64 [ %add.1, %loop_body29 ], [ 1, %loop_body29.preheader ]
  %"var$148[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1, ptr nonnull elementtype(i8) %"var$148", i64 %"$loop_ctr9.0171"), !llfort.type_idx !28
  store i8 %int_zext, ptr %"var$148[]", align 1
  %add.1 = add nuw nsw i64 %"$loop_ctr9.0171", 1
  %exitcond = icmp eq i64 %add.1, %2
  br i1 %exitcond, label %loop_exit30.loopexit, label %loop_body29

loop_exit30.loopexit:                             ; preds = %loop_body29
  br label %loop_exit30

loop_exit30:                                      ; preds = %loop_exit30.loopexit, %do.body21
  %"var$11[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$11", i64 %"var$8.0"), !llfort.type_idx !22
  store ptr %"(ptr)slct.11$", ptr %"var$11[]", align 8, !tbaa !29
  %add.2 = add nuw nsw i64 %"var$12.0", 1
  %add.3 = add nuw nsw i64 %"var$8.0", 1
  %"var$11[]12" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$11", i64 %add.3), !llfort.type_idx !22
  %func_result = call i32 @for_allocate_handle(i64 %int_sext6, ptr nonnull %"var$11[]12", i32 262144, ptr null) #6, !llfort.type_idx !0
  %"var$11[]12_fetch.25" = load ptr, ptr %"var$11[]12", align 8, !tbaa !29, !llfort.type_idx !22
  call void @llvm.for.cpystr.i64.i64.i64(ptr %"var$11[]12_fetch.25", i64 %int_sext6, ptr nonnull %"var$148", i64 %int_sext6, i64 0, i1 false), !llfort.type_idx !30
  %add.4 = add nuw nsw i64 %"var$8.0", 2
  %slct.13 = tail call i64 @llvm.smax.i64(i64 %"var$7.0", i64 %int_sext6)
  %add.5 = add nuw nsw i32 %"var$9.0", 1
  %rel.16.not = icmp sgt i32 %add.5, %int_sext34
  %indvars.iv.next = add i64 %indvars.iv, 1
  br i1 %rel.16.not, label %do.epilog42.preheader, label %do.body21

do.epilog42.preheader:                            ; preds = %loop_exit30
  %add.2.lcssa = phi i64 [ %add.2, %loop_exit30 ]
  %slct.13.lcssa = phi i64 [ %slct.13, %loop_exit30 ]
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %loop_exit30 ]
  %mul.6 = mul nsw i64 %slct.13.lcssa, %add.2.lcssa
  %"var$1014" = alloca i8, i64 %mul.6, align 1, !llfort.type_idx !31
  br label %do.epilog42

do.epilog42:                                      ; preds = %do.epilog42.preheader, %do.epilog42
  %"var$16.0" = phi i64 [ %add.6, %do.epilog42 ], [ 1, %do.epilog42.preheader ]
  %"var$8.1" = phi i64 [ %add.8, %do.epilog42 ], [ 2, %do.epilog42.preheader ]
  %"var$11[]18" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$11", i64 %"var$8.1"), !llfort.type_idx !22
  %"var$11[]18_fetch.38" = load ptr, ptr %"var$11[]18", align 8, !tbaa !29, !llfort.type_idx !20
  %"var$1014[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.13.lcssa, ptr nonnull elementtype(i8) %"var$1014", i64 %"var$16.0"), !llfort.type_idx !32
  %"var$11[]18_fetch.38[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.13.lcssa, ptr elementtype(i8) %"var$11[]18_fetch.38", i64 1), !llfort.type_idx !33
  %sub.6 = add nsw i64 %"var$8.1", -1
  %"var$11[]19" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$11", i64 %sub.6), !llfort.type_idx !22
  %"var$11[]19_fetch.40" = load ptr, ptr %"var$11[]19", align 8, !tbaa !29, !llfort.type_idx !22
  %"(i64)var$11[]19_fetch.40$" = ptrtoint ptr %"var$11[]19_fetch.40" to i64, !llfort.type_idx !21
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"var$1014[]", i64 %slct.13.lcssa, ptr %"var$11[]18_fetch.38[]", i64 %"(i64)var$11[]19_fetch.40$", i64 0, i1 false), !llfort.type_idx !30
  %add.6 = add nuw nsw i64 %"var$16.0", 1
  %func_result22 = tail call i32 @for_deallocate_handle(ptr %"var$11[]18_fetch.38", i32 262144, ptr null) #6, !llfort.type_idx !0
  %add.8 = add nuw nsw i64 %"var$8.1", 2
  %rel.20.not = icmp ugt i64 %add.8, %int_sext35
  br i1 %rel.20.not, label %do.end_do36, label %do.epilog42

do.end_do36:                                      ; preds = %do.epilog42
  %"var$1924" = alloca i8, i64 %mul.6, align 1, !llfort.type_idx !34
  br label %do.body47

do.body47:                                        ; preds = %do.end_do36, %do.body47
  %"var$20.0" = phi i64 [ 1, %do.end_do36 ], [ %add.9, %do.body47 ]
  %"var$1924[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.13.lcssa, ptr nonnull elementtype(i8) %"var$1924", i64 %"var$20.0"), !llfort.type_idx !35
  %"var$1014[]26" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.13.lcssa, ptr nonnull elementtype(i8) %"var$1014", i64 %"var$20.0"), !llfort.type_idx !36
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"var$1924[]", i64 %slct.13.lcssa, ptr nonnull %"var$1014[]26", i64 %slct.13.lcssa, i64 0, i1 false), !llfort.type_idx !30
  %add.9 = add nuw nsw i64 %"var$20.0", 1
  %exitcond186 = icmp eq i64 %add.9, %indvars.iv.lcssa
  br i1 %exitcond186, label %do.end_do48.loopexit, label %do.body47

do.end_do48.loopexit:                             ; preds = %do.body47
  br label %do.end_do48

do.end_do48:                                      ; preds = %do.end_do48.loopexit, %do.end_do36.thread
  %"var$1924159" = phi ptr [ %"var$1924156169.sub", %do.end_do36.thread ], [ %"var$1924", %do.end_do48.loopexit ]
  %"var$7.1149158" = phi i64 [ 0, %do.end_do36.thread ], [ %slct.13.lcssa, %do.end_do48.loopexit ]
  %rel.23.not172 = icmp slt i32 %J17_fetch.6, 1
  br i1 %rel.23.not172, label %loop_exit58, label %loop_body57.preheader

loop_body57.preheader:                            ; preds = %do.end_do48
  %3 = add nsw i64 %int_sext32, 1
  br label %loop_body57

loop_body57:                                      ; preds = %loop_body57.preheader, %loop_body57
  %"$loop_ctr.0173" = phi i64 [ %add.11, %loop_body57 ], [ 1, %loop_body57.preheader ]
  %"var$1924[]27" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$7.1149158", ptr nonnull elementtype(i8) %"var$1924159", i64 %"$loop_ctr.0173"), !llfort.type_idx !37
  %"rt$VECTOR$_14[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.6, ptr nonnull elementtype(i8) %"rt$VECTOR$_14", i64 %"$loop_ctr.0173"), !llfort.type_idx !38
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"rt$VECTOR$_14[]", i64 %slct.6, ptr nonnull %"var$1924[]27", i64 %"var$7.1149158", i64 0, i1 false), !llfort.type_idx !30
  %add.11 = add nuw nsw i64 %"$loop_ctr.0173", 1
  %exitcond187 = icmp eq i64 %add.11, %3
  br i1 %exitcond187, label %loop_exit58.loopexit, label %loop_body57

loop_exit58.loopexit:                             ; preds = %loop_body57
  br label %loop_exit58

loop_exit58:                                      ; preds = %loop_exit58.loopexit, %do.end_do48
  tail call void @llvm.stackrestore(ptr %"$stacksave30"), !llfort.type_idx !30
  %int_sext56 = zext i32 %slct.2 to i64
  %J1_fetch.73 = load i32, ptr %J1, align 1, !tbaa !39, !llfort.type_idx !41
  %"var$36[]57" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$36", i64 0), !llfort.type_idx !21
  store i64 %slct.9, ptr %"var$36[]57", align 8, !tbaa !29
  %"var$29[]58" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$29", i64 0), !llfort.type_idx !21
  %rel.33.not = icmp eq i32 %J1_fetch.73, 1
  %"var$29[]163" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$29", i64 1)
  %"var$31[]49" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$31", i64 0)
  br i1 %rel.33.not, label %bb17.thread, label %bb17

bb17.thread:                                      ; preds = %loop_exit58
  store i64 %int_sext56, ptr %"var$29[]58", align 8, !tbaa !29
  %"var$31[]44" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$31", i64 1)
  store i64 %slct.6, ptr %"var$31[]44", align 8, !tbaa !29
  %mul.12162 = mul nuw nsw i64 %int_sext56, %slct.6
  br label %bb18

bb17:                                             ; preds = %loop_exit58
  store i64 %slct.9, ptr %"var$29[]58", align 8, !tbaa !29
  store i64 %slct.6, ptr %"var$31[]49", align 8, !tbaa !29
  %mul.12 = mul nuw nsw i64 %slct.9, %slct.6
  %rel.34.not = icmp eq i32 %J1_fetch.73, 2
  br i1 %rel.34.not, label %bb19, label %bb17.bb18_crit_edge

bb17.bb18_crit_edge:                              ; preds = %bb17
  %.pre199 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$31", i64 1)
  br label %bb18

bb18:                                             ; preds = %bb17.bb18_crit_edge, %bb17.thread
  %"var$31[]46.pre-phi" = phi ptr [ %.pre199, %bb17.bb18_crit_edge ], [ %"var$31[]49", %bb17.thread ]
  %.pre198.pre-phi = phi i64 [ %mul.12, %bb17.bb18_crit_edge ], [ %mul.12162, %bb17.thread ]
  %.pre.pre-phi = phi ptr [ %.pre199, %bb17.bb18_crit_edge ], [ %"var$31[]44", %bb17.thread ]
  %"var$32.0165" = phi i64 [ 1, %bb17.bb18_crit_edge ], [ 0, %bb17.thread ]
  %"var$36[]45" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$36", i64 %"var$32.0165"), !llfort.type_idx !21
  %"var$36[]45_fetch.90" = load i64, ptr %"var$36[]45", align 8, !tbaa !29, !llfort.type_idx !21
  store i64 %"var$36[]45_fetch.90", ptr %"var$29[]163", align 8, !tbaa !29
  store i64 %.pre198.pre-phi, ptr %"var$31[]46.pre-phi", align 8, !tbaa !29
  %"var$31[]49_fetch.95.pre" = load i64, ptr %"var$31[]49", align 8, !tbaa !29
  br label %bb20

bb19:                                             ; preds = %bb17
  store i64 %int_sext56, ptr %"var$29[]163", align 8, !tbaa !29
  %"var$31[]47" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$31", i64 1)
  store i64 %mul.12, ptr %"var$31[]47", align 8, !tbaa !29
  br label %bb20

bb20:                                             ; preds = %bb19, %bb18
  %mul.14.pre-phi = phi i64 [ %mul.12, %bb19 ], [ %.pre198.pre-phi, %bb18 ]
  %"var$31[]51.pre-phi" = phi ptr [ %"var$31[]47", %bb19 ], [ %.pre.pre-phi, %bb18 ]
  %"var$29[]50_fetch.96" = phi i64 [ %int_sext56, %bb19 ], [ %"var$36[]45_fetch.90", %bb18 ]
  %"var$31[]49_fetch.95" = phi i64 [ %slct.6, %bb19 ], [ %"var$31[]49_fetch.95.pre", %bb18 ]
  %"var$31[]51_fetch.97" = load i64, ptr %"var$31[]51.pre-phi", align 8, !tbaa !29, !llfort.type_idx !21
  %mul.15 = mul nsw i64 %"var$29[]50_fetch.96", %mul.14.pre-phi
  %temp_array = alloca i8, i64 %mul.15, align 1, !llfort.type_idx !42
  %rel.36.not176 = icmp slt i32 %J23_fetch.8, 1
  br i1 %rel.36.not176, label %loop_exit75, label %loop_test69.preheader.lr.ph

loop_test69.preheader.lr.ph:                      ; preds = %bb20
  %4 = add nsw i64 %int_sext32, 1
  %5 = add nuw nsw i64 %int_sext56, 1
  br label %loop_test69.preheader

loop_body70:                                      ; preds = %loop_body70.lr.ph, %loop_body70
  %"$loop_ctr42.0175" = phi i64 [ 1, %loop_body70.lr.ph ], [ %add.19, %loop_body70 ]
  %"rt$VECTOR$_14[]59" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.6, ptr nonnull elementtype(i8) %"rt$VECTOR$_14", i64 %"$loop_ctr42.0175"), !llfort.type_idx !43
  %"temp_array[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %"var$31[]49_fetch.95", ptr nonnull elementtype(i8) %"temp_array[]", i64 %"$loop_ctr42.0175") #7, !llfort.type_idx !44
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"temp_array[][]", i64 %slct.6, ptr nonnull %"rt$VECTOR$_14[]59", i64 %slct.6, i64 0, i1 false), !llfort.type_idx !30
  %add.19 = add nuw nsw i64 %"$loop_ctr42.0175", 1
  %exitcond188 = icmp eq i64 %add.19, %4
  br i1 %exitcond188, label %loop_exit71.loopexit, label %loop_body70

loop_exit71.loopexit:                             ; preds = %loop_body70
  br label %loop_exit71

loop_exit71:                                      ; preds = %loop_exit71.loopexit, %loop_test69.preheader
  %add.20 = add nuw nsw i64 %"$loop_ctr43.0177", 1
  %exitcond189 = icmp eq i64 %add.20, %5
  br i1 %exitcond189, label %loop_exit75.loopexit, label %loop_test69.preheader

loop_test69.preheader:                            ; preds = %loop_test69.preheader.lr.ph, %loop_exit71
  %"$loop_ctr43.0177" = phi i64 [ 1, %loop_test69.preheader.lr.ph ], [ %add.20, %loop_exit71 ]
  br i1 %rel.23.not172, label %loop_exit71, label %loop_body70.lr.ph

loop_body70.lr.ph:                                ; preds = %loop_test69.preheader
  %"temp_array[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %"var$31[]51_fetch.97", ptr nonnull elementtype(i8) %temp_array, i64 %"$loop_ctr43.0177") #7, !llfort.type_idx !45
  br label %loop_body70

loop_exit75.loopexit:                             ; preds = %loop_exit71
  br label %loop_exit75

loop_exit75:                                      ; preds = %loop_exit75.loopexit, %bb20
  %mul.19 = mul nsw i64 %mul.8, %int_sext32
  %temp_array62 = alloca i8, i64 %mul.19, align 1, !llfort.type_idx !46
  br i1 %rel.23.not172, label %loop_exit94, label %loop_test79.preheader.lr.ph

loop_test79.preheader.lr.ph:                      ; preds = %loop_exit75
  %6 = add nsw i64 %int_sext39, 1
  %7 = add nsw i64 %int_sext32, 1
  br label %loop_test79.preheader

loop_body80:                                      ; preds = %loop_body80.lr.ph, %loop_body80
  %"$loop_ctr36.0179" = phi i64 [ 1, %loop_body80.lr.ph ], [ %add.23, %loop_body80 ]
  %"temp_array[][]64" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.6, ptr nonnull elementtype(i8) %"temp_array[]63", i64 %"$loop_ctr36.0179"), !llfort.type_idx !47
  %"temp_array62[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.15, ptr nonnull elementtype(i8) %"temp_array62[]", i64 %"$loop_ctr36.0179"), !llfort.type_idx !48
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"temp_array62[][]", i64 %slct.15, ptr nonnull %"temp_array[][]64", i64 %slct.6, i64 0, i1 false), !llfort.type_idx !30
  %add.23 = add nuw nsw i64 %"$loop_ctr36.0179", 1
  %exitcond190 = icmp eq i64 %add.23, %6
  br i1 %exitcond190, label %loop_exit81.loopexit, label %loop_body80

loop_exit81.loopexit:                             ; preds = %loop_body80
  br label %loop_exit81

loop_exit81:                                      ; preds = %loop_exit81.loopexit, %loop_test79.preheader
  %add.24 = add nuw nsw i64 %"$loop_ctr37.0181", 1
  %exitcond191 = icmp eq i64 %add.24, %7
  br i1 %exitcond191, label %loop_test92.preheader, label %loop_test79.preheader

loop_test79.preheader:                            ; preds = %loop_test79.preheader.lr.ph, %loop_exit81
  %"$loop_ctr37.0181" = phi i64 [ 1, %loop_test79.preheader.lr.ph ], [ %add.24, %loop_exit81 ]
  br i1 %rel.36.not176, label %loop_exit81, label %loop_body80.lr.ph

loop_body80.lr.ph:                                ; preds = %loop_test79.preheader
  %"temp_array[]63" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.14.pre-phi, ptr nonnull elementtype(i8) %temp_array, i64 %"$loop_ctr37.0181"), !llfort.type_idx !49
  %"temp_array62[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.8, ptr nonnull elementtype(i8) %temp_array62, i64 %"$loop_ctr37.0181"), !llfort.type_idx !50
  br label %loop_body80

loop_test92.preheader:                            ; preds = %loop_exit81
  br label %loop_test88.preheader.lr.ph

loop_test88.preheader.lr.ph:                      ; preds = %loop_test92.preheader
  br label %loop_test88.preheader

loop_body89:                                      ; preds = %loop_body89.lr.ph, %loop_body89
  %"$loop_ctr36.1183" = phi i64 [ 1, %loop_body89.lr.ph ], [ %add.25, %loop_body89 ]
  %"rt$MANSWER1$_12[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.15, ptr nonnull elementtype(i8) %"rt$MANSWER1$_12[]", i64 %"$loop_ctr36.1183"), !llfort.type_idx !51
  %"temp_array62[][]66" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %slct.15, ptr nonnull elementtype(i8) %"temp_array62[]65", i64 %"$loop_ctr36.1183"), !llfort.type_idx !52
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"rt$MANSWER1$_12[][]", i64 %slct.15, ptr nonnull %"temp_array62[][]66", i64 %slct.15, i64 0, i1 false), !llfort.type_idx !30
  %add.25 = add nuw nsw i64 %"$loop_ctr36.1183", 1
  %exitcond192 = icmp eq i64 %add.25, %6
  br i1 %exitcond192, label %loop_exit90.loopexit, label %loop_body89

loop_exit90.loopexit:                             ; preds = %loop_body89
  br label %loop_exit90

loop_exit90:                                      ; preds = %loop_exit90.loopexit, %loop_test88.preheader
  %add.26 = add nuw nsw i64 %"$loop_ctr37.1185", 1
  %exitcond193 = icmp eq i64 %add.26, %7
  br i1 %exitcond193, label %loop_exit94.loopexit, label %loop_test88.preheader

loop_test88.preheader:                            ; preds = %loop_test88.preheader.lr.ph, %loop_exit90
  %"$loop_ctr37.1185" = phi i64 [ 1, %loop_test88.preheader.lr.ph ], [ %add.26, %loop_exit90 ]
  br i1 %rel.36.not176, label %loop_exit90, label %loop_body89.lr.ph

loop_body89.lr.ph:                                ; preds = %loop_test88.preheader
  %"rt$MANSWER1$_12[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.8, ptr nonnull elementtype(i8) %"rt$MANSWER1$_12", i64 %"$loop_ctr37.1185"), !llfort.type_idx !53
  %"temp_array62[]65" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul.8, ptr nonnull elementtype(i8) %temp_array62, i64 %"$loop_ctr37.1185"), !llfort.type_idx !54
  br label %loop_body89

loop_exit94.loopexit:                             ; preds = %loop_exit90
  br label %loop_exit94

loop_exit94:                                      ; preds = %loop_exit94.loopexit, %loop_exit75
  ret void
}

declare !llfort.intrin_id !55 i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !56 i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare ptr @llvm.stacksave() #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #3

; Function Attrs: nofree
declare !llfort.intrin_id !57 i32 @for_allocate_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.for.cpystr.i64.i64.i64(ptr noalias nocapture writeonly, i64, ptr noalias nocapture readonly, i64, i64, i1 immarg) #4

declare !llfort.intrin_id !58 i32 @for_deallocate_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.stackrestore(ptr) #2

; Function Attrs: nounwind uwtable
define internal fastcc void @i2c_IP_rt_.1() unnamed_addr #0 {
alloca_1:
  %"rt$MANSWER1$_121" = alloca [3128 x i8], align 1
  %"rt$VECTOR$_142" = alloca [119 x i8], align 1
  %"rt$VECTOR$_142.sub" = getelementptr inbounds [119 x i8], ptr %"rt$VECTOR$_142", i64 0, i64 0
  %"$stacksave30" = tail call ptr @llvm.stacksave(), !llfort.type_idx !22
  %"var$113" = alloca [34 x ptr], align 8
  %"var$113.sub" = getelementptr inbounds [34 x ptr], ptr %"var$113", i64 0, i64 0
  br label %do.body21

do.body21:                                        ; preds = %alloca_1, %loop_exit30
  %"var$9.0" = phi i32 [ 1, %alloca_1 ], [ %add.5, %loop_exit30 ]
  %"var$8.0" = phi i64 [ 1, %alloca_1 ], [ %add.4, %loop_exit30 ]
  %int_zext = trunc i32 %"var$9.0" to i8, !llfort.type_idx !26
  %"var$1484" = alloca [11 x i8], align 1
  %"var$1484.sub" = getelementptr inbounds [11 x i8], ptr %"var$1484", i64 0, i64 0
  br label %loop_body29

loop_body29:                                      ; preds = %do.body21, %loop_body29
  %"$loop_ctr9.02" = phi i64 [ 1, %do.body21 ], [ %add.1, %loop_body29 ]
  %"var$148[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1, ptr nonnull elementtype(i8) %"var$1484.sub", i64 %"$loop_ctr9.02"), !llfort.type_idx !28
  store i8 %int_zext, ptr %"var$148[]", align 1
  %add.1 = add nuw nsw i64 %"$loop_ctr9.02", 1
  %exitcond.not = icmp eq i64 %add.1, 12
  br i1 %exitcond.not, label %loop_exit30, label %loop_body29

loop_exit30:                                      ; preds = %loop_body29
  %"var$11[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$113.sub", i64 %"var$8.0"), !llfort.type_idx !22
  store ptr inttoptr (i64 11 to ptr), ptr %"var$11[]", align 8, !tbaa !29
  %add.3 = add nuw nsw i64 %"var$8.0", 1
  %"var$11[]12" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$113.sub", i64 %add.3), !llfort.type_idx !22
  %func_result = call i32 @for_allocate_handle(i64 11, ptr nonnull %"var$11[]12", i32 262144, ptr null) #6, !llfort.type_idx !0
  %"var$11[]12_fetch.25" = load ptr, ptr %"var$11[]12", align 8, !tbaa !29, !llfort.type_idx !22
  call void @llvm.for.cpystr.i64.i64.i64(ptr %"var$11[]12_fetch.25", i64 11, ptr nonnull %"var$1484.sub", i64 11, i64 0, i1 false), !llfort.type_idx !30
  %add.4 = add nuw nsw i64 %"var$8.0", 2
  %add.5 = add nuw nsw i32 %"var$9.0", 1
  %exitcond10.not = icmp eq i32 %add.5, 18
  br i1 %exitcond10.not, label %do.end_do22, label %do.body21

do.end_do22:                                      ; preds = %loop_exit30
  %"var$101420" = alloca [187 x i8], align 1
  %"var$101420.sub" = getelementptr inbounds [187 x i8], ptr %"var$101420", i64 0, i64 0
  br label %do.epilog42

do.epilog42:                                      ; preds = %do.end_do22, %do.epilog42
  %"var$16.0" = phi i64 [ 1, %do.end_do22 ], [ %add.6, %do.epilog42 ]
  %"var$8.1" = phi i64 [ 2, %do.end_do22 ], [ %add.8, %do.epilog42 ]
  %"var$11[]18" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$113.sub", i64 %"var$8.1"), !llfort.type_idx !22
  %"var$11[]18_fetch.38" = load ptr, ptr %"var$11[]18", align 8, !tbaa !29, !llfort.type_idx !20
  %"var$1014[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 11, ptr nonnull elementtype(i8) %"var$101420.sub", i64 %"var$16.0"), !llfort.type_idx !32
  %"var$11[]18_fetch.38[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 11, ptr elementtype(i8) %"var$11[]18_fetch.38", i64 1), !llfort.type_idx !33
  %sub.6 = add nsw i64 %"var$8.1", -1
  %"var$11[]19" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(ptr) %"var$113.sub", i64 %sub.6), !llfort.type_idx !22
  %"var$11[]19_fetch.40" = load ptr, ptr %"var$11[]19", align 8, !tbaa !29, !llfort.type_idx !22
  %"(i64)var$11[]19_fetch.40$" = ptrtoint ptr %"var$11[]19_fetch.40" to i64, !llfort.type_idx !21
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"var$1014[]", i64 11, ptr %"var$11[]18_fetch.38[]", i64 %"(i64)var$11[]19_fetch.40$", i64 0, i1 false), !llfort.type_idx !30
  %add.6 = add nuw nsw i64 %"var$16.0", 1
  %func_result22 = tail call i32 @for_deallocate_handle(ptr %"var$11[]18_fetch.38", i32 262144, ptr null) #6, !llfort.type_idx !0
  %add.8 = add nuw nsw i64 %"var$8.1", 2
  %exitcond11.not = icmp eq i64 %add.6, 18
  br i1 %exitcond11.not, label %do.end_do36, label %do.epilog42

do.end_do36:                                      ; preds = %do.epilog42
  %"var$192421" = alloca [187 x i8], align 1
  %"var$192421.sub" = getelementptr inbounds [187 x i8], ptr %"var$192421", i64 0, i64 0
  br label %do.body47

do.body47:                                        ; preds = %do.end_do36, %do.body47
  %"var$20.0" = phi i64 [ 1, %do.end_do36 ], [ %add.9, %do.body47 ]
  %"var$1924[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 11, ptr nonnull elementtype(i8) %"var$192421.sub", i64 %"var$20.0"), !llfort.type_idx !35
  %"var$1014[]26" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 11, ptr nonnull elementtype(i8) %"var$101420.sub", i64 %"var$20.0"), !llfort.type_idx !36
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"var$1924[]", i64 11, ptr nonnull %"var$1014[]26", i64 11, i64 0, i1 false), !llfort.type_idx !30
  %add.9 = add nuw nsw i64 %"var$20.0", 1
  %exitcond12 = icmp eq i64 %add.9, 18
  br i1 %exitcond12, label %loop_body57.preheader, label %do.body47

loop_body57.preheader:                            ; preds = %do.body47
  br label %loop_body57

loop_body57:                                      ; preds = %loop_body57.preheader, %loop_body57
  %"$loop_ctr.03" = phi i64 [ %add.11, %loop_body57 ], [ 1, %loop_body57.preheader ]
  %"var$1924[]27" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 11, ptr nonnull elementtype(i8) %"var$192421.sub", i64 %"$loop_ctr.03"), !llfort.type_idx !37
  %"rt$VECTOR$_14[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 7, ptr nonnull elementtype(i8) %"rt$VECTOR$_142.sub", i64 %"$loop_ctr.03"), !llfort.type_idx !38
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"rt$VECTOR$_14[]", i64 7, ptr nonnull %"var$1924[]27", i64 11, i64 0, i1 false), !llfort.type_idx !30
  %add.11 = add nuw nsw i64 %"$loop_ctr.03", 1
  %exitcond13.not = icmp eq i64 %add.11, 18
  br i1 %exitcond13.not, label %bb20, label %loop_body57

bb20:                                             ; preds = %loop_body57
  tail call void @llvm.stackrestore(ptr %"$stacksave30"), !llfort.type_idx !30
  %temp_array1 = alloca [2737 x i8], align 1
  %temp_array1.sub = getelementptr inbounds [2737 x i8], ptr %temp_array1, i64 0, i64 0
  br label %loop_test69.preheader

loop_body70:                                      ; preds = %loop_test69.preheader, %loop_body70
  %"$loop_ctr42.04" = phi i64 [ 1, %loop_test69.preheader ], [ %add.19, %loop_body70 ]
  %"rt$VECTOR$_14[]59" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 7, ptr nonnull elementtype(i8) %"rt$VECTOR$_142.sub", i64 %"$loop_ctr42.04"), !llfort.type_idx !43
  %"temp_array[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 161, ptr nonnull elementtype(i8) %"temp_array[]", i64 %"$loop_ctr42.04") #7, !llfort.type_idx !44
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"temp_array[][]", i64 7, ptr nonnull %"rt$VECTOR$_14[]59", i64 7, i64 0, i1 false), !llfort.type_idx !30
  %add.19 = add nuw nsw i64 %"$loop_ctr42.04", 1
  %exitcond14.not = icmp eq i64 %add.19, 18
  br i1 %exitcond14.not, label %loop_exit71, label %loop_body70

loop_exit71:                                      ; preds = %loop_body70
  %add.20 = add nuw nsw i64 %"$loop_ctr43.05", 1
  %exitcond15.not = icmp eq i64 %add.20, 24
  br i1 %exitcond15.not, label %loop_exit75, label %loop_test69.preheader

loop_test69.preheader:                            ; preds = %bb20, %loop_exit71
  %"$loop_ctr43.05" = phi i64 [ 1, %bb20 ], [ %add.20, %loop_exit71 ]
  %"temp_array[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 7, ptr nonnull elementtype(i8) %temp_array1.sub, i64 %"$loop_ctr43.05") #7, !llfort.type_idx !45
  br label %loop_body70

loop_exit75:                                      ; preds = %loop_exit71
  %temp_array625 = alloca [3128 x i8], align 1
  %temp_array625.sub = getelementptr inbounds [3128 x i8], ptr %temp_array625, i64 0, i64 0
  br label %loop_test79.preheader

loop_body80:                                      ; preds = %loop_test79.preheader, %loop_body80
  %"$loop_ctr36.06" = phi i64 [ 1, %loop_test79.preheader ], [ %add.23, %loop_body80 ]
  %"temp_array[][]64" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 7, ptr nonnull elementtype(i8) %"temp_array[]63", i64 %"$loop_ctr36.06"), !llfort.type_idx !47
  %"temp_array62[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(i8) %"temp_array62[]", i64 %"$loop_ctr36.06"), !llfort.type_idx !48
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"temp_array62[][]", i64 8, ptr nonnull %"temp_array[][]64", i64 7, i64 0, i1 false), !llfort.type_idx !30
  %add.23 = add nuw nsw i64 %"$loop_ctr36.06", 1
  %exitcond16.not = icmp eq i64 %add.23, 24
  br i1 %exitcond16.not, label %loop_exit81, label %loop_body80

loop_exit81:                                      ; preds = %loop_body80
  %add.24 = add nuw nsw i64 %"$loop_ctr37.07", 1
  %exitcond17.not = icmp eq i64 %add.24, 18
  br i1 %exitcond17.not, label %loop_test88.preheader.preheader, label %loop_test79.preheader

loop_test88.preheader.preheader:                  ; preds = %loop_exit81
  %"rt$MANSWER1$_121.sub" = getelementptr inbounds [3128 x i8], ptr %"rt$MANSWER1$_121", i64 0, i64 0
  br label %loop_test88.preheader

loop_test79.preheader:                            ; preds = %loop_exit75, %loop_exit81
  %"$loop_ctr37.07" = phi i64 [ 1, %loop_exit75 ], [ %add.24, %loop_exit81 ]
  %"temp_array[]63" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 161, ptr nonnull elementtype(i8) %temp_array1.sub, i64 %"$loop_ctr37.07"), !llfort.type_idx !49
  %"temp_array62[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 184, ptr nonnull elementtype(i8) %temp_array625.sub, i64 %"$loop_ctr37.07"), !llfort.type_idx !50
  br label %loop_body80

loop_body89:                                      ; preds = %loop_test88.preheader, %loop_body89
  %"$loop_ctr36.18" = phi i64 [ 1, %loop_test88.preheader ], [ %add.25, %loop_body89 ]
  %"rt$MANSWER1$_12[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(i8) %"rt$MANSWER1$_12[]", i64 %"$loop_ctr36.18"), !llfort.type_idx !51
  %"temp_array62[][]66" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(i8) %"temp_array62[]65", i64 %"$loop_ctr36.18"), !llfort.type_idx !52
  call void @llvm.for.cpystr.i64.i64.i64(ptr nonnull %"rt$MANSWER1$_12[][]", i64 8, ptr nonnull %"temp_array62[][]66", i64 8, i64 0, i1 false), !llfort.type_idx !30
  %add.25 = add nuw nsw i64 %"$loop_ctr36.18", 1
  %exitcond18.not = icmp eq i64 %add.25, 24
  br i1 %exitcond18.not, label %loop_exit90, label %loop_body89

loop_exit90:                                      ; preds = %loop_body89
  %add.26 = add nuw nsw i64 %"$loop_ctr37.19", 1
  %exitcond19.not = icmp eq i64 %add.26, 18
  br i1 %exitcond19.not, label %loop_exit94, label %loop_test88.preheader

loop_test88.preheader:                            ; preds = %loop_test88.preheader.preheader, %loop_exit90
  %"$loop_ctr37.19" = phi i64 [ %add.26, %loop_exit90 ], [ 1, %loop_test88.preheader.preheader ]
  %"rt$MANSWER1$_12[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 184, ptr nonnull elementtype(i8) %"rt$MANSWER1$_121.sub", i64 %"$loop_ctr37.19"), !llfort.type_idx !53
  %"temp_array62[]65" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 184, ptr nonnull elementtype(i8) %temp_array625.sub, i64 %"$loop_ctr37.19"), !llfort.type_idx !54
  br label %loop_body89

loop_exit94:                                      ; preds = %loop_exit90
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.smax.i32(i32, i32) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.smax.i64(i64, i64) #5

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #3 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #6 = { nounwind }
attributes #7 = { "stride-reversed" }

!omp_offload.info = !{}

!0 = !{i64 2}
!1 = !{i64 187}
!2 = !{i64 188}
!3 = !{i64 189}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$1", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$i2c_IP_rt_"}
!9 = !{i64 151}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$2", !6, i64 0}
!12 = !{i64 152}
!13 = !{!14, !14, i64 0}
!14 = !{!"ifx$unique_sym$3", !6, i64 0}
!15 = !{i64 153}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$4", !6, i64 0}
!18 = !{i64 154}
!19 = !{i64 16}
!20 = !{i64 15}
!21 = !{i64 3}
!22 = !{i64 11}
!23 = !{!24, !24, i64 0}
!24 = !{!"ifx$unique_sym$5", !6, i64 0}
!25 = !{i64 160}
!26 = !{i64 158}
!27 = !{i64 162}
!28 = !{i64 163}
!29 = !{!6, !6, i64 0}
!30 = !{i64 20}
!31 = !{i64 156}
!32 = !{i64 170}
!33 = !{i64 171}
!34 = !{i64 174}
!35 = !{i64 175}
!36 = !{i64 176}
!37 = !{i64 178}
!38 = !{i64 155}
!39 = !{!40, !40, i64 0}
!40 = !{!"ifx$unique_sym$6", !6, i64 0}
!41 = !{i64 186}
!42 = !{i64 194}
!43 = !{i64 184}
!44 = !{i64 196}
!45 = !{i64 195}
!46 = !{i64 200}
!47 = !{i64 199}
!48 = !{i64 202}
!49 = !{i64 198}
!50 = !{i64 201}
!51 = !{i64 183}
!52 = !{i64 205}
!53 = !{i64 182}
!54 = !{i64 204}
!55 = !{i32 97}
!56 = !{i32 98}
!57 = !{i32 92}
!58 = !{i32 93}

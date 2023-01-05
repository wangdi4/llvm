; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -disable-output 2>&1 | FileCheck %s

; Check that transposing does not not occur for the Loop IV UBRef corresponding to:
; DO i2 = 0, i1, 1
; We cannot create a transpose loop using a UBRef with an IV.
; Def@level is set to 0 for the IV UBRef in this case.

; HIR Before

;      BEGIN REGION { }
;            + DO i1 = 0, %"arbrcs_$NDIM_fetch.2290" + -1, 1
;            |   + DO i2 = 0, i1, 1
;            |   |   %"arbrcs_$X.3" = 0.000000e+00;
;            |   |
;            |   |      %"arbrcs_$X.2" = 0.000000e+00;
;            |   |   + DO i3 = 0, %"arbrcs_$NLINK_fetch.2294" + -1, 1
;            |   |   |   %mul.240 = (%"arbrcs_$B")[i3][i2]  *  (%"arbrcs_$A")[i3][i1];
;            |   |   |   %"arbrcs_$X.2" = %mul.240  +  %"arbrcs_$X.2";
;            |   |   + END LOOP
;            |   |      %"arbrcs_$X.3" = %"arbrcs_$X.2";
;            |   |
;            |   |   %mul.241 = (%"arbrcs_$C")[i2][i1]  *  %"arbrcs_$FC_fetch.2310";
;            |   |   %mul.242 = %"arbrcs_$FA_fetch.2316"  *  %"arbrcs_$X.3";
;            |   |   %add.323 = %mul.242  +  %mul.241;
;            |   |   (%"arbrcs_$C")[i2][i1] = %add.323;
;            |   |   (%"arbrcs_$C")[i1][i2] = %add.323;
;            |   + END LOOP
;            + END LOOP
;      END REGION

; HIR After

;      BEGIN REGION { modified }
;            %call13 = @llvm.stacksave();
;            %TranspTmpArr14 = alloca 8 * (%"arbrcs_$NDIM_fetch.2290" * %"arbrcs_$NLINK_fetch.2294");
;
;            + DO i1 = 0, %"arbrcs_$NDIM_fetch.2290" + -1, 1
;            |   + DO i2 = 0, %"arbrcs_$NLINK_fetch.2294" + -1, 1
;            |   |   (%TranspTmpArr14)[i1][i2] = (%"arbrcs_$A")[i2][i1];
;            |   + END LOOP
;            + END LOOP
;
;
;            + DO i1 = 0, %"arbrcs_$NDIM_fetch.2290" + -1, 1
;            |   + DO i2 = 0, i1, 1
;            |   |   %"arbrcs_$X.3" = 0.000000e+00;
;            |   |
;            |   |      %"arbrcs_$X.2" = 0.000000e+00;
;            |   |   + DO i3 = 0, %"arbrcs_$NLINK_fetch.2294" + -1, 1
;            |   |   |   %mul.240 = (%"arbrcs_$B")[i3][i2]  *  (%TranspTmpArr14)[i1][i3];
;            |   |   |   %"arbrcs_$X.2" = %mul.240  +  %"arbrcs_$X.2";
;            |   |   + END LOOP
;            |   |      %"arbrcs_$X.3" = %"arbrcs_$X.2";
;            |   |
;            |   |   %mul.241 = (%"arbrcs_$C")[i2][i1]  *  %"arbrcs_$FC_fetch.2310";
;            |   |   %mul.242 = %"arbrcs_$FA_fetch.2316"  *  %"arbrcs_$X.3";
;            |   |   %add.323 = %mul.242  +  %mul.241;
;            |   |   (%"arbrcs_$C")[i2][i1] = %add.323;
;            |   |   (%"arbrcs_$C")[i1][i2] = %add.323;
;            |   + END LOOP
;            + END LOOP
;
;            @llvm.stackrestore(&((%call13)[0]));
;      END REGION


; CHECK:  BEGIN REGION { modified }
; CHECK:  DO i1 = 0
; CHECK:  DO i2 = 0

; CHECK:  DO i1 = 0
; CHECK:  DO i2 = 0
; CHECK:  DO i3 = 0
; CHECK:  %mul.240 = (%"arbrcs_$B")[i3][i2]  *  (%TranspTmpArr14)[i1][i3];

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@strlit.93 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0
@strlit.94 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0
@strlit.95 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0
@strlit.96 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0
@strlit.97 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0
@strlit.98 = external hidden unnamed_addr constant [1 x i8], !llfort.type_idx !0

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nofree noinline nounwind uwtable
define void @arbrcs_(i8* noalias nocapture readonly dereferenceable(1) %"arbrcs_$OA", i8* noalias nocapture readonly dereferenceable(1) %"arbrcs_$OB", i64* noalias nocapture readonly dereferenceable(8) %"arbrcs_$NDIM", i64* noalias nocapture readonly dereferenceable(8) %"arbrcs_$NLINK", double* noalias nocapture readonly dereferenceable(8) %"arbrcs_$FA", double* noalias nocapture readonly dereferenceable(8) %"arbrcs_$A", i64* noalias nocapture readonly dereferenceable(8) %"arbrcs_$LDA", double* noalias nocapture readonly dereferenceable(8) %"arbrcs_$B", i64* noalias nocapture readonly dereferenceable(8) %"arbrcs_$LDB", double* noalias nocapture readonly dereferenceable(8) %"arbrcs_$FC", double* noalias nocapture dereferenceable(8) %"arbrcs_$C", i64* noalias nocapture readonly dereferenceable(8) %"arbrcs_$LDC", i64 %"var$159$val", i64 %"var$160$val") local_unnamed_addr #1 !llfort.type_idx !1 {
alloca_12:
  %"arbrcs_$LDA_fetch.2228" = load i64, i64* %"arbrcs_$LDA", align 1, !tbaa !2, !llfort.type_idx !7
  %"arbrcs_$LDB_fetch.2229" = load i64, i64* %"arbrcs_$LDB", align 1, !tbaa !8, !llfort.type_idx !10
  %"arbrcs_$LDC_fetch.2230" = load i64, i64* %"arbrcs_$LDC", align 1, !tbaa !11, !llfort.type_idx !13
  %mul.231 = shl nsw i64 %"arbrcs_$LDA_fetch.2228", 3
  %mul.233 = shl nsw i64 %"arbrcs_$LDB_fetch.2229", 3
  %mul.236 = shl nsw i64 %"arbrcs_$LDC_fetch.2230", 3
  %func_result = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OA", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.98, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %0 = and i64 %func_result, 1
  %int_zext98.not = icmp eq i64 %0, 0
  br i1 %int_zext98.not, label %bb_new1961_else, label %bb_new1939_then

bb665:                                            ; preds = %bb665.preheader, %bb670
  %indvars.iv = phi i64 [ 2, %bb665.preheader ], [ %indvars.iv.next, %bb670 ]
  %"arbrcs_$I.0" = phi i64 [ 1, %bb665.preheader ], [ %add.320, %bb670 ]
  %"arbrcs_$C[]17" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$I.0"), !llfort.type_idx !15
  br label %bb669

bb669:                                            ; preds = %bb674, %bb665
  %"arbrcs_$J.0" = phi i64 [ 1, %bb665 ], [ %add.319, %bb674 ]
  br i1 %rel.414, label %bb674, label %bb673.preheader

bb673.preheader:                                  ; preds = %bb669
  %"arbrcs_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.233, double* nonnull elementtype(double) %"arbrcs_$B", i64 %"arbrcs_$J.0"), !llfort.type_idx !16
  br label %bb673

bb673:                                            ; preds = %bb673, %bb673.preheader
  %"arbrcs_$K.0" = phi i64 [ %add.316, %bb673 ], [ 1, %bb673.preheader ]
  %"arbrcs_$X.0" = phi double [ %add.315, %bb673 ], [ 0.000000e+00, %bb673.preheader ]
  %"arbrcs_$A[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.231, double* nonnull elementtype(double) %"arbrcs_$A", i64 %"arbrcs_$K.0"), !llfort.type_idx !17
  %"arbrcs_$A[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$A[]", i64 %"arbrcs_$I.0"), !llfort.type_idx !18
  %"arbrcs_$A[][]_fetch.2248" = load double, double* %"arbrcs_$A[][]", align 1, !tbaa !19, !llfort.type_idx !18
  %"arbrcs_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$B[]", i64 %"arbrcs_$K.0"), !llfort.type_idx !21
  %"arbrcs_$B[][]_fetch.2255" = load double, double* %"arbrcs_$B[][]", align 1, !tbaa !22, !llfort.type_idx !21
  %mul.235 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$B[][]_fetch.2255", %"arbrcs_$A[][]_fetch.2248"
  %add.315 = fadd reassoc ninf nsz arcp contract afn double %mul.235, %"arbrcs_$X.0"
  %add.316 = add nuw nsw i64 %"arbrcs_$K.0", 1
  %exitcond = icmp eq i64 %add.316, %3
  br i1 %exitcond, label %bb674.loopexit, label %bb673

bb674.loopexit:                                   ; preds = %bb673
  %add.315.lcssa = phi double [ %add.315, %bb673 ]
  br label %bb674

bb674:                                            ; preds = %bb674.loopexit, %bb669
  %"arbrcs_$X.1" = phi double [ 0.000000e+00, %bb669 ], [ %add.315.lcssa, %bb674.loopexit ]
  %"arbrcs_$C[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$J.0"), !llfort.type_idx !24
  %"arbrcs_$C[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]", i64 %"arbrcs_$I.0"), !llfort.type_idx !25
  %"arbrcs_$C[][]_fetch.2266" = load double, double* %"arbrcs_$C[][]", align 1, !tbaa !26, !llfort.type_idx !25
  %mul.238 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$C[][]_fetch.2266", %"arbrcs_$FC_fetch.2259"
  %mul.239 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$FA_fetch.2267", %"arbrcs_$X.1"
  %add.318 = fadd reassoc ninf nsz arcp contract afn double %mul.239, %mul.238
  store double %add.318, double* %"arbrcs_$C[][]", align 1, !tbaa !26
  %"arbrcs_$C[][]18" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]17", i64 %"arbrcs_$J.0"), !llfort.type_idx !28
  store double %add.318, double* %"arbrcs_$C[][]18", align 1, !tbaa !26
  %add.319 = add nuw nsw i64 %"arbrcs_$J.0", 1
  %exitcond240 = icmp eq i64 %add.319, %indvars.iv
  br i1 %exitcond240, label %bb670, label %bb669

bb670:                                            ; preds = %bb674
  %add.320 = add nuw nsw i64 %"arbrcs_$I.0", 1
  %indvars.iv.next = add nuw i64 %indvars.iv, 1
  %exitcond241 = icmp eq i64 %indvars.iv.next, %4
  br i1 %exitcond241, label %bb721_endif.loopexit262, label %bb665

bb677:                                            ; preds = %bb677.preheader, %bb682
  %indvars.iv243 = phi i64 [ 2, %bb677.preheader ], [ %indvars.iv.next244, %bb682 ]
  %"arbrcs_$I.1" = phi i64 [ 1, %bb677.preheader ], [ %add.325, %bb682 ]
  %"arbrcs_$C[]37" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$I.1"), !llfort.type_idx !29
  br label %bb681

bb681:                                            ; preds = %bb686, %bb677
  %"arbrcs_$J.1" = phi i64 [ 1, %bb677 ], [ %add.324, %bb686 ]
  br i1 %rel.420, label %bb686, label %bb685.preheader

bb685.preheader:                                  ; preds = %bb681
  br label %bb685

bb685:                                            ; preds = %bb685, %bb685.preheader
  %"arbrcs_$K.1" = phi i64 [ %add.322, %bb685 ], [ 1, %bb685.preheader ]
  %"arbrcs_$X.2" = phi double [ %add.321, %bb685 ], [ 0.000000e+00, %bb685.preheader ]
  %"arbrcs_$A[]27" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.231, double* nonnull elementtype(double) %"arbrcs_$A", i64 %"arbrcs_$K.1"), !llfort.type_idx !30
  %"arbrcs_$A[][]28" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$A[]27", i64 %"arbrcs_$I.1"), !llfort.type_idx !31
  %"arbrcs_$A[][]_fetch.2301" = load double, double* %"arbrcs_$A[][]28", align 1, !tbaa !19, !llfort.type_idx !31
  %"arbrcs_$B[]29" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.233, double* nonnull elementtype(double) %"arbrcs_$B", i64 %"arbrcs_$K.1"), !llfort.type_idx !32
  %"arbrcs_$B[][]30" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$B[]29", i64 %"arbrcs_$J.1"), !llfort.type_idx !33
  %"arbrcs_$B[][]_fetch.2306" = load double, double* %"arbrcs_$B[][]30", align 1, !tbaa !22, !llfort.type_idx !33
  %mul.240 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$B[][]_fetch.2306", %"arbrcs_$A[][]_fetch.2301"
  %add.321 = fadd reassoc ninf nsz arcp contract afn double %mul.240, %"arbrcs_$X.2"
  %add.322 = add nuw nsw i64 %"arbrcs_$K.1", 1
  %exitcond242 = icmp eq i64 %add.322, %1
  br i1 %exitcond242, label %bb686.loopexit, label %bb685

bb686.loopexit:                                   ; preds = %bb685
  %add.321.lcssa = phi double [ %add.321, %bb685 ]
  br label %bb686

bb686:                                            ; preds = %bb686.loopexit, %bb681
  %"arbrcs_$X.3" = phi double [ 0.000000e+00, %bb681 ], [ %add.321.lcssa, %bb686.loopexit ]
  %"arbrcs_$C[]31" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$J.1"), !llfort.type_idx !34
  %"arbrcs_$C[][]32" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]31", i64 %"arbrcs_$I.1"), !llfort.type_idx !35
  %"arbrcs_$C[][]_fetch.2315" = load double, double* %"arbrcs_$C[][]32", align 1, !tbaa !26, !llfort.type_idx !35
  %mul.241 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$C[][]_fetch.2315", %"arbrcs_$FC_fetch.2310"
  %mul.242 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$FA_fetch.2316", %"arbrcs_$X.3"
  %add.323 = fadd reassoc ninf nsz arcp contract afn double %mul.242, %mul.241
  store double %add.323, double* %"arbrcs_$C[][]32", align 1, !tbaa !26
  %"arbrcs_$C[][]38" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]37", i64 %"arbrcs_$J.1"), !llfort.type_idx !36
  store double %add.323, double* %"arbrcs_$C[][]38", align 1, !tbaa !26
  %add.324 = add nuw nsw i64 %"arbrcs_$J.1", 1
  %exitcond245 = icmp eq i64 %add.324, %indvars.iv243
  br i1 %exitcond245, label %bb682, label %bb681

bb682:                                            ; preds = %bb686
  %add.325 = add nuw nsw i64 %"arbrcs_$I.1", 1
  %indvars.iv.next244 = add nuw i64 %indvars.iv243, 1
  %exitcond246 = icmp eq i64 %indvars.iv.next244, %2
  br i1 %exitcond246, label %bb721_endif.loopexit261, label %bb677

bb_new1956_then:                                  ; preds = %bb_new1953_else
  %"arbrcs_$NDIM_fetch.2290" = load i64, i64* %"arbrcs_$NDIM", align 1, !tbaa !37, !llfort.type_idx !39
  %rel.418 = icmp slt i64 %"arbrcs_$NDIM_fetch.2290", 1
  br i1 %rel.418, label %bb721_endif, label %bb677.preheader

bb677.preheader:                                  ; preds = %bb_new1956_then
  %"arbrcs_$NLINK_fetch.2294" = load i64, i64* %"arbrcs_$NLINK", align 1, !tbaa !40, !llfort.type_idx !42
  %rel.420 = icmp slt i64 %"arbrcs_$NLINK_fetch.2294", 1
  %"arbrcs_$FC_fetch.2310" = load double, double* %"arbrcs_$FC", align 1, !tbaa !43, !llfort.type_idx !45
  %"arbrcs_$FA_fetch.2316" = load double, double* %"arbrcs_$FA", align 1, !tbaa !46, !llfort.type_idx !48
  %1 = add nsw i64 %"arbrcs_$NLINK_fetch.2294", 1
  %2 = add nuw i64 %"arbrcs_$NDIM_fetch.2290", 2
  br label %bb677

bb_new1942_then:                                  ; preds = %bb_new1939_then
  %"arbrcs_$NDIM_fetch.2235" = load i64, i64* %"arbrcs_$NDIM", align 1, !tbaa !37, !llfort.type_idx !49
  %rel.412 = icmp slt i64 %"arbrcs_$NDIM_fetch.2235", 1
  br i1 %rel.412, label %bb721_endif, label %bb665.preheader

bb665.preheader:                                  ; preds = %bb_new1942_then
  %"arbrcs_$NLINK_fetch.2239" = load i64, i64* %"arbrcs_$NLINK", align 1, !tbaa !40, !llfort.type_idx !50
  %rel.414 = icmp slt i64 %"arbrcs_$NLINK_fetch.2239", 1
  %"arbrcs_$FC_fetch.2259" = load double, double* %"arbrcs_$FC", align 1, !tbaa !43, !llfort.type_idx !51
  %"arbrcs_$FA_fetch.2267" = load double, double* %"arbrcs_$FA", align 1, !tbaa !46, !llfort.type_idx !52
  %3 = add nsw i64 %"arbrcs_$NLINK_fetch.2239", 1
  %4 = add nuw i64 %"arbrcs_$NDIM_fetch.2235", 2
  br label %bb665

bb_new1953_else:                                  ; preds = %bb_new1939_then
  %func_result20 = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OB", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.96, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %5 = and i64 %func_result20, 1
  %int_zext.not = icmp eq i64 %5, 0
  br i1 %int_zext.not, label %bb721_endif, label %bb_new1956_then

bb692:                                            ; preds = %bb692.preheader, %bb697
  %indvars.iv248 = phi i64 [ 2, %bb692.preheader ], [ %indvars.iv.next249, %bb697 ]
  %"arbrcs_$I.2" = phi i64 [ 1, %bb692.preheader ], [ %add.330, %bb697 ]
  %"arbrcs_$C[]67" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$I.2"), !llfort.type_idx !53
  %"arbrcs_$A[]57" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.231, double* nonnull elementtype(double) %"arbrcs_$A", i64 %"arbrcs_$I.2")
  br label %bb696

bb696:                                            ; preds = %bb701, %bb692
  %"arbrcs_$J.2" = phi i64 [ 1, %bb692 ], [ %add.329, %bb701 ]
  br i1 %rel.426, label %bb701, label %bb700.preheader

bb700.preheader:                                  ; preds = %bb696
  %"arbrcs_$B[]59" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.233, double* nonnull elementtype(double) %"arbrcs_$B", i64 %"arbrcs_$J.2"), !llfort.type_idx !54
  br label %bb700

bb700:                                            ; preds = %bb700, %bb700.preheader
  %"arbrcs_$K.2" = phi i64 [ %add.327, %bb700 ], [ 1, %bb700.preheader ]
  %"arbrcs_$X.4" = phi double [ %add.326, %bb700 ], [ 0.000000e+00, %bb700.preheader ]
  %"arbrcs_$A[][]58" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$A[]57", i64 %"arbrcs_$K.2"), !llfort.type_idx !55
  %"arbrcs_$A[][]_fetch.2352" = load double, double* %"arbrcs_$A[][]58", align 1, !tbaa !19, !llfort.type_idx !55
  %"arbrcs_$B[][]60" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$B[]59", i64 %"arbrcs_$K.2"), !llfort.type_idx !56
  %"arbrcs_$B[][]_fetch.2357" = load double, double* %"arbrcs_$B[][]60", align 1, !tbaa !22, !llfort.type_idx !56
  %mul.243 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$B[][]_fetch.2357", %"arbrcs_$A[][]_fetch.2352"
  %add.326 = fadd reassoc ninf nsz arcp contract afn double %mul.243, %"arbrcs_$X.4"
  %add.327 = add nuw nsw i64 %"arbrcs_$K.2", 1
  %exitcond247 = icmp eq i64 %add.327, %8
  br i1 %exitcond247, label %bb701.loopexit, label %bb700

bb701.loopexit:                                   ; preds = %bb700
  %add.326.lcssa = phi double [ %add.326, %bb700 ]
  br label %bb701

bb701:                                            ; preds = %bb701.loopexit, %bb696
  %"arbrcs_$X.5" = phi double [ 0.000000e+00, %bb696 ], [ %add.326.lcssa, %bb701.loopexit ]
  %"arbrcs_$C[]61" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$J.2"), !llfort.type_idx !57
  %"arbrcs_$C[][]62" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]61", i64 %"arbrcs_$I.2"), !llfort.type_idx !58
  %"arbrcs_$C[][]_fetch.2366" = load double, double* %"arbrcs_$C[][]62", align 1, !tbaa !26, !llfort.type_idx !58
  %mul.244 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$C[][]_fetch.2366", %"arbrcs_$FC_fetch.2361"
  %mul.245 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$FA_fetch.2367", %"arbrcs_$X.5"
  %add.328 = fadd reassoc ninf nsz arcp contract afn double %mul.245, %mul.244
  store double %add.328, double* %"arbrcs_$C[][]62", align 1, !tbaa !26
  %"arbrcs_$C[][]68" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]67", i64 %"arbrcs_$J.2"), !llfort.type_idx !59
  store double %add.328, double* %"arbrcs_$C[][]68", align 1, !tbaa !26
  %add.329 = add nuw nsw i64 %"arbrcs_$J.2", 1
  %exitcond250 = icmp eq i64 %add.329, %indvars.iv248
  br i1 %exitcond250, label %bb697, label %bb696

bb697:                                            ; preds = %bb701
  %add.330 = add nuw nsw i64 %"arbrcs_$I.2", 1
  %indvars.iv.next249 = add nuw i64 %indvars.iv248, 1
  %exitcond251 = icmp eq i64 %indvars.iv.next249, %9
  br i1 %exitcond251, label %bb721_endif.loopexit260, label %bb692

bb704:                                            ; preds = %bb704.preheader, %bb709
  %indvars.iv253 = phi i64 [ 2, %bb704.preheader ], [ %indvars.iv.next254, %bb709 ]
  %"arbrcs_$I.3" = phi i64 [ 1, %bb704.preheader ], [ %add.335, %bb709 ]
  %"arbrcs_$C[]87" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$I.3"), !llfort.type_idx !60
  %"arbrcs_$A[]77" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.231, double* nonnull elementtype(double) %"arbrcs_$A", i64 %"arbrcs_$I.3")
  br label %bb708

bb708:                                            ; preds = %bb713, %bb704
  %"arbrcs_$J.3" = phi i64 [ 1, %bb704 ], [ %add.334, %bb713 ]
  br i1 %rel.432, label %bb713, label %bb712.preheader

bb712.preheader:                                  ; preds = %bb708
  br label %bb712

bb712:                                            ; preds = %bb712, %bb712.preheader
  %"arbrcs_$K.3" = phi i64 [ %add.332, %bb712 ], [ 1, %bb712.preheader ]
  %"arbrcs_$X.6" = phi double [ %add.331, %bb712 ], [ 0.000000e+00, %bb712.preheader ]
  %"arbrcs_$A[][]78" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$A[]77", i64 %"arbrcs_$K.3"), !llfort.type_idx !61
  %"arbrcs_$A[][]_fetch.2401" = load double, double* %"arbrcs_$A[][]78", align 1, !tbaa !19, !llfort.type_idx !61
  %"arbrcs_$B[]79" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.233, double* nonnull elementtype(double) %"arbrcs_$B", i64 %"arbrcs_$K.3"), !llfort.type_idx !62
  %"arbrcs_$B[][]80" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$B[]79", i64 %"arbrcs_$J.3"), !llfort.type_idx !63
  %"arbrcs_$B[][]_fetch.2406" = load double, double* %"arbrcs_$B[][]80", align 1, !tbaa !22, !llfort.type_idx !63
  %mul.246 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$B[][]_fetch.2406", %"arbrcs_$A[][]_fetch.2401"
  %add.331 = fadd reassoc ninf nsz arcp contract afn double %mul.246, %"arbrcs_$X.6"
  %add.332 = add nuw nsw i64 %"arbrcs_$K.3", 1
  %exitcond252 = icmp eq i64 %add.332, %6
  br i1 %exitcond252, label %bb713.loopexit, label %bb712

bb713.loopexit:                                   ; preds = %bb712
  %add.331.lcssa = phi double [ %add.331, %bb712 ]
  br label %bb713

bb713:                                            ; preds = %bb713.loopexit, %bb708
  %"arbrcs_$X.7" = phi double [ 0.000000e+00, %bb708 ], [ %add.331.lcssa, %bb713.loopexit ]
  %"arbrcs_$C[]81" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.236, double* nonnull elementtype(double) %"arbrcs_$C", i64 %"arbrcs_$J.3"), !llfort.type_idx !64
  %"arbrcs_$C[][]82" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]81", i64 %"arbrcs_$I.3"), !llfort.type_idx !65
  %"arbrcs_$C[][]_fetch.2415" = load double, double* %"arbrcs_$C[][]82", align 1, !tbaa !26, !llfort.type_idx !65
  %mul.247 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$C[][]_fetch.2415", %"arbrcs_$FC_fetch.2410"
  %mul.248 = fmul reassoc ninf nsz arcp contract afn double %"arbrcs_$FA_fetch.2416", %"arbrcs_$X.7"
  %add.333 = fadd reassoc ninf nsz arcp contract afn double %mul.248, %mul.247
  store double %add.333, double* %"arbrcs_$C[][]82", align 1, !tbaa !26
  %"arbrcs_$C[][]88" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"arbrcs_$C[]87", i64 %"arbrcs_$J.3"), !llfort.type_idx !66
  store double %add.333, double* %"arbrcs_$C[][]88", align 1, !tbaa !26
  %add.334 = add nuw nsw i64 %"arbrcs_$J.3", 1
  %exitcond255 = icmp eq i64 %add.334, %indvars.iv253
  br i1 %exitcond255, label %bb709, label %bb708

bb709:                                            ; preds = %bb713
  %add.335 = add nuw nsw i64 %"arbrcs_$I.3", 1
  %indvars.iv.next254 = add nuw i64 %indvars.iv253, 1
  %exitcond256 = icmp eq i64 %indvars.iv.next254, %7
  br i1 %exitcond256, label %bb721_endif.loopexit, label %bb704

bb_new1975_then:                                  ; preds = %bb_new1972_else
  %"arbrcs_$NDIM_fetch.2390" = load i64, i64* %"arbrcs_$NDIM", align 1, !tbaa !37, !llfort.type_idx !67
  %rel.430 = icmp slt i64 %"arbrcs_$NDIM_fetch.2390", 1
  br i1 %rel.430, label %bb721_endif, label %bb704.preheader

bb704.preheader:                                  ; preds = %bb_new1975_then
  %"arbrcs_$NLINK_fetch.2394" = load i64, i64* %"arbrcs_$NLINK", align 1, !tbaa !40, !llfort.type_idx !68
  %rel.432 = icmp slt i64 %"arbrcs_$NLINK_fetch.2394", 1
  %"arbrcs_$FC_fetch.2410" = load double, double* %"arbrcs_$FC", align 1, !tbaa !43, !llfort.type_idx !69
  %"arbrcs_$FA_fetch.2416" = load double, double* %"arbrcs_$FA", align 1, !tbaa !46, !llfort.type_idx !70
  %6 = add nsw i64 %"arbrcs_$NLINK_fetch.2394", 1
  %7 = add nuw i64 %"arbrcs_$NDIM_fetch.2390", 2
  br label %bb704

bb_new1967_then:                                  ; preds = %bb_new1964_then
  %"arbrcs_$NDIM_fetch.2341" = load i64, i64* %"arbrcs_$NDIM", align 1, !tbaa !37, !llfort.type_idx !71
  %rel.424 = icmp slt i64 %"arbrcs_$NDIM_fetch.2341", 1
  br i1 %rel.424, label %bb721_endif, label %bb692.preheader

bb692.preheader:                                  ; preds = %bb_new1967_then
  %"arbrcs_$NLINK_fetch.2345" = load i64, i64* %"arbrcs_$NLINK", align 1, !tbaa !40, !llfort.type_idx !72
  %rel.426 = icmp slt i64 %"arbrcs_$NLINK_fetch.2345", 1
  %"arbrcs_$FC_fetch.2361" = load double, double* %"arbrcs_$FC", align 1, !tbaa !43, !llfort.type_idx !73
  %"arbrcs_$FA_fetch.2367" = load double, double* %"arbrcs_$FA", align 1, !tbaa !46, !llfort.type_idx !74
  %8 = add nsw i64 %"arbrcs_$NLINK_fetch.2345", 1
  %9 = add nuw i64 %"arbrcs_$NDIM_fetch.2341", 2
  br label %bb692

bb_new1972_else:                                  ; preds = %bb_new1964_then
  %func_result70 = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OB", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.93, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %10 = and i64 %func_result70, 1
  %int_zext90.not = icmp eq i64 %10, 0
  br i1 %int_zext90.not, label %bb721_endif, label %bb_new1975_then

bb_new1964_then:                                  ; preds = %bb_new1961_else
  %func_result50 = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OB", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.94, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %11 = and i64 %func_result50, 1
  %int_zext92.not = icmp eq i64 %11, 0
  br i1 %int_zext92.not, label %bb_new1972_else, label %bb_new1967_then

bb_new1939_then:                                  ; preds = %alloca_12
  %func_result6 = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OB", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.97, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %12 = and i64 %func_result6, 1
  %int_zext40.not = icmp eq i64 %12, 0
  br i1 %int_zext40.not, label %bb_new1953_else, label %bb_new1942_then

bb_new1961_else:                                  ; preds = %alloca_12
  %func_result42 = tail call i64 @for_cpstr(i8* nonnull %"arbrcs_$OA", i64 1, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @strlit.95, i64 0, i64 0), i64 1, i64 2) #3, !llfort.type_idx !14
  %13 = and i64 %func_result42, 1
  %int_zext95.not = icmp eq i64 %13, 0
  br i1 %int_zext95.not, label %bb721_endif, label %bb_new1964_then

bb721_endif.loopexit:                             ; preds = %bb709
  br label %bb721_endif

bb721_endif.loopexit260:                          ; preds = %bb697
  br label %bb721_endif

bb721_endif.loopexit261:                          ; preds = %bb682
  br label %bb721_endif

bb721_endif.loopexit262:                          ; preds = %bb670
  br label %bb721_endif

bb721_endif:                                      ; preds = %bb721_endif.loopexit262, %bb721_endif.loopexit261, %bb721_endif.loopexit260, %bb721_endif.loopexit, %bb_new1961_else, %bb_new1972_else, %bb_new1967_then, %bb_new1975_then, %bb_new1953_else, %bb_new1942_then, %bb_new1956_then
  ret void
}

; Function Attrs: nofree
declare !llfort.type_idx !75 !llfort.intrin_id !76 i64 @for_cpstr(i8* nocapture readonly, i64, i8* nocapture readonly, i64, i64) local_unnamed_addr #2

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree noinline nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}

!0 = !{i64 39}
!1 = !{i64 6240}
!2 = !{!3, !3, i64 0}
!3 = !{!"ifx$unique_sym$679", !4, i64 0}
!4 = !{!"Fortran Data Symbol", !5, i64 0}
!5 = !{!"Generic Fortran Symbol", !6, i64 0}
!6 = !{!"ifx$root$13$arbrcs_"}
!7 = !{i64 6261}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$680", !4, i64 0}
!10 = !{i64 6262}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$681", !4, i64 0}
!13 = !{i64 6263}
!14 = !{i64 3}
!15 = !{i64 6288}
!16 = !{i64 6278}
!17 = !{i64 6276}
!18 = !{i64 6277}
!19 = !{!20, !20, i64 0}
!20 = !{!"ifx$unique_sym$692", !4, i64 0}
!21 = !{i64 6279}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$693", !4, i64 0}
!24 = !{i64 6281}
!25 = !{i64 6282}
!26 = !{!27, !27, i64 0}
!27 = !{!"ifx$unique_sym$695", !4, i64 0}
!28 = !{i64 6289}
!29 = !{i64 6307}
!30 = !{i64 6295}
!31 = !{i64 6296}
!32 = !{i64 6297}
!33 = !{i64 6298}
!34 = !{i64 6300}
!35 = !{i64 6301}
!36 = !{i64 6308}
!37 = !{!38, !38, i64 0}
!38 = !{!"ifx$unique_sym$686", !4, i64 0}
!39 = !{i64 6293}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$690", !4, i64 0}
!42 = !{i64 6294}
!43 = !{!44, !44, i64 0}
!44 = !{!"ifx$unique_sym$694", !4, i64 0}
!45 = !{i64 6299}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$696", !4, i64 0}
!48 = !{i64 6302}
!49 = !{i64 6274}
!50 = !{i64 6275}
!51 = !{i64 6280}
!52 = !{i64 6283}
!53 = !{i64 6329}
!54 = !{i64 6319}
!55 = !{i64 6318}
!56 = !{i64 6320}
!57 = !{i64 6322}
!58 = !{i64 6323}
!59 = !{i64 6330}
!60 = !{i64 6348}
!61 = !{i64 6337}
!62 = !{i64 6338}
!63 = !{i64 6339}
!64 = !{i64 6341}
!65 = !{i64 6342}
!66 = !{i64 6349}
!67 = !{i64 6334}
!68 = !{i64 6335}
!69 = !{i64 6340}
!70 = !{i64 6343}
!71 = !{i64 6315}
!72 = !{i64 6316}
!73 = !{i64 6321}
!74 = !{i64 6324}
!75 = !{i64 6267}
!76 = !{i32 8}

; REQUIRES: asserts
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -disable-output -vplan-force-vf=2 -vplan-cost-model-print-analysis-for-vf=2 \
; RUN:     -mtriple=x86_64-unknown-unknown 2>&1 -vplan-enable-new-cfg-merge-hir=0 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -disable-output -vplan-force-vf=2 -vplan-cost-model-print-analysis-for-vf=2 \
; RUN:     -mtriple=x86_64-unknown-unknown 2>&1 -vplan-enable-new-cfg-merge-hir=1 | FileCheck %s

; Cost of serializing select instruction with non-vectorizable operand types
; Here cost of serializing select instruction = cost of n extracts from vec cond + cost of n selects, where n = VF
; CHECK-LABEL:  Cost Model for VPlan extent_mp_foo_nd_:HIR.#{{[0-9]+}} with VF = 2:
; CHECK-NEXT:  Analyzing VPBasicBlock [[BB0:BB[0-9]+]]
; CHECK-NEXT:    Cost 0 for br [[BB1:BB[0-9]+]]
; CHECK-NEXT:  [[BB0]]: base cost: 0
; CHECK-NEXT:  Analyzing VPBasicBlock [[BB1]]
; CHECK-NEXT:    Cost 1 for i64 [[VP0:%.*]] = add i64 [[VP1:%.*]] i64 1
; CHECK-NEXT:    Cost Unknown for i64 [[VP_VECTOR_TRIP_COUNT:%.*]] = vector-trip-count i64 [[VP0]], UF = 1
; CHECK-NEXT:    Cost 0 for i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 live-in0 i64 1
; CHECK-NEXT:    Cost 0 for i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:    Cost 0 for br [[BB2:BB[0-9]+]]
; CHECK-NEXT:  [[BB1]]: base cost: 1
; CHECK-NEXT:  Cost Model for Loop preheader [[BB0]] : [[BB1]] for VF = 2 resulted Cost = 1
; CHECK-NEXT:  Analyzing VPBasicBlock [[BB2]]
; CHECK-NEXT:    Cost Unknown for i64 [[VP2:%.*]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP3:%.*]], [[BB2]] ]
; CHECK-NEXT:    Cost 0 for i32* [[VP_SUBSCRIPT:%.*]] = subscript inbounds %"EXTENT$.btINTVL"* %"A.addr_a0$_fetch.8" i64 [[VP2]] (0 )
; CHECK-NEXT:    Cost 9 for i32 [[VP_LOAD:%.*]] = load i32* [[VP_SUBSCRIPT]]
; CHECK-NEXT:    Cost 1 for i32 [[VP4:%.*]] = add i32 [[VP_LOAD]] i32 1
; CHECK-NEXT:    Cost Unknown for %"EXTENT$.btINTVL" = type { i32, i32 } [[VP5:%.*]] = insertvalue %"EXTENT$.btINTVL" undef i32 [[VP4]]
; CHECK-NEXT:    Cost Unknown for %"EXTENT$.btINTVL" = type { i32, i32 } [[VP7:%.*]] = insertvalue %"EXTENT$.btINTVL" undef i32 [[VP4]]
; CHECK-NEXT:    Cost 1 for i32 [[VP8:%.*]] = add i32 [[VP_LOAD]] i32 2
; CHECK-NEXT:    Cost Unknown for %"EXTENT$.btINTVL" = type { i32, i32 } [[VP9:%.*]] = insertvalue %"EXTENT$.btINTVL" undef i32 [[VP8]]
; CHECK-NEXT:    Cost Unknown for %"EXTENT$.btINTVL" = type { i32, i32 } [[VP11:%.*]] = insertvalue %"EXTENT$.btINTVL" undef i32 [[VP8]]
; CHECK-NEXT:    Cost 4 for i1 [[VP12:%.*]] = icmp sgt i32 [[VP_LOAD]] i32 1
; CHECK-NEXT:    Cost 6 for %"EXTENT$.btINTVL" = type { i32, i32 } [[VP13:%.*]] = select i1 [[VP12]] %"EXTENT$.btINTVL" = type { i32, i32 } [[VP7]] %"EXTENT$.btINTVL" = type { i32, i32 } [[VP11]]
; CHECK-NEXT:    Cost Unknown for i32 [[VP14:%.*]] = extractvalue %"EXTENT$.btINTVL" = type { i32, i32 } [[VP13]]
; CHECK-NEXT:    Cost Unknown for i32 [[VP15:%.*]] = extractvalue %"EXTENT$.btINTVL" = type { i32, i32 } [[VP13]]
; CHECK-NEXT:    Cost 1 for i32 [[VP16:%.*]] = add i32 [[VP15]] i32 [[VP14]]
; CHECK-NEXT:    Cost 0 for i32* [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds i32* %"var$6" i64 [[VP2]]
; CHECK-NEXT:    Cost 1.0625 for store i32 [[VP16]] i32* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:    Cost 1 for i64 [[VP3]] = add i64 [[VP2]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:    Cost 8 for i1 [[VP17:%.*]] = icmp slt i64 [[VP3]] i64 [[VP_VECTOR_TRIP_COUNT]]
; CHECK-NEXT:    Cost 0 for br i1 [[VP17]], [[BB2]], [[BB3:BB[0-9]+]]
; CHECK-NEXT:  [[BB2]]: base cost: 32.0625
; CHECK-NEXT:  Base Cost: 32.0625
; CHECK-NEXT:  Analyzing VPBasicBlock [[BB3]]
; CHECK-NEXT:    Cost 0 for i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:    Cost 0 for br [[BB4:BB[0-9]+]]
; CHECK-NEXT:  [[BB3]]: base cost: 0
; CHECK-NEXT:  Analyzing VPBasicBlock [[BB4]]
; CHECK-NEXT:    Cost 0 for br <External Block>
; CHECK-NEXT:  [[BB4]]: base cost: 0
; CHECK-NEXT:  Cost Model for Loop postexit [[BB3]] : [[BB4]] for VF = 2 resulted Cost = 0

%"EXTENT$.btINTVL" = type { i32, i32 }
%"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$" = type { %"EXTENT$.btINTVL"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nofree nosync nounwind uwtable
define i32 @extent_mp_foo_nd_(%"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$"* noalias nocapture readonly dereferenceable(72) "assumed_shape" "ptrnoalias" %A) local_unnamed_addr #2 {
alloca_2:
  %"A.addr_a0$" = getelementptr inbounds %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$", %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$"* %A, i64 0, i32 0
  %"A.addr_a0$_fetch.8" = load %"EXTENT$.btINTVL"*, %"EXTENT$.btINTVL"** %"A.addr_a0$", align 1
  %"A.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$", %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$"* %A, i64 0, i32 6, i64 0, i32 1
  %"A.dim_info$.spacing$[]" = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"A.dim_info$.spacing$", i32 0)
  %"A.dim_info$.spacing$[]_fetch.9" = load i64, i64* %"A.dim_info$.spacing$[]", align 1
  %"A.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$", %"QNCA_a0$%\22EXTENT$.btINTVL\22*$rank1$"* %A, i64 0, i32 6, i64 0, i32 0
  %"A.dim_info$.extent$[]" = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"A.dim_info$.extent$", i32 0)
  %"A.dim_info$.extent$[]_fetch.10" = load i64, i64* %"A.dim_info$.extent$[]", align 1
  %0 = icmp sgt i64 %"A.dim_info$.extent$[]_fetch.10", 0
  %slct.3 = select i1 %0, i64 %"A.dim_info$.extent$[]_fetch.10", i64 0
  %"var$6" = alloca i32, i64 %slct.3, align 4
  %rel.4.not11 = icmp slt i64 %"A.dim_info$.extent$[]_fetch.10", 1
  br i1 %rel.4.not11, label %loop_exit11, label %loop_body10.preheader

loop_body10.preheader:                            ; preds = %alloca_2
  %1 = add nuw nsw i64 %"A.dim_info$.extent$[]_fetch.10", 1
  br label %loop_body10

loop_body10:                                      ; preds = %loop_body10.preheader, %loop_body10
  %"$loop_ctr.012" = phi i64 [ %add.3, %loop_body10 ], [ 1, %loop_body10.preheader ]
  %"A.addr_a0$_fetch.8[]" = tail call %"EXTENT$.btINTVL"* @"llvm.intel.subscript.p0s_EXTENT$.btINTVLs.i64.i64.p0s_EXTENT$.btINTVLs.i64"(i8 0, i64 1, i64 %"A.dim_info$.spacing$[]_fetch.9", %"EXTENT$.btINTVL"* elementtype(%"EXTENT$.btINTVL") %"A.addr_a0$_fetch.8", i64 %"$loop_ctr.012")
  %"A.FIRST$.i" = getelementptr inbounds %"EXTENT$.btINTVL", %"EXTENT$.btINTVL"* %"A.addr_a0$_fetch.8[]", i64 0, i32 0
  %"A.FIRST$_fetch.1.i" = load i32, i32* %"A.FIRST$.i", align 1
  %add.1.i = add nsw i32 %"A.FIRST$_fetch.1.i", 1
  %"var$2_fetch.3.fca.0.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.1.i, 0
  %"var$2_fetch.3.fca.1.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.1.i, 1
  %add.2.i = add nsw i32 %"A.FIRST$_fetch.1.i", 2
  %"var$3_fetch.5.fca.0.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.2.i, 0
  %"var$3_fetch.5.fca.1.insert.i" = insertvalue %"EXTENT$.btINTVL" undef, i32 %add.2.i, 1
  %rel.1.i = icmp sgt i32 %"A.FIRST$_fetch.1.i", 1
  %slct.1.i = select i1 %rel.1.i, %"EXTENT$.btINTVL" %"var$2_fetch.3.fca.1.insert.i", %"EXTENT$.btINTVL" %"var$3_fetch.5.fca.1.insert.i"
  %slct.1.fca.0.extract.i = extractvalue %"EXTENT$.btINTVL" %slct.1.i, 0
  %slct.1.fca.1.extract.i = extractvalue %"EXTENT$.btINTVL" %slct.1.i, 1
  %slct.1.fca.extractsum.i = add nsw i32 %slct.1.fca.0.extract.i, %slct.1.fca.1.extract.i
  %"var$6[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) nonnull %"var$6", i64 %"$loop_ctr.012")
  store i32 %slct.1.fca.extractsum.i, i32* %"var$6[]", align 1
  %add.3 = add nuw nsw i64 %"$loop_ctr.012", 1
  %exitcond = icmp eq i64 %add.3, %1
  br i1 %exitcond, label %loop_exit11.loopexit, label %loop_body10

loop_exit11.loopexit:                             ; preds = %loop_body10
  br label %loop_exit11

loop_exit11:                                      ; preds = %loop_exit11.loopexit, %alloca_2
  %2 = shl i64 %slct.3, 63
  %sext = ashr exact i64 %2, 63
  %slct.4 = trunc i64 %sext to i32
  ret i32 %slct.4
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare %"EXTENT$.btINTVL"* @"llvm.intel.subscript.p0s_EXTENT$.btINTVLs.i64.i64.p0s_EXTENT$.btINTVLs.i64"(i8, i64, i64, %"EXTENT$.btINTVL"*, i64) #3

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #3

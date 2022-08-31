; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low  < %s -disable-output -xmain-opt-level=3 < %s 2>&1 | FileCheck %s

; This test case checks that the debug information is set correctly after
; unswitching the select instructions. It was created from the following
; test case:

;  subroutine loop_merge_compute(nproma, nlev, nblk, &
;          i_startblk, i_endblk, &
;          i_startlev, i_endlev, &
;          i_startidx, i_endidx, &
;          blkidx, array)
;    implicit none
;    integer, intent(in) :: nproma, nlev, nblk
;    integer, intent(in) :: i_startblk, i_endblk
;    integer, intent(in) :: i_startlev, i_endlev
;    integer, intent(in) :: i_startidx, i_endidx
;
;    integer, intent(in) :: blkidx(nlev,nblk)
;    real(kind=real64) :: array(nproma, nlev, nblk)
;
;    logical :: blkidx_pos
;
;    integer :: jb, jk, jc, jbind
;    real(kind=real64) :: add1, add2, add3
;
;    do jb = i_startblk, i_endblk
;      ! Vectorize along the grid levels, indices blkidx(:,jb) are identical
;      do jk = i_startlev, i_endlev
;        do jc = i_startidx, i_endidx
;          blkidx_pos = blkidx(jk,jb) > 0
;          jbind = MERGE(blkidx(jk,jb), 1, blkidx_pos)
;          add1 = MERGE(array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add2 = MERGE(-array(jc,jk,jb), real(0,real64), blkidx_pos)
;          add3 = MERGE(real(1.0,real64)*array(jc,jk,jb), real(0,real64), blkidx_pos)
;          array(jc,jk,jbind) = add1 + add2 + add3
;        end do
;      end do
;    end do
;
;  end subroutine loop_merge_compute

; HIR before transformation

;   BEGIN REGION { }
;         + DO i1 = 0, sext.i32.i64((1 + %I_ENDBLK_fetch.104)) + -1 * sext.i32.i64(%I_STARTBLK_fetch.103) + -1, 1   <DO_LOOP>
;         |   if (%I_ENDLEV_fetch.107 >= %I_STARTLEV_fetch.106)
;         |   {
;         |      + DO i2 = 0, sext.i32.i64((1 + %I_ENDLEV_fetch.107)) + -1 * sext.i32.i64(%I_STARTLEV_fetch.106) + -1, 1   <DO_LOOP>
;         |      |   if (%I_ENDIDX_fetch.110 >= %I_STARTIDX_fetch.109)
;         |      |   {
;         |      |      %"BLKIDX[][]_fetch.118" = (%BLKIDX)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103) + -1][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1];
;         |      |      %slct.7 = (%"BLKIDX[][]_fetch.118" > 0) ? %"BLKIDX[][]_fetch.118" : 1;
;         |      |
;         |      |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
;         |      |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
;         |      |      |   %slct.8 = (%"BLKIDX[][]_fetch.118" > 0) ? %"ARRAY[][][]_fetch.137" : 0.000000e+00;
;         |      |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
;         |      |      |   %slct.9 = (%"BLKIDX[][]_fetch.118" > 0) ? %neg.1 : 0.000000e+00;
;         |      |      |   %add.26 = %slct.8  +  %slct.9;
;         |      |      |   %add.27 = %slct.8  +  %add.26;
;         |      |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
;         |      |      + END LOOP
;         |      |   }
;         |      + END LOOP
;         |   }
;         + END LOOP
;   END REGION

; HIR after transformation

;  BEGIN REGION { modified }
;        if (%I_ENDLEV_fetch.107 >= %I_STARTLEV_fetch.106)
;        {
;           if (%I_ENDIDX_fetch.110 >= %I_STARTIDX_fetch.109)
;           {
;              + DO i1 = 0, sext.i32.i64((1 + %I_ENDBLK_fetch.104)) + -1 * sext.i32.i64(%I_STARTBLK_fetch.103) + -1, 1   <DO_LOOP>
;              |   + DO i2 = 0, sext.i32.i64((1 + %I_ENDLEV_fetch.107)) + -1 * sext.i32.i64(%I_STARTLEV_fetch.106) + -1, 1   <DO_LOOP>
;              |   |   %"BLKIDX[][]_fetch.118" = (%BLKIDX)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103) + -1][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1];
;              |   |   %slct.7 = (%"BLKIDX[][]_fetch.118" > 0) ? %"BLKIDX[][]_fetch.118" : 1;
;              |   |   if (%"BLKIDX[][]_fetch.118" > 0)
;              |   |   {
;              |   |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
;              |   |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
;              |   |      |   %slct.8 = %"ARRAY[][][]_fetch.137";
;              |   |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
;              |   |      |   %slct.9 = %neg.1;
;              |   |      |   %add.26 = %slct.8  +  %slct.9;
;              |   |      |   %add.27 = %slct.8  +  %add.26;
;              |   |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
;              |   |      + END LOOP
;              |   |   }
;              |   |   else
;              |   |   {
;              |   |      + DO i3 = 0, sext.i32.i64((1 + %I_ENDIDX_fetch.110)) + -1 * sext.i32.i64(%I_STARTIDX_fetch.109) + -1, 1   <DO_LOOP>
;              |   |      |   %"ARRAY[][][]_fetch.137" = (%ARRAY)[i1 + sext.i32.i64(%I_STARTBLK_fetch.103)][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1];
;              |   |      |   %slct.8 = 0.000000e+00;
;              |   |      |   %neg.1 =  - %"ARRAY[][][]_fetch.137";
;              |   |      |   %slct.9 = 0.000000e+00;
;              |   |      |   %add.26 = %slct.8  +  %slct.9;
;              |   |      |   %add.27 = %slct.8  +  %add.26;
;              |   |      |   (%ARRAY)[%slct.7][i2 + sext.i32.i64(%I_STARTLEV_fetch.106) + -1][i3 + sext.i32.i64(%I_STARTIDX_fetch.109) + -1] = %add.27;
;              |   |      + END LOOP
;              |   |   }
;              |   + END LOOP
;              + END LOOP
;           }
;        }
;  END REGION

; Check that the debug information is correct

; CHECK:   LOOP BEGIN at loop_merge_mod.F90 (62, 18)
; CHECK:   <Predicate Optimized v4>
; CHECK:   LOOP END
; CHECK:   LOOP BEGIN at loop_merge_mod.F90 (62, 18)
; CHECK:   <Predicate Optimized v3>
; CHECK:       remark #25422: Invariant Condition at lines 62 and 61 hoisted out of this loop
; CHECK:   LOOP END


define void @loop_merge_mod_mp_loop_merge_compute_(i32* noalias nocapture readonly dereferenceable(4) %NPROMA, i32* noalias nocapture readonly dereferenceable(4) %NLEV, i32* noalias nocapture readonly dereferenceable(4) %NBLK, i32* noalias nocapture readonly dereferenceable(4) %I_STARTBLK, i32* noalias nocapture readonly dereferenceable(4) %I_ENDBLK, i32* noalias nocapture readonly dereferenceable(4) %I_STARTLEV, i32* noalias nocapture readonly dereferenceable(4) %I_ENDLEV, i32* noalias nocapture readonly dereferenceable(4) %I_STARTIDX, i32* noalias nocapture readonly dereferenceable(4) %I_ENDIDX, i32* noalias nocapture readonly dereferenceable(4) %BLKIDX, double* noalias nocapture dereferenceable(8) %ARRAY) !dbg !141 !llfort.type_idx !178 {
alloca_3:
  call void @llvm.dbg.declare(metadata i32* %NPROMA, metadata !143, metadata !DIExpression()), !dbg !179
  call void @llvm.dbg.declare(metadata i32* %NLEV, metadata !144, metadata !DIExpression()), !dbg !180
  call void @llvm.dbg.declare(metadata i32* %NBLK, metadata !145, metadata !DIExpression()), !dbg !181
  call void @llvm.dbg.declare(metadata i32* %I_STARTBLK, metadata !146, metadata !DIExpression()), !dbg !182
  call void @llvm.dbg.declare(metadata i32* %I_ENDBLK, metadata !147, metadata !DIExpression()), !dbg !183
  call void @llvm.dbg.declare(metadata i32* %I_STARTLEV, metadata !148, metadata !DIExpression()), !dbg !184
  call void @llvm.dbg.declare(metadata i32* %I_ENDLEV, metadata !149, metadata !DIExpression()), !dbg !185
  call void @llvm.dbg.declare(metadata i32* %I_STARTIDX, metadata !150, metadata !DIExpression()), !dbg !186
  call void @llvm.dbg.declare(metadata i32* %I_ENDIDX, metadata !151, metadata !DIExpression()), !dbg !187
  call void @llvm.dbg.declare(metadata i32* %BLKIDX, metadata !155, metadata !DIExpression()), !dbg !188
  call void @llvm.dbg.declare(metadata double* %ARRAY, metadata !163, metadata !DIExpression()), !dbg !189
  %NLEV_fetch.100 = load i32, i32* %NLEV, !dbg !190, !llfort.type_idx !196
  %NPROMA_fetch.102 = load i32, i32* %NPROMA, !dbg !190, !llfort.type_idx !199
  call void @llvm.dbg.value(metadata i32 %NLEV_fetch.100, metadata !152, metadata !DIExpression(DW_OP_LLVM_convert, 32, DW_ATE_signed, DW_OP_LLVM_convert, 64, DW_ATE_signed, DW_OP_stack_value)), !dbg !200
  call void @llvm.dbg.value(metadata i32 undef, metadata !154, metadata !DIExpression(DW_OP_LLVM_convert, 32, DW_ATE_signed, DW_OP_LLVM_convert, 64, DW_ATE_signed, DW_OP_stack_value)), !dbg !200
  call void @llvm.dbg.value(metadata i32 %NPROMA_fetch.102, metadata !160, metadata !DIExpression(DW_OP_LLVM_convert, 32, DW_ATE_signed, DW_OP_LLVM_convert, 64, DW_ATE_signed, DW_OP_stack_value)), !dbg !200
  call void @llvm.dbg.value(metadata i32 %NLEV_fetch.100, metadata !161, metadata !DIExpression(DW_OP_LLVM_convert, 32, DW_ATE_signed, DW_OP_LLVM_convert, 64, DW_ATE_signed, DW_OP_stack_value)), !dbg !200
  call void @llvm.dbg.value(metadata i32 undef, metadata !162, metadata !DIExpression(DW_OP_LLVM_convert, 32, DW_ATE_signed, DW_OP_LLVM_convert, 64, DW_ATE_signed, DW_OP_stack_value)), !dbg !200
  %int_sext5 = sext i32 %NLEV_fetch.100 to i64, !dbg !201, !llfort.type_idx !29
  %mul.14 = shl nsw i64 %int_sext5, 2, !dbg !201
  %int_sext13 = sext i32 %NPROMA_fetch.102 to i64, !dbg !202, !llfort.type_idx !29
  %mul.16 = shl nsw i64 %int_sext13, 3, !dbg !202
  %mul.17 = mul nsw i64 %mul.16, %int_sext5, !dbg !202
  %I_STARTBLK_fetch.103 = load i32, i32* %I_STARTBLK, !dbg !202, !llfort.type_idx !205
  %I_ENDBLK_fetch.104 = load i32, i32* %I_ENDBLK, !dbg !202, !llfort.type_idx !208
  call void @llvm.dbg.value(metadata i32 %I_STARTBLK_fetch.103, metadata !177, metadata !DIExpression()), !dbg !200
  %rel.25 = icmp slt i32 %I_ENDBLK_fetch.104, %I_STARTBLK_fetch.103, !dbg !202
  br i1 %rel.25, label %bb23, label %bb22.preheader, !dbg !202

bb22.preheader:                                   ; preds = %alloca_3
  %I_STARTLEV_fetch.106 = load i32, i32* %I_STARTLEV, !dbg !209, !llfort.type_idx !212
  %I_ENDLEV_fetch.107 = load i32, i32* %I_ENDLEV, !dbg !209, !llfort.type_idx !215
  %rel.26 = icmp slt i32 %I_ENDLEV_fetch.107, %I_STARTLEV_fetch.106, !dbg !209
  %I_STARTIDX_fetch.109 = load i32, i32* %I_STARTIDX, !dbg !216
  %I_ENDIDX_fetch.110 = load i32, i32* %I_ENDIDX, !dbg !216
  %rel.27 = icmp slt i32 %I_ENDIDX_fetch.110, %I_STARTIDX_fetch.109, !dbg !216
  %0 = sext i32 %I_STARTIDX_fetch.109 to i64, !dbg !209
  %1 = add nsw i32 %I_ENDIDX_fetch.110, 1, !dbg !209
  %2 = sext i32 %I_STARTLEV_fetch.106 to i64, !dbg !209
  %3 = add nsw i32 %I_ENDLEV_fetch.107, 1, !dbg !209
  %4 = sext i32 %I_STARTBLK_fetch.103 to i64, !dbg !209
  %5 = add nsw i32 %I_ENDBLK_fetch.104, 1, !dbg !209
  %wide.trip.count87 = sext i32 %5 to i64, !dbg !217
  %wide.trip.count83 = sext i32 %3 to i64, !dbg !218
  %wide.trip.count = sext i32 %1 to i64, !dbg !219
  br label %bb22, !dbg !209

bb22:                                             ; preds = %bb22.preheader, %bb27
  %indvars.iv85 = phi i64 [ %4, %bb22.preheader ], [ %indvars.iv.next86, %bb27 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv85, metadata !177, metadata !DIExpression()), !dbg !200
  call void @llvm.dbg.value(metadata i32 %I_STARTLEV_fetch.106, metadata !176, metadata !DIExpression()), !dbg !200
  br i1 %rel.26, label %bb27, label %bb26.preheader, !dbg !209

bb26.preheader:                                   ; preds = %bb22
  %"BLKIDX[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.14, i32* nonnull elementtype(i32) %BLKIDX, i64 %indvars.iv85), !dbg !201
  %"ARRAY[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.17, double* nonnull elementtype(double) %ARRAY, i64 %indvars.iv85), !dbg !202
  br label %bb26, !dbg !216

bb26:                                             ; preds = %bb26.preheader, %bb31
  %indvars.iv81 = phi i64 [ %2, %bb26.preheader ], [ %indvars.iv.next82, %bb31 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv81, metadata !176, metadata !DIExpression()), !dbg !200
  call void @llvm.dbg.value(metadata i32 %I_STARTIDX_fetch.109, metadata !173, metadata !DIExpression()), !dbg !200
  br i1 %rel.27, label %bb31, label %bb30.preheader, !dbg !216

bb30.preheader:                                   ; preds = %bb26
  %"BLKIDX[][]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"BLKIDX[]", i64 %indvars.iv81), !dbg !201, !llfort.type_idx !220
  %"BLKIDX[][]_fetch.118" = load i32, i32* %"BLKIDX[][]", !dbg !201, !llfort.type_idx !220
  %rel.28 = icmp sgt i32 %"BLKIDX[][]_fetch.118", 0, !dbg !223
  %slct.7 = select i1 %rel.28, i32 %"BLKIDX[][]_fetch.118", i32 1, !dbg !224
  %"ARRAY[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.16, double* nonnull elementtype(double) %"ARRAY[]", i64 %indvars.iv81), !dbg !202, !llfort.type_idx !225
  %int_sext33 = zext i32 %slct.7 to i64, !dbg !226
  %"ARRAY[]34" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.17, double* nonnull elementtype(double) %ARRAY, i64 %int_sext33), !dbg !226, !llfort.type_idx !227
  %"ARRAY[][]35" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul.16, double* nonnull elementtype(double) %"ARRAY[]34", i64 %indvars.iv81), !dbg !226, !llfort.type_idx !228
  br label %bb30, !dbg !219

bb30:                                             ; preds = %bb30.preheader, %bb30
  %indvars.iv = phi i64 [ %0, %bb30.preheader ], [ %indvars.iv.next, %bb30 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !173, metadata !DIExpression()), !dbg !200
  call void @llvm.dbg.value(metadata i32 undef, metadata !174, metadata !DIExpression()), !dbg !200
  call void @llvm.dbg.value(metadata i32 %slct.7, metadata !172, metadata !DIExpression()), !dbg !200
  %"ARRAY[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"ARRAY[][]", i64 %indvars.iv), !dbg !202, !llfort.type_idx !229
  %"ARRAY[][][]_fetch.137" = load double, double* %"ARRAY[][][]", !dbg !202, !llfort.type_idx !229
  %slct.8 = select i1 %rel.28, double %"ARRAY[][][]_fetch.137", double 0.000000e+00, !dbg !232
  call void @llvm.dbg.value(metadata double %slct.8, metadata !171, metadata !DIExpression()), !dbg !200
  %neg.1 = fneg double %"ARRAY[][][]_fetch.137", !dbg !233
  %slct.9 = select i1 %rel.28, double %neg.1, double 0.000000e+00, !dbg !234
  call void @llvm.dbg.value(metadata double %slct.9, metadata !170, metadata !DIExpression()), !dbg !200
  call void @llvm.dbg.value(metadata double %slct.8, metadata !169, metadata !DIExpression()), !dbg !200
  %add.26 = fadd double %slct.8, %slct.9, !dbg !235
  %add.27 = fadd double %slct.8, %add.26, !dbg !226
  %"ARRAY[][][]36" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"ARRAY[][]35", i64 %indvars.iv), !dbg !226, !llfort.type_idx !236
  store double %add.27, double* %"ARRAY[][][]36", !dbg !237
  %indvars.iv.next = add nsw i64 %indvars.iv, 1, !dbg !219
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !173, metadata !DIExpression()), !dbg !200
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !219
  br i1 %exitcond, label %bb31.loopexit, label %bb30, !dbg !219

bb31.loopexit:                                    ; preds = %bb30
  br label %bb31, !dbg !218

bb31:                                             ; preds = %bb31.loopexit, %bb26
  %indvars.iv.next82 = add nsw i64 %indvars.iv81, 1, !dbg !218
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next82, metadata !176, metadata !DIExpression()), !dbg !200
  %exitcond84 = icmp eq i64 %indvars.iv.next82, %wide.trip.count83, !dbg !218
  br i1 %exitcond84, label %bb27.loopexit, label %bb26, !dbg !218

bb27.loopexit:                                    ; preds = %bb31
  br label %bb27, !dbg !217

bb27:                                             ; preds = %bb27.loopexit, %bb22
  %indvars.iv.next86 = add nsw i64 %indvars.iv85, 1, !dbg !217
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next86, metadata !177, metadata !DIExpression()), !dbg !200
  %exitcond88 = icmp eq i64 %indvars.iv.next86, %wide.trip.count87, !dbg !217
  br i1 %exitcond88, label %bb23.loopexit, label %bb22, !dbg !217

bb23.loopexit:                                    ; preds = %bb27
  br label %bb23, !dbg !238

bb23:                                             ; preds = %bb23.loopexit, %alloca_3
  ret void, !dbg !238
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { nofree nosync nounwind readnone speculatable }
attributes #1 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }


!llvm.module.flags = !{!0, !1, !2}
!llvm.dbg.cu = !{!3}
!omp_offload.info = !{}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"TraceBack", i32 1}
!3 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !4, producer: "Intel(R) Fortran 22.0-1849", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!4 = !DIFile(filename: "loop_merge_mod.F90", directory: "/localdisk2/ayrivera/dev-new-select-unswitch/test/loop_merge")
!5 = !{i64 67}
!6 = distinct !DISubprogram(name: "loop_merge_init", linkageName: "loop_merge_mod_mp_loop_merge_init_", scope: !7, file: !4, line: 6, type: !8, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !10)
!7 = !DIModule(scope: null, name: "loop_merge_mod", file: !4, line: 1)
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !{!11, !13, !14, !15, !20, !25, !26}
!11 = !DILocalVariable(name: "nproma", arg: 1, scope: !6, file: !4, line: 6, type: !12)
!12 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "nlev", arg: 2, scope: !6, file: !4, line: 6, type: !12)
!14 = !DILocalVariable(name: "nblk", arg: 3, scope: !6, file: !4, line: 6, type: !12)
!15 = !DILocalVariable(name: "blkidx", arg: 4, scope: !6, file: !4, line: 6, type: !16)
!16 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, elements: !17, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!17 = !{!18, !19}
!18 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 64, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 48, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 56, DW_OP_deref))
!19 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 88, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 72, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 80, DW_OP_deref))
!20 = !DILocalVariable(name: "array", arg: 5, scope: !6, file: !4, line: 6, type: !21)
!21 = !DICompositeType(tag: DW_TAG_array_type, baseType: !22, elements: !23, dataLocation: !DIExpression(DW_OP_push_object_address, DW_OP_deref), allocated: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 24, DW_OP_deref, DW_OP_constu, 1, DW_OP_and))
!22 = !DIBasicType(name: "REAL*8", size: 64, encoding: DW_ATE_float)
!23 = !{!18, !19, !24}
!24 = !DISubrange(lowerBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 112, DW_OP_deref), upperBound: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 112, DW_OP_deref, DW_OP_push_object_address, DW_OP_plus_uconst, 96, DW_OP_deref, DW_OP_plus, DW_OP_constu, 1, DW_OP_minus), stride: !DIExpression(DW_OP_push_object_address, DW_OP_plus_uconst, 104, DW_OP_deref))
!25 = !DILocalVariable(name: "jk", scope: !6, file: !4, line: 13, type: !12)
!26 = !DILocalVariable(name: "jb", scope: !6, file: !4, line: 13, type: !12)
!27 = !{i64 93}
!28 = !DILocation(line: 6, column: 30, scope: !6)
!29 = !{i64 3}
!30 = !DILocation(line: 6, column: 38, scope: !6)
!31 = !DILocation(line: 6, column: 44, scope: !6)
!32 = !DILocation(line: 6, column: 50, scope: !6)
!33 = !DILocation(line: 6, column: 58, scope: !6)
!34 = !DILocation(line: 15, column: 5, scope: !6)
!35 = !{!36, !37, i64 24}
!36 = !{!"ifx$descr$1", !37, i64 0, !37, i64 8, !37, i64 16, !37, i64 24, !37, i64 32, !37, i64 40, !37, i64 48, !37, i64 56, !37, i64 64, !37, i64 72, !37, i64 80, !37, i64 88}
!37 = !{!"ifx$descr$field", !38, i64 0}
!38 = !{!"Fortran Dope Vector Symbol", !39, i64 0}
!39 = !{!"Generic Fortran Symbol", !40, i64 0}
!40 = !{!"ifx$root$1$loop_merge_mod_mp_loop_merge_init_"}
!41 = !{i64 75}
!42 = !{!36, !37, i64 40}
!43 = !{!36, !37, i64 8}
!44 = !{!36, !37, i64 32}
!45 = !{!36, !37, i64 16}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$1", !48, i64 0}
!48 = !{!"Fortran Data Symbol", !39, i64 0}
!49 = !{i64 69}
!50 = !{i64 136}
!51 = !{!36, !37, i64 64}
!52 = !{i64 138}
!53 = !{!36, !37, i64 48}
!54 = !{!55, !55, i64 0}
!55 = !{!"ifx$unique_sym$2", !48, i64 0}
!56 = !{i64 70}
!57 = !{i64 139}
!58 = !{i64 140}
!59 = !{i64 141}
!60 = !{!36, !37, i64 56}
!61 = !{i64 142}
!62 = !{i64 2}
!63 = !{!48, !48, i64 0}
!64 = !{i64 147}
!65 = !{!66, !37, i64 24}
!66 = !{!"ifx$descr$2", !37, i64 0, !37, i64 8, !37, i64 16, !37, i64 24, !37, i64 32, !37, i64 40, !37, i64 48, !37, i64 56, !37, i64 64, !37, i64 72, !37, i64 80, !37, i64 88, !37, i64 96, !37, i64 104, !37, i64 112}
!67 = !{i64 86}
!68 = !{!66, !37, i64 40}
!69 = !{!66, !37, i64 8}
!70 = !{!66, !37, i64 32}
!71 = !{!66, !37, i64 16}
!72 = !{!73, !73, i64 0}
!73 = !{!"ifx$unique_sym$3", !48, i64 0}
!74 = !{i64 68}
!75 = !{i64 152}
!76 = !{!66, !37, i64 64}
!77 = !{i64 153}
!78 = !{!66, !37, i64 48}
!79 = !{i64 154}
!80 = !{i64 155}
!81 = !{i64 156}
!82 = !{i64 157}
!83 = !{i64 158}
!84 = !{!66, !37, i64 56}
!85 = !{i64 159}
!86 = !{i64 160}
!87 = !{i64 164}
!88 = !DILocation(line: 0, scope: !6)
!89 = !DILocation(line: 18, column: 5, scope: !6)
!90 = !DILocation(line: 19, column: 7, scope: !6)
!91 = !DILocation(line: 20, column: 9, scope: !6)
!92 = !DILocation(line: 22, column: 5, scope: !6)
!93 = !DILocation(line: 21, column: 7, scope: !6)
!94 = !{i64 177}
!95 = !{i64 178}
!96 = !{!97, !97, i64 0}
!97 = !{!"ifx$unique_sym$6", !48, i64 0}
!98 = !DILocation(line: 23, column: 5, scope: !6)
!99 = !{!66, !37, i64 0}
!100 = !{i64 6}
!101 = !{i64 179}
!102 = !{i64 1, i64 -9223372036854775808}
!103 = !{i64 184}
!104 = !{i64 185}
!105 = !{i64 190}
!106 = !{i64 191}
!107 = !{i64 194}
!108 = !{i64 188}
!109 = !{i64 182}
!110 = !{i64 203}
!111 = !{!112, !112, i64 0}
!112 = !{!"ifx$unique_sym$7", !48, i64 0}
!113 = !{i64 202}
!114 = !DILocation(line: 24, column: 3, scope: !6)
!115 = distinct !DISubprogram(name: "loop_merge_finalize", linkageName: "loop_merge_mod_mp_loop_merge_finalize_", scope: !7, file: !4, line: 26, type: !8, scopeLine: 26, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !116)
!116 = !{!117, !118}
!117 = !DILocalVariable(name: "blkidx", arg: 1, scope: !115, file: !4, line: 26, type: !16)
!118 = !DILocalVariable(name: "array", arg: 2, scope: !115, file: !4, line: 26, type: !21)
!119 = !{i64 116}
!120 = !DILocation(line: 26, column: 34, scope: !115)
!121 = !DILocation(line: 26, column: 42, scope: !115)
!122 = !DILocation(line: 32, column: 5, scope: !115)
!123 = !{!124, !125, i64 0}
!124 = !{!"ifx$descr$3", !125, i64 0, !125, i64 8, !125, i64 16, !125, i64 24, !125, i64 32, !125, i64 40, !125, i64 48, !125, i64 56, !125, i64 64, !125, i64 72, !125, i64 80, !125, i64 88}
!125 = !{!"ifx$descr$field", !126, i64 0}
!126 = !{!"Fortran Dope Vector Symbol", !127, i64 0}
!127 = !{!"Generic Fortran Symbol", !128, i64 0}
!128 = !{!"ifx$root$2$loop_merge_mod_mp_loop_merge_finalize_"}
!129 = !{!124, !125, i64 24}
!130 = !{i64 98}
!131 = !{!124, !125, i64 40}
!132 = !{i64 100}
!133 = !{i64 11}
!134 = !{!135, !125, i64 0}
!135 = !{!"ifx$descr$4", !125, i64 0, !125, i64 8, !125, i64 16, !125, i64 24, !125, i64 32, !125, i64 40, !125, i64 48, !125, i64 56, !125, i64 64, !125, i64 72, !125, i64 80, !125, i64 88, !125, i64 96, !125, i64 104, !125, i64 112}
!136 = !{!135, !125, i64 24}
!137 = !{i64 109}
!138 = !{!135, !125, i64 40}
!139 = !{i64 111}
!140 = !DILocation(line: 33, column: 3, scope: !115)
!141 = distinct !DISubprogram(name: "loop_merge_compute", linkageName: "loop_merge_mod_mp_loop_merge_compute_", scope: !7, file: !4, line: 35, type: !8, scopeLine: 35, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !142)
!142 = !{!143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !154, !155, !160, !161, !162, !163, !169, !170, !171, !172, !173, !174, !176, !177}
!143 = !DILocalVariable(name: "nproma", arg: 1, scope: !141, file: !4, line: 35, type: !12)
!144 = !DILocalVariable(name: "nlev", arg: 2, scope: !141, file: !4, line: 35, type: !12)
!145 = !DILocalVariable(name: "nblk", arg: 3, scope: !141, file: !4, line: 35, type: !12)
!146 = !DILocalVariable(name: "i_startblk", arg: 4, scope: !141, file: !4, line: 36, type: !12)
!147 = !DILocalVariable(name: "i_endblk", arg: 5, scope: !141, file: !4, line: 36, type: !12)
!148 = !DILocalVariable(name: "i_startlev", arg: 6, scope: !141, file: !4, line: 37, type: !12)
!149 = !DILocalVariable(name: "i_endlev", arg: 7, scope: !141, file: !4, line: 37, type: !12)
!150 = !DILocalVariable(name: "i_startidx", arg: 8, scope: !141, file: !4, line: 38, type: !12)
!151 = !DILocalVariable(name: "i_endidx", arg: 9, scope: !141, file: !4, line: 38, type: !12)
!152 = !DILocalVariable(name: "BLKIDX$1$upperbound", scope: !141, type: !153, flags: DIFlagArtificial)
!153 = !DIBasicType(name: "INTEGER*8", size: 64, encoding: DW_ATE_signed)
!154 = !DILocalVariable(name: "BLKIDX$2$upperbound", scope: !141, type: !153, flags: DIFlagArtificial)
!155 = !DILocalVariable(name: "blkidx", arg: 10, scope: !141, file: !4, line: 39, type: !156)
!156 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, elements: !157)
!157 = !{!158, !159}
!158 = !DISubrange(lowerBound: 1, upperBound: !152)
!159 = !DISubrange(lowerBound: 1, upperBound: !154)
!160 = !DILocalVariable(name: "ARRAY$1$upperbound", scope: !141, type: !153, flags: DIFlagArtificial)
!161 = !DILocalVariable(name: "ARRAY$2$upperbound", scope: !141, type: !153, flags: DIFlagArtificial)
!162 = !DILocalVariable(name: "ARRAY$3$upperbound", scope: !141, type: !153, flags: DIFlagArtificial)
!163 = !DILocalVariable(name: "array", arg: 11, scope: !141, file: !4, line: 39, type: !164)
!164 = !DICompositeType(tag: DW_TAG_array_type, baseType: !22, elements: !165)
!165 = !{!166, !167, !168}
!166 = !DISubrange(lowerBound: 1, upperBound: !160)
!167 = !DISubrange(lowerBound: 1, upperBound: !161)
!168 = !DISubrange(lowerBound: 1, upperBound: !162)
!169 = !DILocalVariable(name: "add3", scope: !141, file: !4, line: 52, type: !22)
!170 = !DILocalVariable(name: "add2", scope: !141, file: !4, line: 52, type: !22)
!171 = !DILocalVariable(name: "add1", scope: !141, file: !4, line: 52, type: !22)
!172 = !DILocalVariable(name: "jbind", scope: !141, file: !4, line: 51, type: !12)
!173 = !DILocalVariable(name: "jc", scope: !141, file: !4, line: 51, type: !12)
!174 = !DILocalVariable(name: "blkidx_pos", scope: !141, file: !4, line: 49, type: !175)
!175 = !DIBasicType(name: "LOGICAL*4", size: 32, encoding: DW_ATE_boolean)
!176 = !DILocalVariable(name: "jk", scope: !141, file: !4, line: 51, type: !12)
!177 = !DILocalVariable(name: "jb", scope: !141, file: !4, line: 51, type: !12)
!178 = !{i64 128}
!179 = !DILocation(line: 35, column: 33, scope: !141)
!180 = !DILocation(line: 35, column: 41, scope: !141)
!181 = !DILocation(line: 35, column: 47, scope: !141)
!182 = !DILocation(line: 36, column: 11, scope: !141)
!183 = !DILocation(line: 36, column: 23, scope: !141)
!184 = !DILocation(line: 37, column: 11, scope: !141)
!185 = !DILocation(line: 37, column: 23, scope: !141)
!186 = !DILocation(line: 38, column: 11, scope: !141)
!187 = !DILocation(line: 38, column: 23, scope: !141)
!188 = !DILocation(line: 39, column: 11, scope: !141)
!189 = !DILocation(line: 39, column: 19, scope: !141)
!190 = !DILocation(line: 35, column: 14, scope: !141)
!191 = !{!192, !192, i64 0}
!192 = !{!"ifx$unique_sym$8", !193, i64 0}
!193 = !{!"Fortran Data Symbol", !194, i64 0}
!194 = !{!"Generic Fortran Symbol", !195, i64 0}
!195 = !{!"ifx$root$3$loop_merge_mod_mp_loop_merge_compute_"}
!196 = !{i64 118}
!197 = !{!198, !198, i64 0}
!198 = !{!"ifx$unique_sym$10", !193, i64 0}
!199 = !{i64 117}
!200 = !DILocation(line: 0, scope: !141)
!201 = !DILocation(line: 59, column: 11, scope: !141)
!202 = !DILocation(line: 61, column: 11, scope: !141)
!203 = !{!204, !204, i64 0}
!204 = !{!"ifx$unique_sym$11", !193, i64 0}
!205 = !{i64 120}
!206 = !{!207, !207, i64 0}
!207 = !{!"ifx$unique_sym$12", !193, i64 0}
!208 = !{i64 121}
!209 = !DILocation(line: 56, column: 7, scope: !141)
!210 = !{!211, !211, i64 0}
!211 = !{!"ifx$unique_sym$14", !193, i64 0}
!212 = !{i64 122}
!213 = !{!214, !214, i64 0}
!214 = !{!"ifx$unique_sym$15", !193, i64 0}
!215 = !{i64 123}
!216 = !DILocation(line: 58, column: 9, scope: !141)
!217 = !DILocation(line: 84, column: 5, scope: !141)
!218 = !DILocation(line: 83, column: 7, scope: !141)
!219 = !DILocation(line: 65, column: 9, scope: !141)
!220 = !{i64 219}
!221 = !{!222, !222, i64 0}
!222 = !{!"ifx$unique_sym$20", !193, i64 0}
!223 = !DILocation(line: 59, column: 38, scope: !141)
!224 = !DILocation(line: 60, column: 19, scope: !141)
!225 = !{i64 223}
!226 = !DILocation(line: 64, column: 44, scope: !141)
!227 = !{i64 231}
!228 = !{i64 232}
!229 = !{i64 224}
!230 = !{!231, !231, i64 0}
!231 = !{!"ifx$unique_sym$23", !193, i64 0}
!232 = !DILocation(line: 61, column: 18, scope: !141)
!233 = !DILocation(line: 62, column: 24, scope: !141)
!234 = !DILocation(line: 62, column: 18, scope: !141)
!235 = !DILocation(line: 64, column: 37, scope: !141)
!236 = !{i64 233}
!237 = !DILocation(line: 64, column: 11, scope: !141)
!238 = !DILocation(line: 86, column: 3, scope: !141)
!239 = !{i32 103}
!240 = !{i64 161}
!241 = !{i32 94}
!242 = !{i64 162}
!243 = !{i32 96}
!244 = !{i64 207}
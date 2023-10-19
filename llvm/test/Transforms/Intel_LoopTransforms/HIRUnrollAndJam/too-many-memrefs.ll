; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -debug-only=hir-unroll-and-jam -disable-output < %s 2>&1 | FileCheck %s

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-unroll-and-jam,print<hir>" -debug-only=hir-unroll-and-jam -hir-unroll-and-jam-max-unrolled-loop-memrefs=32 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=UNDER-THRESHOLD

; Verify that we skip unroll & jam for both i1 and i2 loops due to too many
; memrefs in the loop.

; CHECK: Number of memrefs in loop: 30
; CHECK: Skipping unroll & jam for loop and all its parents as the number of memrefs exceeds threshold!

; i1 loop is already throttled.
; CHECK-NOT: Skipping unroll & jam

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, zext.i32.i64(%ub.new) + -1 * zext.i32.i64(%lb.new), 1   <DO_LOOP> <ivdep>
; CHECK: |      %mg_constants_mod_mp_work__fetch.647 = (@mg_constants_mod_mp_work_)[0].0;
; CHECK: |      %"val$[]_fetch.648" = (@mg_constants_mod_mp_work_)[0].6[0].2;
; CHECK: |      %"val$[]_fetch.650" = (getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 1))[1];
; CHECK: |      %"val$[]_fetch.651" = (getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 2))[1];
; CHECK: |      %"val$[]_fetch.653" = (getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 1))[2];
; CHECK: |      %"val$[]_fetch.654" = (getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 2))[2];
; CHECK: |   + DO i2 = 0, zext.i32.i64((1 + %mg_options_mod_mp_ny__fetch.446)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |      %"var$41_fetch.454" = (%"var$41")[0];
; CHECK: |   |      %"var$42_fetch.456" = (%"var$42")[0];
; CHECK: |   |   + DO i3 = 0, zext.i32.i64((1 + %mg_options_mod_mp_nx__fetch.448)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK: |   |   |   %add.169 = (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 1][i3]  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2][i3];
; CHECK: |   |   |   %add.172 = %add.169  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 2][i3];
; CHECK: |   |   |   %add.174 = %add.172  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2][i3 + 1];
; CHECK: |   |   |   %add.176 = %add.174  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 1][i3 + 1];
; CHECK: |   |   |   %add.179 = %add.176  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 2][i3 + 1];
; CHECK: |   |   |   %add.182 = %add.179  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2][i3 + 2];
; CHECK: |   |   |   %add.185 = %add.182  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 1][i3 + 2];
; CHECK: |   |   |   %add.189 = %add.185  +  (%GRID)[i1 + zext.i32.i64(%lb.new)][i2 + 2][i3 + 2];
; CHECK: |   |   |   %add.153 = %add.189  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2][i3];
; CHECK: |   |   |   %add.155 = %add.153  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 1][i3];
; CHECK: |   |   |   %add.156 = %add.155  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 2][i3];
; CHECK: |   |   |   %add.157 = %add.156  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2][i3 + 1];
; CHECK: |   |   |   %add.159 = %add.157  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 1][i3 + 1];
; CHECK: |   |   |   %add.161 = %add.159  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 2][i3 + 1];
; CHECK: |   |   |   %add.163 = %add.161  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2][i3 + 2];
; CHECK: |   |   |   %add.166 = %add.163  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 1][i3 + 2];
; CHECK: |   |   |   %add.139 = %add.166  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 2][i3 + 2];
; CHECK: |   |   |   %add.141 = %add.139  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2][i3];
; CHECK: |   |   |   %add.142 = %add.141  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 1][i3];
; CHECK: |   |   |   %add.143 = %add.142  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 2][i3];
; CHECK: |   |   |   %add.145 = %add.143  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2][i3 + 1];
; CHECK: |   |   |   %add.147 = %add.145  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 1][i3 + 1];
; CHECK: |   |   |   %add.149 = %add.147  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 2][i3 + 1];
; CHECK: |   |   |   %add.152 = %add.149  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2][i3 + 2];
; CHECK: |   |   |   %add.190 = %add.152  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 1][i3 + 2];
; CHECK: |   |   |   %add.191 = %add.190  +  (%GRID)[i1 + zext.i32.i64(%lb.new) + 2][i2 + 2][i3 + 2];
; CHECK: |   |   |   %div.4 = %add.191  *  0x3FA2F684BDA12F68;
; CHECK: |   |   |   (%mg_constants_mod_mp_work__fetch.647)[i1 + zext.i32.i64(%lb.new) + 1][i2 + 1][i3 + 1] = %div.4;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

; Verify that i2 loop is unrolled when the memrefs are within the threshold.
; UNDER-THRESHOLD: modified

; UNDER-THRESHOLD: DO i2 = 0, %tgu + -1, 1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%struct.ident_t = type { i32, i32, i32, i32, ptr }

@mg_constants_mod_mp_work_ = external local_unnamed_addr global %"QNCA_a0$double*$rank3$"
@mg_options_mod_mp_ny_ = external local_unnamed_addr global i32, align 8
@mg_options_mod_mp_nx_ = external local_unnamed_addr global i32, align 8
@.kmpc_loc.0.0.36 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.38 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nounwind uwtable
define hidden void @mg_stencil_comps_mod_mp_mg_stencil_3d27pt_.DIR.OMP.PARALLEL.LOOP.2.split261(ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture readonly %GRID, i64 %omp.pdo.norm.lb.val.zext, i64 %omp.pdo.norm.ub.0.val, ptr nocapture readonly %"var$41", ptr nocapture readonly %"var$42") {
DIR.OMP.PARALLEL.LOOP.6:
  %t0 = trunc i64 %omp.pdo.norm.ub.0.val to i32
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i32, align 4
  %upper.bnd = alloca i32, align 4
  %stride = alloca i32, align 4
  store i32 0, ptr %is.last, align 4
  %rel.73.not = icmp sgt i32 0, %t0
  br i1 %rel.73.not, label %DIR.OMP.PARALLEL.LOOP.11, label %DIR.PRAGMA.BLOCK_LOOP.4.preheader

DIR.PRAGMA.BLOCK_LOOP.126:                        ; preds = %DIR.PRAGMA.BLOCK_LOOP.5.preheader, %DIR.PRAGMA.END.BLOCK_LOOP.3
  %indvars.iv21 = phi i64 [ %t8, %DIR.PRAGMA.BLOCK_LOOP.5.preheader ], [ %indvars.iv.next22, %DIR.PRAGMA.END.BLOCK_LOOP.3 ]
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  br label %DIR.PRAGMA.BLOCK_LOOP.1

DIR.PRAGMA.BLOCK_LOOP.1:                          ; preds = %DIR.PRAGMA.BLOCK_LOOP.126
  br i1 %rel.74, label %DIR.PRAGMA.END.BLOCK_LOOP.227, label %do.body171.preheader

do.body171.preheader:                             ; preds = %DIR.PRAGMA.BLOCK_LOOP.1
  %t2 = add nuw nsw i64 %indvars.iv21, 2
  %mg_constants_mod_mp_work__fetch.647 = load ptr, ptr @mg_constants_mod_mp_work_, align 8
  %"val$[]_fetch.648" = load i64, ptr %"val$[]", align 1
  %"val$[]_fetch.650" = load i64, ptr %"val$[]167", align 1, !range !2
  %"val$[]_fetch.651" = load i64, ptr %"val$[]168", align 1
  %"val$[]_fetch.653" = load i64, ptr %"val$[]170", align 1, !range !2
  %"val$[]_fetch.654" = load i64, ptr %"val$[]171", align 1
  %"mg_constants_mod_mp_work__fetch.647[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"val$[]_fetch.654", i64 %"val$[]_fetch.653", ptr elementtype(double) %mg_constants_mod_mp_work__fetch.647, i64 %indvars.iv.next22)
  br label %do.body171

do.body171:                                       ; preds = %do.end_do176, %do.body171.preheader
  %indvars.iv15 = phi i64 [ 1, %do.body171.preheader ], [ %indvars.iv.next16.pre-phi, %do.end_do176 ]
  br i1 %rel.75, label %do.body171.do.end_do176_crit_edge, label %do.body175.preheader

do.body171.do.end_do176_crit_edge:                ; preds = %do.body171
  %.pre = add nuw nsw i64 %indvars.iv15, 1
  br label %do.end_do176

do.body175.preheader:                             ; preds = %do.body171
  %"var$41_fetch.454" = load i64, ptr %"var$41", align 8, !tbaa !3, !alias.scope !7, !noalias !12, !llvm.access.group !1
  %t3 = add nsw i64 %indvars.iv15, -1
  %"var$42_fetch.456" = load i64, ptr %"var$42", align 8, !tbaa !3, !alias.scope !31, !noalias !12, !llvm.access.group !1
  %"GRID[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 %"var$42_fetch.456", ptr elementtype(double) %GRID, i64 %indvars.iv21), !llfort.type_idx !35
  %"GRID[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr elementtype(double) %"GRID[]", i64 %t3), !llfort.type_idx !35
  %"GRID[][]14" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr elementtype(double) %"GRID[]", i64 %indvars.iv15), !llfort.type_idx !35
  %t4 = add nuw nsw i64 %indvars.iv15, 1
  %int_sext17 = and i64 %t4, 4294967295
  %"GRID[][]20" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr elementtype(double) %"GRID[]", i64 %int_sext17), !llfort.type_idx !35
  %"GRID[]61" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 %"var$42_fetch.456", ptr elementtype(double) %GRID, i64 %indvars.iv.next22), !llfort.type_idx !35
  %"GRID[][]62" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]61", i64 %t3), !llfort.type_idx !35
  %"GRID[][]68" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]61", i64 %indvars.iv15), !llfort.type_idx !35
  %"GRID[][]74" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]61", i64 %int_sext17), !llfort.type_idx !35
  %"GRID[]115" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 %"var$42_fetch.456", ptr elementtype(double) %GRID, i64 %t2), !llfort.type_idx !35
  %"GRID[][]116" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]115", i64 %t3), !llfort.type_idx !35
  %"GRID[][]122" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]115", i64 %indvars.iv15), !llfort.type_idx !35
  %"GRID[][]128" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %"var$41_fetch.454", ptr nonnull elementtype(double) %"GRID[]115", i64 %int_sext17), !llfort.type_idx !35
  %"mg_constants_mod_mp_work__fetch.647[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"val$[]_fetch.651", i64 %"val$[]_fetch.650", ptr elementtype(double) %"mg_constants_mod_mp_work__fetch.647[]", i64 %indvars.iv15), !llfort.type_idx !36
  br label %do.body175

do.body175:                                       ; preds = %do.body175, %do.body175.preheader
  %indvars.iv = phi i64 [ 1, %do.body175.preheader ], [ %indvars.iv.next, %do.body175 ]
  %t5 = add nsw i64 %indvars.iv, -1
  %"GRID[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %"GRID[][]", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.461" = load double, ptr %"GRID[][][]", align 1, !tbaa !37, !alias.scope !39, !noalias !12, !llvm.access.group !1, !llfort.type_idx !41
  %"GRID[][][]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]14", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.468" = load double, ptr %"GRID[][][]15", align 1, !tbaa !37, !alias.scope !42, !noalias !12, !llvm.access.group !1, !llfort.type_idx !44
  %"GRID[][][]21" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]20", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.475" = load double, ptr %"GRID[][][]21", align 1, !tbaa !37, !alias.scope !45, !noalias !12, !llvm.access.group !1, !llfort.type_idx !47
  %"GRID[][][]27" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %"GRID[][]", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.482" = load double, ptr %"GRID[][][]27", align 1, !tbaa !37, !alias.scope !48, !noalias !12, !llvm.access.group !1, !llfort.type_idx !50
  %"GRID[][][]33" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]14", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.489" = load double, ptr %"GRID[][][]33", align 1, !tbaa !37, !alias.scope !51, !noalias !12, !llvm.access.group !1, !llfort.type_idx !53
  %"GRID[][][]39" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]20", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.496" = load double, ptr %"GRID[][][]39", align 1, !tbaa !37, !alias.scope !54, !noalias !12, !llvm.access.group !1, !llfort.type_idx !56
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %int_sext40 = and i64 %indvars.iv.next, 4294967295
  %"GRID[][][]45" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %"GRID[][]", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.503" = load double, ptr %"GRID[][][]45", align 1, !tbaa !37, !alias.scope !57, !noalias !12, !llvm.access.group !1, !llfort.type_idx !59
  %"GRID[][][]51" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]14", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.510" = load double, ptr %"GRID[][][]51", align 1, !tbaa !37, !alias.scope !60, !noalias !12, !llvm.access.group !1, !llfort.type_idx !62
  %"GRID[][][]57" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]20", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.517" = load double, ptr %"GRID[][][]57", align 1, !tbaa !37, !alias.scope !63, !noalias !12, !llvm.access.group !1, !llfort.type_idx !65
  %"GRID[][][]63" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]62", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.524" = load double, ptr %"GRID[][][]63", align 1, !tbaa !37, !alias.scope !66, !noalias !12, !llvm.access.group !1, !llfort.type_idx !68
  %"GRID[][][]69" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]68", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.531" = load double, ptr %"GRID[][][]69", align 1, !tbaa !37, !alias.scope !69, !noalias !12, !llvm.access.group !1, !llfort.type_idx !71
  %"GRID[][][]75" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]74", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.538" = load double, ptr %"GRID[][][]75", align 1, !tbaa !37, !alias.scope !72, !noalias !12, !llvm.access.group !1, !llfort.type_idx !74
  %"GRID[][][]81" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]62", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.545" = load double, ptr %"GRID[][][]81", align 1, !tbaa !37, !alias.scope !75, !noalias !12, !llvm.access.group !1, !llfort.type_idx !77
  %"GRID[][][]87" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]68", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.552" = load double, ptr %"GRID[][][]87", align 1, !tbaa !37, !alias.scope !78, !noalias !12, !llvm.access.group !1, !llfort.type_idx !80
  %"GRID[][][]93" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]74", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.559" = load double, ptr %"GRID[][][]93", align 1, !tbaa !37, !alias.scope !81, !noalias !12, !llvm.access.group !1, !llfort.type_idx !83
  %"GRID[][][]99" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]62", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.566" = load double, ptr %"GRID[][][]99", align 1, !tbaa !37, !alias.scope !84, !noalias !12, !llvm.access.group !1, !llfort.type_idx !86
  %"GRID[][][]105" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]68", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.573" = load double, ptr %"GRID[][][]105", align 1, !tbaa !37, !alias.scope !87, !noalias !12, !llvm.access.group !1, !llfort.type_idx !89
  %"GRID[][][]111" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]74", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.580" = load double, ptr %"GRID[][][]111", align 1, !tbaa !37, !alias.scope !90, !noalias !12, !llvm.access.group !1, !llfort.type_idx !92
  %"GRID[][][]117" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]116", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.587" = load double, ptr %"GRID[][][]117", align 1, !tbaa !37, !alias.scope !93, !noalias !12, !llvm.access.group !1, !llfort.type_idx !95
  %"GRID[][][]123" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]122", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.594" = load double, ptr %"GRID[][][]123", align 1, !tbaa !37, !alias.scope !96, !noalias !12, !llvm.access.group !1, !llfort.type_idx !98
  %"GRID[][][]129" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]128", i64 %t5), !llfort.type_idx !35
  %"GRID[][][]_fetch.601" = load double, ptr %"GRID[][][]129", align 1, !tbaa !37, !alias.scope !99, !noalias !12, !llvm.access.group !1, !llfort.type_idx !101
  %"GRID[][][]135" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]116", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.608" = load double, ptr %"GRID[][][]135", align 1, !tbaa !37, !alias.scope !102, !noalias !12, !llvm.access.group !1, !llfort.type_idx !104
  %"GRID[][][]141" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]122", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.615" = load double, ptr %"GRID[][][]141", align 1, !tbaa !37, !alias.scope !105, !noalias !12, !llvm.access.group !1, !llfort.type_idx !107
  %"GRID[][][]147" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]128", i64 %indvars.iv), !llfort.type_idx !35
  %"GRID[][][]_fetch.622" = load double, ptr %"GRID[][][]147", align 1, !tbaa !37, !alias.scope !108, !noalias !12, !llvm.access.group !1, !llfort.type_idx !110
  %"GRID[][][]153" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]116", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.629" = load double, ptr %"GRID[][][]153", align 1, !tbaa !37, !alias.scope !111, !noalias !12, !llvm.access.group !1, !llfort.type_idx !113
  %"GRID[][][]159" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]122", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.636" = load double, ptr %"GRID[][][]159", align 1, !tbaa !37, !alias.scope !114, !noalias !12, !llvm.access.group !1, !llfort.type_idx !116
  %"GRID[][][]165" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %"GRID[][]128", i64 %int_sext40), !llfort.type_idx !35
  %"GRID[][][]_fetch.643" = load double, ptr %"GRID[][][]165", align 1, !tbaa !37, !alias.scope !117, !noalias !12, !llvm.access.group !1, !llfort.type_idx !119
  %add.169 = fadd reassoc ninf nsz arcp contract afn double %"GRID[][][]_fetch.468", %"GRID[][][]_fetch.461"
  %add.172 = fadd reassoc ninf nsz arcp contract afn double %add.169, %"GRID[][][]_fetch.475"
  %add.174 = fadd reassoc ninf nsz arcp contract afn double %add.172, %"GRID[][][]_fetch.482"
  %add.176 = fadd reassoc ninf nsz arcp contract afn double %add.174, %"GRID[][][]_fetch.489"
  %add.179 = fadd reassoc ninf nsz arcp contract afn double %add.176, %"GRID[][][]_fetch.496"
  %add.182 = fadd reassoc ninf nsz arcp contract afn double %add.179, %"GRID[][][]_fetch.503"
  %add.185 = fadd reassoc ninf nsz arcp contract afn double %add.182, %"GRID[][][]_fetch.510"
  %add.189 = fadd reassoc ninf nsz arcp contract afn double %add.185, %"GRID[][][]_fetch.517"
  %add.153 = fadd reassoc ninf nsz arcp contract afn double %add.189, %"GRID[][][]_fetch.524"
  %add.155 = fadd reassoc ninf nsz arcp contract afn double %add.153, %"GRID[][][]_fetch.531"
  %add.156 = fadd reassoc ninf nsz arcp contract afn double %add.155, %"GRID[][][]_fetch.538"
  %add.157 = fadd reassoc ninf nsz arcp contract afn double %add.156, %"GRID[][][]_fetch.545"
  %add.159 = fadd reassoc ninf nsz arcp contract afn double %add.157, %"GRID[][][]_fetch.552"
  %add.161 = fadd reassoc ninf nsz arcp contract afn double %add.159, %"GRID[][][]_fetch.559"
  %add.163 = fadd reassoc ninf nsz arcp contract afn double %add.161, %"GRID[][][]_fetch.566"
  %add.166 = fadd reassoc ninf nsz arcp contract afn double %add.163, %"GRID[][][]_fetch.573"
  %add.139 = fadd reassoc ninf nsz arcp contract afn double %add.166, %"GRID[][][]_fetch.580"
  %add.141 = fadd reassoc ninf nsz arcp contract afn double %add.139, %"GRID[][][]_fetch.587"
  %add.142 = fadd reassoc ninf nsz arcp contract afn double %add.141, %"GRID[][][]_fetch.594"
  %add.143 = fadd reassoc ninf nsz arcp contract afn double %add.142, %"GRID[][][]_fetch.601"
  %add.145 = fadd reassoc ninf nsz arcp contract afn double %add.143, %"GRID[][][]_fetch.608"
  %add.147 = fadd reassoc ninf nsz arcp contract afn double %add.145, %"GRID[][][]_fetch.615"
  %add.149 = fadd reassoc ninf nsz arcp contract afn double %add.147, %"GRID[][][]_fetch.622"
  %add.152 = fadd reassoc ninf nsz arcp contract afn double %add.149, %"GRID[][][]_fetch.629"
  %add.190 = fadd reassoc ninf nsz arcp contract afn double %add.152, %"GRID[][][]_fetch.636"
  %add.191 = fadd reassoc ninf nsz arcp contract afn double %add.190, %"GRID[][][]_fetch.643"
  %div.4 = fmul reassoc ninf nsz arcp contract afn double %add.191, 0x3FA2F684BDA12F68
  %"mg_constants_mod_mp_work__fetch.647[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]_fetch.648", i64 8, ptr elementtype(double) %"mg_constants_mod_mp_work__fetch.647[][]", i64 %indvars.iv), !llfort.type_idx !36
  store double %div.4, ptr %"mg_constants_mod_mp_work__fetch.647[][][]", align 1, !tbaa !120, !alias.scope !122, !noalias !123, !llvm.access.group !1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %do.end_do176.loopexit, label %do.body175

do.end_do176.loopexit:                            ; preds = %do.body175
  br label %do.end_do176

do.end_do176:                                     ; preds = %do.end_do176.loopexit, %do.body171.do.end_do176_crit_edge
  %indvars.iv.next16.pre-phi = phi i64 [ %.pre, %do.body171.do.end_do176_crit_edge ], [ %t4, %do.end_do176.loopexit ]
  %exitcond20.not = icmp eq i64 %indvars.iv.next16.pre-phi, %wide.trip.count19
  br i1 %exitcond20.not, label %DIR.PRAGMA.END.BLOCK_LOOP.227.loopexit, label %do.body171

DIR.PRAGMA.END.BLOCK_LOOP.227.loopexit:           ; preds = %do.end_do176
  br label %DIR.PRAGMA.END.BLOCK_LOOP.227

DIR.PRAGMA.END.BLOCK_LOOP.227:                    ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.227.loopexit, %DIR.PRAGMA.BLOCK_LOOP.1
  br label %DIR.PRAGMA.END.BLOCK_LOOP.3

DIR.PRAGMA.END.BLOCK_LOOP.3:                      ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.227
  %exitcond25 = icmp eq i64 %indvars.iv.next22, %wide.trip.count24
  br i1 %exitcond25, label %loop.region.exit.loopexit, label %DIR.PRAGMA.BLOCK_LOOP.126, !llvm.loop !134

DIR.PRAGMA.BLOCK_LOOP.4.preheader:                ; preds = %DIR.OMP.PARALLEL.LOOP.6
  %my.tid = load i32, ptr %tid, align 4
  store i32 0, ptr %lower.bnd, align 4
  store i32 %t0, ptr %upper.bnd, align 4
  store i32 1, ptr %stride, align 4
  %lb.new = load i32, ptr %lower.bnd, align 4, !range !137
  %ub.new = load i32, ptr %upper.bnd, align 4, !range !137
  %omp.ztt.not = icmp ugt i32 %lb.new, %ub.new
  br i1 %omp.ztt.not, label %loop.region.exit, label %DIR.PRAGMA.BLOCK_LOOP.5.preheader

DIR.PRAGMA.BLOCK_LOOP.5.preheader:                ; preds = %DIR.PRAGMA.BLOCK_LOOP.4.preheader
  %mg_options_mod_mp_ny__fetch.446 = load i32, ptr @mg_options_mod_mp_ny_, align 8
  %rel.74 = icmp slt i32 %mg_options_mod_mp_ny__fetch.446, 1
  %mg_options_mod_mp_nx__fetch.448 = load i32, ptr @mg_options_mod_mp_nx_, align 8
  %rel.75 = icmp slt i32 %mg_options_mod_mp_nx__fetch.448, 1
  %"val$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %"val$[]167" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %"val$[]168" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %"val$[]170" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 1), i32 2)
  %"val$[]171" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$double*$rank3$", ptr @mg_constants_mod_mp_work_, i64 0, i32 6, i64 0, i32 2), i32 2)
  %t6 = add i32 %mg_options_mod_mp_nx__fetch.448, 1
  %t7 = add i32 %mg_options_mod_mp_ny__fetch.446, 1
  %t8 = zext i32 %lb.new to i64
  %t9 = add nuw nsw i32 %ub.new, 1
  %wide.trip.count24 = zext i32 %t9 to i64
  %wide.trip.count19 = zext i32 %t7 to i64
  %wide.trip.count = zext i32 %t6 to i64
  br label %DIR.PRAGMA.BLOCK_LOOP.126

loop.region.exit.loopexit:                        ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.3
  br label %loop.region.exit

loop.region.exit:                                 ; preds = %loop.region.exit.loopexit, %DIR.PRAGMA.BLOCK_LOOP.4.preheader
  br label %DIR.OMP.PARALLEL.LOOP.11

DIR.OMP.PARALLEL.LOOP.11:                         ; preds = %loop.region.exit, %DIR.OMP.PARALLEL.LOOP.6
  ret void
}

attributes #1 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}

!0 = !{i32 7, !"openmp", i32 50}
!1 = distinct !{}
!2 = !{i64 1, i64 -9223372036854775808}
!3 = !{!4, !4, i64 0}
!4 = !{!"Fortran Data Symbol", !5, i64 0}
!5 = !{!"Generic Fortran Symbol", !6, i64 0}
!6 = !{!"ifx$root$4$mg_stencil_comps_mod_mp_mg_stencil_3d27pt_"}
!7 = !{!8, !10, !11}
!8 = distinct !{!8, !9, !"OMPAliasScope"}
!9 = distinct !{!9, !"OMPDomain"}
!10 = distinct !{!10, !9, !"OMPAliasScope"}
!11 = distinct !{!11, !9, !"OMPAliasScope"}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30}
!13 = distinct !{!13, !9, !"OMPAliasScope"}
!14 = distinct !{!14, !9, !"OMPAliasScope"}
!15 = distinct !{!15, !9, !"OMPAliasScope"}
!16 = distinct !{!16, !9, !"OMPAliasScope"}
!17 = distinct !{!17, !9, !"OMPAliasScope"}
!18 = distinct !{!18, !9, !"OMPAliasScope"}
!19 = distinct !{!19, !9, !"OMPAliasScope"}
!20 = distinct !{!20, !9, !"OMPAliasScope"}
!21 = distinct !{!21, !9, !"OMPAliasScope"}
!22 = distinct !{!22, !9, !"OMPAliasScope"}
!23 = distinct !{!23, !9, !"OMPAliasScope"}
!24 = distinct !{!24, !9, !"OMPAliasScope"}
!25 = distinct !{!25, !9, !"OMPAliasScope"}
!26 = distinct !{!26, !9, !"OMPAliasScope"}
!27 = distinct !{!27, !9, !"OMPAliasScope"}
!28 = distinct !{!28, !9, !"OMPAliasScope"}
!29 = distinct !{!29, !9, !"OMPAliasScope"}
!30 = distinct !{!30, !9, !"OMPAliasScope"}
!31 = !{!32, !33, !34}
!32 = distinct !{!32, !9, !"OMPAliasScope"}
!33 = distinct !{!33, !9, !"OMPAliasScope"}
!34 = distinct !{!34, !9, !"OMPAliasScope"}
!35 = !{i64 236}
!36 = !{i64 6}
!37 = !{!38, !38, i64 0}
!38 = !{!"ifx$unique_sym$48", !4, i64 0}
!39 = !{!40}
!40 = distinct !{!40, !9, !"OMPAliasScope"}
!41 = !{i64 338}
!42 = !{!43}
!43 = distinct !{!43, !9, !"OMPAliasScope"}
!44 = !{i64 339}
!45 = !{!46}
!46 = distinct !{!46, !9, !"OMPAliasScope"}
!47 = !{i64 340}
!48 = !{!49}
!49 = distinct !{!49, !9, !"OMPAliasScope"}
!50 = !{i64 341}
!51 = !{!52}
!52 = distinct !{!52, !9, !"OMPAliasScope"}
!53 = !{i64 342}
!54 = !{!55}
!55 = distinct !{!55, !9, !"OMPAliasScope"}
!56 = !{i64 343}
!57 = !{!58}
!58 = distinct !{!58, !9, !"OMPAliasScope"}
!59 = !{i64 344}
!60 = !{!61}
!61 = distinct !{!61, !9, !"OMPAliasScope"}
!62 = !{i64 345}
!63 = !{!64}
!64 = distinct !{!64, !9, !"OMPAliasScope"}
!65 = !{i64 346}
!66 = !{!67}
!67 = distinct !{!67, !9, !"OMPAliasScope"}
!68 = !{i64 347}
!69 = !{!70}
!70 = distinct !{!70, !9, !"OMPAliasScope"}
!71 = !{i64 348}
!72 = !{!73}
!73 = distinct !{!73, !9, !"OMPAliasScope"}
!74 = !{i64 349}
!75 = !{!76}
!76 = distinct !{!76, !9, !"OMPAliasScope"}
!77 = !{i64 350}
!78 = !{!79}
!79 = distinct !{!79, !9, !"OMPAliasScope"}
!80 = !{i64 351}
!81 = !{!82}
!82 = distinct !{!82, !9, !"OMPAliasScope"}
!83 = !{i64 352}
!84 = !{!85}
!85 = distinct !{!85, !9, !"OMPAliasScope"}
!86 = !{i64 353}
!87 = !{!88}
!88 = distinct !{!88, !9, !"OMPAliasScope"}
!89 = !{i64 354}
!90 = !{!91}
!91 = distinct !{!91, !9, !"OMPAliasScope"}
!92 = !{i64 355}
!93 = !{!94}
!94 = distinct !{!94, !9, !"OMPAliasScope"}
!95 = !{i64 356}
!96 = !{!97}
!97 = distinct !{!97, !9, !"OMPAliasScope"}
!98 = !{i64 357}
!99 = !{!100}
!100 = distinct !{!100, !9, !"OMPAliasScope"}
!101 = !{i64 358}
!102 = !{!103}
!103 = distinct !{!103, !9, !"OMPAliasScope"}
!104 = !{i64 359}
!105 = !{!106}
!106 = distinct !{!106, !9, !"OMPAliasScope"}
!107 = !{i64 360}
!108 = !{!109}
!109 = distinct !{!109, !9, !"OMPAliasScope"}
!110 = !{i64 361}
!111 = !{!112}
!112 = distinct !{!112, !9, !"OMPAliasScope"}
!113 = !{i64 362}
!114 = !{!115}
!115 = distinct !{!115, !9, !"OMPAliasScope"}
!116 = !{i64 363}
!117 = !{!118}
!118 = distinct !{!118, !9, !"OMPAliasScope"}
!119 = !{i64 364}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$52", !4, i64 0}
!122 = !{!30}
!123 = !{!124, !125, !13, !14, !15, !16, !126, !17, !18, !19, !20, !21, !127, !22, !23, !24, !25, !26, !8, !32, !40, !43, !46, !49, !52, !55, !58, !61, !64, !27, !10, !33, !67, !70, !73, !76, !79, !82, !85, !88, !91, !28, !11, !34, !94, !97, !100, !103, !106, !109, !112, !115, !118, !29, !128, !129, !130, !131, !132, !133}
!124 = distinct !{!124, !9, !"OMPAliasScope"}
!125 = distinct !{!125, !9, !"OMPAliasScope"}
!126 = distinct !{!126, !9, !"OMPAliasScope"}
!127 = distinct !{!127, !9, !"OMPAliasScope"}
!128 = distinct !{!128, !9, !"OMPAliasScope"}
!129 = distinct !{!129, !9, !"OMPAliasScope"}
!130 = distinct !{!130, !9, !"OMPAliasScope"}
!131 = distinct !{!131, !9, !"OMPAliasScope"}
!132 = distinct !{!132, !9, !"OMPAliasScope"}
!133 = distinct !{!133, !9, !"OMPAliasScope"}
!134 = distinct !{!134, !135, !136}
!135 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!136 = !{!"llvm.loop.parallel_accesses", !1}
!137 = !{i32 0, i32 2147483647}

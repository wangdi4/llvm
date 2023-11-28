; RUN: opt -disable-output -passes="hir-ssa-deconstruction,print<hir>,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -vplan-enable-hir-f90-dv -debug-only=vploop-analysis < %s 2>&1 | FileCheck %s

; NOTE: Verify that one of the aliases is in loop UB before VPlan
; CHECK:        %omp.simd = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LINEAR:TYPED.IV(&((%"sum_vec$J$_1.linear.iv")[0]), 0, 1, 1),  QUAL.OMP.PRIVATE:F90_DV.TYPED(&((%C1.priv)[0]), zeroinitializer, 0),  QUAL.OMP.NORMALIZED.IV:TYPED(null, 0),  QUAL.OMP.NORMALIZED.UB:TYPED(null, 0),  QUAL.OMP.LIVEIN(&((%M)[0])),  QUAL.OMP.LIVEIN(&((%N)[0])),  QUAL.OMP.LIVEIN:F90_DV(&((%C2)[0])) ]
; CHECK-NEXT:   %"C1.addr_a0$_fetch.22" = (%C1.priv)[0].0;
; CHECK-NEXT:   %"C1.dim_info$.spacing$[]_fetch.23" = (%C1.priv)[0].6[0].1;
; CHECK-NEXT:   %"C1.dim_info$.extent$[]_fetch.24" = (%C1.priv)[0].6[0].0;
; CHECK:        |   + DO i2 = 0, %"C1.dim_info$.extent$[]_fetch.24" + -1, 1   <DO_LOOP>
; CHECK-NEXT:   |   |   (%"C1.addr_a0$_fetch.22")[i2] = 2;
; CHECK-NEXT:   |   + END LOOP

; NOTE: Test checks whether aliases are correcty recognized by HIR including one alias being UB of an inner loop.
; CHEK:        Aliases:
; CHEK-NEXT:   ptr %"C1.addr_a0$_fetch.22"
; CHEK-NEXT:   <6>          %"C1.addr_a0$_fetch.22" = (%C1.priv)[0].0;
; CHEK-NEXT:   ptr [[TMP1:%.*]] = load ptr [[TMP2:%.*]]
; CHEK-EMPTY:
; CHEK-NEXT:   i64 %"C1.dim_info$.spacing$[]_fetch.23"
; CHEK-NEXT:   <9>          %"C1.dim_info$.spacing$[]_fetch.23" = (%C1.priv)[0].6[0].1;
; CHEK-NEXT:   i64 [[TMP3:%.*]] = load ptr [[TMP4:%.*]]
; CHEK-EMPTY:
; CHEK-NEXT:   i64 %"C1.dim_info$.extent$[]_fetch.24"
; CHEK-NEXT:   <12>         %"C1.dim_info$.extent$[]_fetch.24" = (%C1.priv)[0].6[0].0;
; CHEK-NEXT:   i64 [[TMP5]] = load ptr [[TMP6:%.*]]

; CHECK:   After replacement of private and aliases within the preheader.
; CHECK:   BB2: # preds: BB1
; CHECK:    ptr [[PRIV_ALLOCA:%.*]] = allocate-priv %"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }, OrigAlign = 8
; CHECK:    i64 [[F90DV:%.*]] = call ptr [[PRIV_ALLOCA]] ptr %C1.priv ptr @_f90_dope_vector_init2
; CHECK:    f90-dv-buffer-init i64 [[F90DV]] ptr [[PRIV_ALLOCA]]
; CHECK:    ptr [[TMP7:%.*]] = subscript inbounds ptr [[PRIV_ALLOCA]] i64 0 (0 )
; CHECK:    ptr [[TMP8:%.*]] = load ptr [[TMP7]]
; CHECK:    ptr [[TMP9:%.*]] = subscript inbounds ptr [[PRIV_ALLOCA]] i64 0 (6 ) i64 0 (1 )
; CHECK:    i64 [[TMP10:%.*]] = load ptr [[TMP9]]
; CHECK:    ptr [[TMP11:%.*]] = subscript inbounds ptr [[PRIV_ALLOCA]] i64 0 (6 ) i64 0 (0 )

; CHECK:   %nsbgepcopy = &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]);
; CHECK:   %.vec14 = (<2 x ptr>*)(%nsbgepcopy)[0].0;
; CHECK:   %nsbgepcopy18 = &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]);
; CHECK:   %.vec20 = (<2 x i64>*)(%nsbgepcopy18)[0].6[0].0;
; CHECK:   + DO i1 = 0, %loop.ub, 2   <DO_LOOP>
; CHECK:   |   %.vec23 = %.vec20 >= 1;
; CHECK:   |   %0 = bitcast.<2 x i1>.i2(%.vec23);
; CHECK:   |      %phi.temp26 = %.vec23;
; CHECK:   |      
; CHECK:   |      + UNKNOWN LOOP i2 <novectorize>
; CHECK:   |      |   <i2 = 0>
; CHECK:   |      |   BB6.166:
; CHECK:   |      |   %.vec28 = (%.vec20 >= 1) ? %phi.temp26 : 0;
; CHECK:   |      |   (<2 x i32>*)(%.vec14)[i2] = 2, Mask = @{%.vec28};
; CHECK:   |      |   %.vec29 = i2 + 1 < %.vec20;
; CHECK:   |      + END LOOP
; CHECK:   + END LOOP

; ModuleID = '<stdin>'
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @compare_dope_vectors_IP_sum_vec_(ptr nocapture readnone %SUM_VEC.uplevel_ptr3, ptr noalias dereferenceable(72) "assumed_shape" "ptrnoalias" %C1, ptr noalias nocapture dereferenceable(72) "assumed_shape" "ptrnoalias" %C2, ptr noalias nocapture dereferenceable(4) %N, ptr noalias nocapture dereferenceable(4) %M) {
DIR.OMP.SIMD.2:
  %"sum_vec$J$_1.linear.iv" = alloca i32, align 4
  %C1.priv = alloca %"QNCA_a0$i32*$rank1$", align 8
  %N_fetch.16 = load i32, ptr %N, align 1
  %.dv.init = call i64 @_f90_dope_vector_init2(ptr nonnull %C1.priv, ptr nonnull %C1) #2
  %is.allocated = icmp sgt i64 %.dv.init, 0
  br i1 %is.allocated, label %allocated.then, label %DIR.OMP.SIMD.1

allocated.then:                                   ; preds = %DIR.OMP.SIMD.2
  %C1.priv.alloc.num_elements52 = lshr i64 %.dv.init, 2
  %C1.priv.addr0 = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C1.priv, i64 0, i32 0
  %C1.priv.data = alloca i32, i64 %C1.priv.alloc.num_elements52, align 4
  store ptr %C1.priv.data, ptr %C1.priv.addr0, align 8
  br label %DIR.OMP.SIMD.1

omp.pdo.body47:                                   ; preds = %DIR.OMP.SIMD.163, %do.end_do61
  %indvars.iv58 = phi i64 [ 0, %DIR.OMP.SIMD.163 ], [ %indvars.iv.next59, %do.end_do61 ]
  %indvars.iv.next59 = add nuw nsw i64 %indvars.iv58, 1
  br i1 %rel.5.not53, label %loop_exit54, label %loop_body53.preheader

loop_body53.preheader:                            ; preds = %omp.pdo.body47
  br label %loop_body53

loop_body53:                                      ; preds = %loop_body53, %loop_body53.preheader
  %storemerge54 = phi i64 [ %add.4, %loop_body53 ], [ 1, %loop_body53.preheader ]
  %"C1.addr_a0$_fetch.22[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"C1.dim_info$.spacing$[]_fetch.23", ptr elementtype(i32) %"C1.addr_a0$_fetch.22", i64 %storemerge54)
  store i32 2, ptr %"C1.addr_a0$_fetch.22[]", align 1
  %add.4 = add nuw nsw i64 %storemerge54, 1
  %exitcond = icmp eq i64 %add.4, %1
  br i1 %exitcond, label %loop_exit54.loopexit, label %loop_body53

loop_exit54.loopexit:                             ; preds = %loop_body53
  br label %loop_exit54

loop_exit54:                                      ; preds = %loop_exit54.loopexit, %omp.pdo.body47
  br i1 %rel.6, label %do.end_do61, label %do.body55.preheader

do.body55.preheader:                              ; preds = %loop_exit54
  %"C1.addr_a0$_fetch.34[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"C1.dim_info$.spacing$[]_fetch.23", ptr elementtype(i32) %"C1.addr_a0$_fetch.22", i64 %indvars.iv.next59)
  %"C1.addr_a0$_fetch.34[].promoted" = load i32, ptr %"C1.addr_a0$_fetch.34[]", align 1
  %0 = add i32 %13, %"C1.addr_a0$_fetch.34[].promoted"
  store i32 %0, ptr %"C1.addr_a0$_fetch.34[]", align 1
  br label %do.body60

do.body60:                                        ; preds = %do.body60, %do.body55.preheader
  %indvars.iv = phi i64 [ 1, %do.body55.preheader ], [ %indvars.iv.next, %do.body60 ]
  %"C1.addr_a0$_fetch.48[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"C1.dim_info$.spacing$[]_fetch.23", ptr nonnull elementtype(i32) %"C1.addr_a0$_fetch.22", i64 %indvars.iv)
  %"C1.addr_a0$_fetch.48[]_fetch.52" = load i32, ptr %"C1.addr_a0$_fetch.48[]", align 1
  %add.7 = add nsw i32 %mul.9, %"C1.addr_a0$_fetch.48[]_fetch.52"
  %"C2.addr_a0$_fetch.55[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"C2.dim_info$.spacing$[]_fetch.56", ptr elementtype(i32) %"C2.addr_a0$_fetch.55", i64 %indvars.iv)
  store i32 %add.7, ptr %"C2.addr_a0$_fetch.55[]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond57.not, label %do.end_do61.loopexit, label %do.body60

do.end_do61.loopexit:                             ; preds = %do.body60
  br label %do.end_do61

do.end_do61:                                      ; preds = %do.end_do61.loopexit, %loop_exit54
  %exitcond61.not = icmp eq i64 %indvars.iv.next59, %wide.trip.count60
  br i1 %exitcond61.not, label %DIR.OMP.END.SIMD.2, label %omp.pdo.body47

DIR.OMP.SIMD.1:                                   ; preds = %allocated.then, %DIR.OMP.SIMD.2
  %rel.3.not = icmp slt i32 %N_fetch.16, 1
  br i1 %rel.3.not, label %DIR.OMP.END.SIMD.446, label %DIR.OMP.SIMD.162

DIR.OMP.SIMD.162:                                 ; preds = %DIR.OMP.SIMD.1
  %omp.simd = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED.IV"(ptr %"sum_vec$J$_1.linear.iv", i32 0, i64 1, i32 1), "QUAL.OMP.PRIVATE:F90_DV.TYPED"(ptr %C1.priv, %"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LIVEIN"(ptr %M), "QUAL.OMP.LIVEIN"(ptr %N), "QUAL.OMP.LIVEIN:F90_DV"(ptr %C2) ]
  br label %DIR.OMP.SIMD.163

DIR.OMP.SIMD.163:                                 ; preds = %DIR.OMP.SIMD.162
  %"C1.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C1.priv, i64 0, i32 0
  %"C1.addr_a0$_fetch.22" = load ptr, ptr %"C1.addr_a0$", align 1
  %"C1.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C1.priv, i64 0, i32 6, i64 0, i32 1
  %"C1.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"C1.dim_info$.spacing$", i32 0)
  %"C1.dim_info$.spacing$[]_fetch.23" = load i64, ptr %"C1.dim_info$.spacing$[]", align 1
  %"C1.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C1.priv, i64 0, i32 6, i64 0, i32 0
  %"C1.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"C1.dim_info$.extent$", i32 0)
  %"C1.dim_info$.extent$[]_fetch.24" = load i64, ptr %"C1.dim_info$.extent$[]", align 1
  %rel.5.not53 = icmp slt i64 %"C1.dim_info$.extent$[]_fetch.24", 1
  %N_fetch.29 = load i32, ptr %N, align 1
  %rel.6 = icmp slt i32 %N_fetch.29, 1
  %M_fetch.31 = load i32, ptr %M, align 1
  %mul.9 = mul nsw i32 %M_fetch.31, %M_fetch.31
  %"C2.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C2, i64 0, i32 0
  %"C2.addr_a0$_fetch.55" = load ptr, ptr %"C2.addr_a0$", align 1
  %"C2.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %C2, i64 0, i32 6, i64 0, i32 1
  %"C2.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"C2.dim_info$.spacing$", i32 0)
  %"C2.dim_info$.spacing$[]_fetch.56" = load i64, ptr %"C2.dim_info$.spacing$[]", align 1
  %1 = add nsw i64 %"C1.dim_info$.extent$[]_fetch.24", 1
  %2 = mul i32 %M_fetch.31, %N_fetch.29
  %3 = shl i32 %N_fetch.29, 1
  %4 = add i32 %N_fetch.29, -1
  %5 = zext i32 %4 to i33
  %6 = add i32 %N_fetch.29, -2
  %7 = zext i32 %6 to i33
  %8 = mul i33 %5, %7
  %9 = lshr i33 %8, 1
  %10 = trunc i33 %9 to i32
  %11 = add i32 %3, %10
  %12 = add i32 %11, -1
  %13 = mul i32 %2, %12
  %14 = add i32 %N_fetch.29, 1
  %wide.trip.count60 = zext i32 %N_fetch.16 to i64
  %wide.trip.count = zext i32 %14 to i64
  br label %omp.pdo.body47

DIR.OMP.END.SIMD.2:                               ; preds = %do.end_do61
  %add.9.le = add nuw i32 %N_fetch.16, 1
  store i32 %add.9.le, ptr %"sum_vec$J$_1.linear.iv", align 4
  br label %DIR.OMP.END.SIMD.264

DIR.OMP.END.SIMD.264:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %omp.simd) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.446

DIR.OMP.END.SIMD.446:                             ; preds = %DIR.OMP.END.SIMD.264, %DIR.OMP.SIMD.1
  ret void
}

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32)

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare i64 @_f90_dope_vector_init2(ptr, ptr) local_unnamed_addr

attributes #0 = { nounwind }

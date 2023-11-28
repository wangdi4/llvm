; REQUIRES: asserts
; RUN: opt -passes=local-array-transpose -debug-only=local-array-transpose -S < %s 2>&1 | FileCheck %s

; Check that ACOX is not a candidate because it is not square:

; CHECK: LocalArrayTranspose: No Valid Candidates for foo_

; Check that the subscripts of ACOX are not inverted.

; CHECK: %indvars.iv194 = phi i64 [ 1, %do.body9.preheader ], [ %indvars.iv.next195, %do.end_do14.loopexit ]
; CHECK: %indvars.iv = phi i64 [ 1, %do.body9 ], [ %indvars.iv.next, %do.body13 ]
; CHECK: %"foo_$ACOX.addr_a0$_fetch.26[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.29", ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26", i64 %indvars.iv), !llfort.type_idx !27
; CHECK: %"foo_$ACOX.addr_a0$_fetch.26[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26[]", i64 %indvars.iv194), !llfort.type_idx !27

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; Function Attrs: nounwind uwtable
define void @foo_(ptr noalias nocapture readonly dereferenceable(96) "ptrnoalias" %"foo_$INPUT", ptr noalias nocapture readnone dereferenceable(96) "ptrnoalias" %"foo_$RESULT", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$ACOX" = alloca %"QNCA_a0$float*$rank2$", align 8, !llfort.type_idx !2
  %"var$4" = alloca i64, align 8, !llfort.type_idx !3
  %"var$2_fetch.1.fca.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 0
  store ptr null, ptr %"var$2_fetch.1.fca.0.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.1.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.3.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 3
  store i64 128, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.4.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 4
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.5.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 5
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.0.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.0.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.0.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.1.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.0.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 0, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.0.2.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.1.0.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 0
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.0.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.1.1.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 1
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.1.gep", align 8, !tbaa !4
  %"var$2_fetch.1.fca.6.1.2.gep" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$ACOX", i64 0, i32 6, i64 1, i32 2
  store i64 0, ptr %"var$2_fetch.1.fca.6.1.2.gep", align 8, !tbaa !4
  store i64 133, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  store i64 0, ptr %"var$2_fetch.1.fca.5.gep", align 8, !tbaa !11
  store i64 4, ptr %"var$2_fetch.1.fca.1.gep", align 8, !tbaa !12
  store i64 2, ptr %"var$2_fetch.1.fca.4.gep", align 8, !tbaa !13
  store i64 0, ptr %"var$2_fetch.1.fca.2.gep", align 8, !tbaa !14
  %"foo_$ACOX.dim_info$.lower_bound$[]22" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.2.gep", i32 0), !llfort.type_idx !15
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1, !tbaa !16
  %"foo_$ACOX.dim_info$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.0.gep", i32 0), !llfort.type_idx !17
  store i64 0, ptr %"foo_$ACOX.dim_info$.extent$[]", align 1, !tbaa !18
  %"foo_$ACOX.dim_info$.spacing$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.1.gep", i32 0), !llfort.type_idx !19
  store i64 4, ptr %"foo_$ACOX.dim_info$.spacing$[]", align 1, !tbaa !20
  %"foo_$N_fetch.4" = load i32, ptr %"foo_$N", align 1, !tbaa !21
  %int_sext25 = sext i32 %"foo_$N_fetch.4" to i64, !llfort.type_idx !3
  %"foo_$ACOX.dim_info$.lower_bound$[]28" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.2.gep", i32 1), !llfort.type_idx !15
  store i64 1, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1, !tbaa !16
  %rel.4 = icmp sgt i64 %int_sext25, 0
  %slct.2 = select i1 %rel.4, i64 %int_sext25, i64 0
  %"foo_$ACOX.dim_info$.extent$[]31" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.0.gep", i32 1), !llfort.type_idx !17
  store i64 %slct.2, ptr %"foo_$ACOX.dim_info$.extent$[]31", align 1, !tbaa !18
  %"foo_$ACOX.dim_info$.spacing$[]34" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$2_fetch.1.fca.6.0.1.gep", i32 1), !llfort.type_idx !19
  store i64 0, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !tbaa !20
  %func_result = call i32 (ptr, i32, ...) @for_check_mult_overflow64(ptr nonnull %"var$4", i32 3, i64 0, i64 %slct.2, i64 4) #3, !llfort.type_idx !24
  %"var$4_fetch.5" = load i64, ptr %"var$4", align 8, !tbaa !25, !llfort.type_idx !3
  store i64 1073741957, ptr %"var$2_fetch.1.fca.3.gep", align 8, !tbaa !8
  %and.8 = shl i32 %func_result, 4
  %shl.5 = and i32 %and.8, 16
  %or.9 = or i32 %shl.5, 262146
  %func_result12 = call i32 @for_alloc_allocatable_handle(i64 %"var$4_fetch.5", ptr nonnull %"var$2_fetch.1.fca.0.gep", i32 %or.9, ptr null) #3, !llfort.type_idx !24
  %rel.8 = icmp slt i32 %"foo_$N_fetch.4", 1
  %"foo_$INPUT.addr_a0$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 0
  %"foo_$INPUT.addr_a0$_fetch.16" = load ptr, ptr %"foo_$INPUT.addr_a0$", align 1
  %"foo_$INPUT.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 2
  %"foo_$INPUT.dim_info$.lower_bound$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 0)
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.17" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]", align 1
  %"foo_$INPUT.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$float*$rank2$", ptr %"foo_$INPUT", i64 0, i32 6, i64 0, i32 1
  %"foo_$INPUT.dim_info$.spacing$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.spacing$", i32 1)
  %"foo_$INPUT.dim_info$.spacing$[]_fetch.19" = load i64, ptr %"foo_$INPUT.dim_info$.spacing$[]", align 1, !range !26
  %"foo_$INPUT.dim_info$.lower_bound$[]40" = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$INPUT.dim_info$.lower_bound$", i32 1)
  %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.20" = load i64, ptr %"foo_$INPUT.dim_info$.lower_bound$[]40", align 1
  %"foo_$ACOX.addr_a0$_fetch.26" = load ptr, ptr %"var$2_fetch.1.fca.0.gep", align 8
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]22", align 1
  %"foo_$ACOX.dim_info$.spacing$[]_fetch.29" = load i64, ptr %"foo_$ACOX.dim_info$.spacing$[]34", align 1, !range !26
  %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30" = load i64, ptr %"foo_$ACOX.dim_info$.lower_bound$[]28", align 1
  br i1 %rel.8, label %do.body9.us.preheader, label %do.body9.preheader

do.body9.preheader:                               ; preds = %alloca_0
  %0 = add nuw nsw i32 %"foo_$N_fetch.4", 1
  br label %do.body9

do.body9.us.preheader:                            ; preds = %alloca_0
  br label %do.body9.us

do.body9.us:                                      ; preds = %do.body9.us.preheader, %do.body9.us
  br label %do.body9.us

do.body9:                                         ; preds = %do.body9.preheader, %do.end_do14.loopexit
  %indvars.iv194 = phi i64 [ 1, %do.body9.preheader ], [ %indvars.iv.next195, %do.end_do14.loopexit ]
  %"foo_$INPUT.addr_a0$_fetch.16[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.20", i64 %"foo_$INPUT.dim_info$.spacing$[]_fetch.19", ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.16", i64 %indvars.iv194), !llfort.type_idx !27
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body13

do.body13:                                        ; preds = %do.body9, %do.body13
  %indvars.iv = phi i64 [ 1, %do.body9 ], [ %indvars.iv.next, %do.body13 ]
  %"foo_$INPUT.addr_a0$_fetch.16[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$INPUT.dim_info$.lower_bound$[]_fetch.17", i64 4, ptr elementtype(float) %"foo_$INPUT.addr_a0$_fetch.16[]", i64 %indvars.iv), !llfort.type_idx !27
  %"foo_$INPUT.addr_a0$_fetch.16[][]_fetch.25" = load float, ptr %"foo_$INPUT.addr_a0$_fetch.16[][]", align 1, !tbaa !28, !llfort.type_idx !27
  %"foo_$ACOX.addr_a0$_fetch.26[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.30", i64 %"foo_$ACOX.dim_info$.spacing$[]_fetch.29", ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26", i64 %indvars.iv), !llfort.type_idx !27
  %"foo_$ACOX.addr_a0$_fetch.26[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"foo_$ACOX.dim_info$.lower_bound$[]_fetch.27", i64 4, ptr elementtype(float) %"foo_$ACOX.addr_a0$_fetch.26[]", i64 %indvars.iv194), !llfort.type_idx !27
  store float %"foo_$INPUT.addr_a0$_fetch.16[][]_fetch.25", ptr %"foo_$ACOX.addr_a0$_fetch.26[][]", align 4, !tbaa !30
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do14.loopexit, label %do.body13

do.end_do14.loopexit:                             ; preds = %do.body13
  %indvars.iv.next195 = add nuw i64 %indvars.iv194, 1
  br label %do.body9
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #1

; Function Attrs: nofree
declare !llfort.intrin_id !32 i32 @for_check_mult_overflow64(ptr nocapture, i32, ...) local_unnamed_addr #2

; Function Attrs: nofree
declare !llfort.intrin_id !33 i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #2

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nofree "intel-lang"="fortran" }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i64 25}
!3 = !{i64 3}
!4 = !{!5, !5, i64 0}
!5 = !{!"Fortran Dope Vector Symbol", !6, i64 0}
!6 = !{!"Generic Fortran Symbol", !7, i64 0}
!7 = !{!"ifx$root$1$foo_"}
!8 = !{!9, !10, i64 24}
!9 = !{!"ifx$descr$1", !10, i64 0, !10, i64 8, !10, i64 16, !10, i64 24, !10, i64 32, !10, i64 40, !10, i64 48, !10, i64 56, !10, i64 64, !10, i64 72, !10, i64 80, !10, i64 88}
!10 = !{!"ifx$descr$field", !5, i64 0}
!11 = !{!9, !10, i64 40}
!12 = !{!9, !10, i64 8}
!13 = !{!9, !10, i64 32}
!14 = !{!9, !10, i64 16}
!15 = !{i64 20}
!16 = !{!9, !10, i64 64}
!17 = !{i64 18}
!18 = !{!9, !10, i64 48}
!19 = !{i64 19}
!20 = !{!9, !10, i64 56}
!21 = !{!22, !22, i64 0}
!22 = !{!"ifx$unique_sym$2", !23, i64 0}
!23 = !{!"Fortran Data Symbol", !6, i64 0}
!24 = !{i64 2}
!25 = !{!23, !23, i64 0}
!26 = !{i64 1, i64 -9223372036854775808}
!27 = !{i64 5}
!28 = !{!29, !29, i64 0}
!29 = !{!"ifx$unique_sym$5", !23, i64 0}
!30 = !{!31, !31, i64 0}
!31 = !{!"ifx$unique_sym$6", !23, i64 0}
!32 = !{i32 102}
!33 = !{i32 94}

; RUN: opt -passes=instcombine <%s -S | FileCheck %s

; Verify that a probable innermost level subscript is not changed into a form with ashr.
; Notice that the rank number is zero for the relavant subscript.
; This supression was intended to help the vectorization of innermost loops.
;
; CHECK-DAG:  %[[INT_SEXT164:.*]] = sext i32 %add.53 to i64
; CHECK-DAG:  %"fetch.153[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]_fetch.154", i64 8, ptr elementtype(double) %"fetch.153[][]", i64 %[[INT_SEXT164]])

; Instcombine is not suppressed for a non-zero rank, 2.
;
;            %mul.49 = mul i64 %omp.pdo.norm.iv198.priv.0, %omp.pdo.step197.val
;            %add.51 = add i64 %mul.49, %omp.pdo.start195.val
; CHECK-DAG: %[[SEXT1:.*]] = shl i64 %add.51, 32
; CHECK-DAG: %[[INT_SEXT170:.*]] = ashr exact i64 %[[SEXT1]], 32
; CHECK-DAG: call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"val$[]_fetch.160", i64 %"val$[]_fetch.159", ptr elementtype(double) %fetch.153, i64 %[[INT_SEXT170]])

; Instcombine is not suppressed for a non-zero rank, 1.
;
;             %mul.50 = mul i64 %storemerge, %do.step204.val
;             %add.52 = add i64 %mul.50, %do.start202.val
; CHECK-DAG:  %[[SEXT:.*]] = shl i64 %add.52, 32
; CHECK-DAG:  %[[INT_SEXT167:.*]] = ashr exact i64 %[[SEXT]], 32
; CHECK-DAG:  call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %mul.55, ptr elementtype(double) %"var$2.addr_a0$_fetch.168[]", i64 %[[INT_SEXT167]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$ptr$rank3$" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
%struct.ident_t = type { i32, i32, i32, i32, ptr }
%"QNCA_a0$ptr$rank3$.0" = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

@my_work_ = external global %"QNCA_a0$ptr$rank3$"
@nx_ = external global i32, align 8
@.kmpc_loc.0.0.8 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.10 = external hidden unnamed_addr global %struct.ident_t

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare void @__kmpc_dist_for_static_init_8(ptr, i32, i32, ptr, ptr, ptr, ptr, ptr, i64, i64) #1

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(ptr nocapture readonly, i32) #2

; Function Attrs: nounwind uwtable
define hidden void @foo.DIR.OMP.DISTRIBUTE.PARLOOP.25.split416.split(ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture readonly %"var$2", i64 %"var$4.fpriv333.fp393.val", i64 %"var$3.fpriv336.fp395.val", i64 %do.step204.val, i64 %do.start202.val, i64 %omp.pdo.step197.val, i64 %omp.pdo.start195.val, i64 %omp.collapsed.ub216.priv.val, i64 %do.norm.ub206.val421, i64 %omp.pdo.norm.ub200.val, i64 %omp.collapsed.lb215.priv.val, i64 %omp.pdo.norm.lb199.val, i64 %do.norm.lb205.val) #3 {
DIR.OMP.DISTRIBUTE.PARLOOP.13.split:
  %is.last = alloca i32, align 4
  %lower.bnd = alloca i64, align 4
  %upper.bnd = alloca i64, align 4
  %stride = alloca i64, align 4
  %upperD = alloca i64, align 4
  %"foo$I.fpriv339.linear.iv" = alloca i32, align 8
  store i32 0, ptr %is.last, align 4
  %0 = trunc i64 %"var$4.fpriv333.fp393.val" to i32
  %1 = trunc i64 %"var$3.fpriv336.fp395.val" to i32
  %2 = trunc i64 %do.step204.val to i32
  %3 = trunc i64 %do.start202.val to i32
  %4 = trunc i64 %omp.pdo.step197.val to i32
  %5 = trunc i64 %omp.pdo.start195.val to i32
  %6 = add nuw nsw i64 %do.norm.ub206.val421, 1
  %.not376401 = icmp sgt i64 %omp.collapsed.lb215.priv.val, %omp.collapsed.ub216.priv.val
  br i1 %.not376401, label %DIR.OMP.END.DISTRIBUTE.PARLOOP.21.loopexit, label %omp.collapsed.loop.body220.lr.ph, !prof !11

omp.pdo.cond35:                                   ; preds = %omp.collapsed.loop.body220, %do.epilog43
  %omp.pdo.norm.iv198.priv.0 = phi i64 [ %9, %omp.collapsed.loop.body220 ], [ %add.63, %do.epilog43 ]
  %rel.7.not = icmp sgt i64 %omp.pdo.norm.iv198.priv.0, %9
  br i1 %rel.7.not, label %omp.collapsed.loop.inc222, label %omp.pdo.body36

omp.pdo.body36:                                   ; preds = %omp.pdo.cond35
  %int_sext158 = trunc i64 %omp.pdo.norm.iv198.priv.0 to i32
  %mul.49 = mul nsw i32 %4, %int_sext158
  %add.51 = add nsw i32 %mul.49, %5
  br label %do.cond41

do.cond41:                                        ; preds = %DIR.OMP.END.SIMD.20385, %omp.pdo.body36
  %storemerge = phi i64 [ %10, %omp.pdo.body36 ], [ %add.62, %DIR.OMP.END.SIMD.20385 ]
  %rel.8.not = icmp sgt i64 %storemerge, %10
  br i1 %rel.8.not, label %do.epilog43, label %DIR.OMP.SIMD.11

omp.pdo.body46:                                   ; preds = %omp.pdo.body46.lr.ph, %omp.pdo.body46
  %omp.pdo.norm.iv191.local.0414 = phi i64 [ 0, %omp.pdo.body46.lr.ph ], [ %add.61, %omp.pdo.body46 ]
  %int_sext162 = trunc i64 %omp.pdo.norm.iv191.local.0414 to i32
  %add.53 = add nsw i32 %int_sext162, 1
  store i32 %add.53, ptr %"foo$I.fpriv339.linear.iv", align 1, !tbaa !12, !alias.scope !16, !noalias !21, !llvm.access.group !124
  %fetch.153 = load ptr, ptr @my_work_, align 1, !tbaa !127, !alias.scope !130, !noalias !131, !llvm.access.group !124
  %"val$[]163" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank3$", ptr @my_work_, i64 0, i32 6, i64 0, i32 2), i32 0)
  %"val$[]_fetch.154" = load i64, ptr %"val$[]163", align 1, !tbaa !132, !alias.scope !133, !noalias !131, !llvm.access.group !124
  %int_sext164 = sext i32 %add.53 to i64
  %"val$[]165" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank3$", ptr @my_work_, i64 0, i32 6, i64 0, i32 1), i32 1)
  %"val$[]_fetch.156" = load i64, ptr %"val$[]165", align 1, !tbaa !134, !range !135, !alias.scope !136, !noalias !131, !llvm.access.group !124
  %"val$[]166" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank3$", ptr @my_work_, i64 0, i32 6, i64 0, i32 2), i32 1)
  %"val$[]_fetch.157" = load i64, ptr %"val$[]166", align 1, !tbaa !132, !alias.scope !137, !noalias !131, !llvm.access.group !124
  %int_sext167 = sext i32 %add.52 to i64
  %"val$[]168" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank3$", ptr @my_work_, i64 0, i32 6, i64 0, i32 1), i32 2)
  %"val$[]_fetch.159" = load i64, ptr %"val$[]168", align 1, !tbaa !134, !range !135, !alias.scope !138, !noalias !131, !llvm.access.group !124
  %"val$[]169" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$ptr$rank3$", ptr @my_work_, i64 0, i32 6, i64 0, i32 2), i32 2)
  %"val$[]_fetch.160" = load i64, ptr %"val$[]169", align 1, !tbaa !132, !alias.scope !139, !noalias !131, !llvm.access.group !124
  %int_sext170 = sext i32 %add.51 to i64
  %"fetch.153[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %"val$[]_fetch.160", i64 %"val$[]_fetch.159", ptr elementtype(double) %fetch.153, i64 %int_sext170)
  %"fetch.153[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %"val$[]_fetch.157", i64 %"val$[]_fetch.156", ptr elementtype(double) %"fetch.153[]", i64 %int_sext167)
  %"fetch.153[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %"val$[]_fetch.154", i64 8, ptr elementtype(double) %"fetch.153[][]", i64 %int_sext164)
  %"fetch.153[][][]_fetch.167" = load double, ptr %"fetch.153[][][]", align 1, !tbaa !140, !alias.scope !142, !noalias !143, !llvm.access.group !124
  %"var$2.addr_a0$_fetch.168" = load ptr, ptr %"var$2", align 1, !tbaa !144, !alias.scope !146, !noalias !131, !llvm.access.group !124
  %add.57 = add nsw i32 %1, 2
  %int_sext178 = sext i32 %add.57 to i64
  %mul.55 = shl nsw i64 %int_sext178, 3
  %add.59 = add nsw i32 %0, 2
  %int_sext180 = sext i32 %add.59 to i64
  %mul.56 = mul nsw i64 %mul.55, %int_sext180
  %"var$2.addr_a0$_fetch.168[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 %mul.56, ptr elementtype(double) %"var$2.addr_a0$_fetch.168", i64 %int_sext170)
  %"var$2.addr_a0$_fetch.168[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 %mul.55, ptr elementtype(double) %"var$2.addr_a0$_fetch.168[]", i64 %int_sext167)
  %"var$2.addr_a0$_fetch.168[][][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr elementtype(double) %"var$2.addr_a0$_fetch.168[][]", i64 %int_sext164)
  store double %"fetch.153[][][]_fetch.167", ptr %"var$2.addr_a0$_fetch.168[][][]", align 1, !tbaa !147, !alias.scope !149, !noalias !150, !llvm.access.group !124
  %add.61 = add nsw i64 %omp.pdo.norm.iv191.local.0414, 1
  %7 = add nsw i64 %int_sext194, 1
  %rel.9.not = icmp sgt i64 %7, %add.61
  br i1 %rel.9.not, label %omp.pdo.body46, label %omp.pdo.cond45.DIR.OMP.END.SIMD.20.loopexit_crit_edge, !llvm.loop !151

DIR.OMP.SIMD.11:                                  ; preds = %do.cond41
  %int_sext160 = trunc i64 %storemerge to i32
  %mul.50 = mul nsw i32 %2, %int_sext160
  %add.52 = add nsw i32 %mul.50, %3
  %fetch.143 = load i32, ptr @nx_, align 1, !tbaa !154, !alias.scope !156, !noalias !131, !llvm.access.group !125
  %sub.8 = sub nsw i32 %fetch.143, 1
  %int_sext194 = sext i32 %sub.8 to i64
  %rel.9.not413 = icmp sgt i64 0, %int_sext194
  br i1 %rel.9.not413, label %DIR.OMP.END.SIMD.20385, label %omp.pdo.body46.lr.ph

omp.pdo.body46.lr.ph:                             ; preds = %DIR.OMP.SIMD.11
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(ptr %"foo$I.fpriv339.linear.iv", i32 1), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ], !llvm.access.group !125
  br label %omp.pdo.body46

omp.pdo.cond45.DIR.OMP.END.SIMD.20.loopexit_crit_edge: ; preds = %omp.pdo.body46
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SIMD"() ], !llvm.access.group !125
  br label %DIR.OMP.END.SIMD.20385

DIR.OMP.END.SIMD.20385:                           ; preds = %omp.pdo.cond45.DIR.OMP.END.SIMD.20.loopexit_crit_edge, %DIR.OMP.SIMD.11
  %add.62 = add nsw i64 %storemerge, 1
  br label %do.cond41

do.epilog43:                                      ; preds = %do.cond41
  %add.63 = add nsw i64 %omp.pdo.norm.iv198.priv.0, 1
  br label %omp.pdo.cond35

omp.collapsed.loop.body220.lr.ph:                 ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.13.split
  %my.tid = load i32, ptr %tid, align 4
  store i64 %omp.collapsed.lb215.priv.val, ptr %lower.bnd, align 4
  store i64 %omp.collapsed.ub216.priv.val, ptr %upper.bnd, align 4
  store i64 1, ptr %stride, align 4
  store i64 %omp.collapsed.ub216.priv.val, ptr %upperD, align 4
  call void @__kmpc_dist_for_static_init_8(ptr @.kmpc_loc.0.0.8, i32 %my.tid, i32 34, ptr nonnull %is.last, ptr nonnull %lower.bnd, ptr nonnull %upper.bnd, ptr nonnull %upperD, ptr nonnull %stride, i64 1, i64 1)
  %lb.new = load i64, ptr %lower.bnd, align 4, !range !157
  %ub.new = load i64, ptr %upper.bnd, align 4, !range !157
  %omp.ztt = icmp sle i64 %lb.new, %ub.new
  br i1 %omp.ztt, label %omp.collapsed.loop.body220, label %loop.region.exit

omp.collapsed.loop.body220:                       ; preds = %omp.collapsed.loop.inc222, %omp.collapsed.loop.body220.lr.ph
  %omp.collapsed.iv214.priv.local.0402 = phi i64 [ %11, %omp.collapsed.loop.inc222 ], [ %lb.new, %omp.collapsed.loop.body220.lr.ph ]
  %9 = sdiv i64 %omp.collapsed.iv214.priv.local.0402, %6
  %10 = srem i64 %omp.collapsed.iv214.priv.local.0402, %6
  br label %omp.pdo.cond35

omp.collapsed.loop.inc222:                        ; preds = %omp.pdo.cond35
  %11 = add nuw nsw i64 %omp.collapsed.iv214.priv.local.0402, 1
  %.not376 = icmp sle i64 %11, %ub.new
  br i1 %.not376, label %omp.collapsed.loop.body220, label %loop.region.exit, !prof !158, !llvm.loop !159

loop.region.exit:                                 ; preds = %omp.collapsed.loop.inc222, %omp.collapsed.loop.body220.lr.ph
  call void @__kmpc_for_static_fini(ptr @.kmpc_loc.0.0.10, i32 %my.tid)
  br label %DIR.OMP.END.DISTRIBUTE.PARLOOP.21.loopexit

DIR.OMP.END.DISTRIBUTE.PARLOOP.21.loopexit:       ; preds = %loop.region.exit, %DIR.OMP.DISTRIBUTE.PARLOOP.13.split
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nounwind }
attributes #2 = { nofree nounwind }
attributes #3 = { nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "prefer-vector-width"="512" "processed-by-vpo" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.module.flags = !{!8, !9, !10}
!nvvm.annotations = !{}

!0 = !{i32 0, i32 2050, i32 63970565, !"foo", i32 81, i32 0, i32 0}
!1 = !{i32 0, i32 2050, i32 63970565, !"foo", i32 100, i32 1, i32 0}
!2 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_2d9pt_", i32 152, i32 2, i32 0}
!3 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_2d9pt_", i32 171, i32 3, i32 0}
!4 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_3d7pt_", i32 229, i32 4, i32 0}
!5 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_3d7pt_", i32 250, i32 5, i32 0}
!6 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_3d27pt_", i32 310, i32 6, i32 0}
!7 = !{i32 0, i32 2050, i32 63970565, !"mg_stencil_comps_mod_mp_mg_stencil_3d27pt_", i32 340, i32 7, i32 0}
!8 = !{i32 1, !"ThinLTO", i32 0}
!9 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!10 = !{i32 7, !"openmp", i32 50}
!11 = !{!"branch_weights", i32 1, i32 99}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$7", !14, i64 0}
!14 = !{!"Generic Fortran Symbol", !15, i64 0}
!15 = !{!"Simple Fortran Alias Analysis 1"}
!16 = !{!17, !19}
!17 = distinct !{!17, !18, !"OMPAliasScope"}
!18 = distinct !{!18, !"OMPDomain"}
!19 = distinct !{!19, !20, !"OMPAliasScope"}
!20 = distinct !{!20, !"OMPDomain"}
!21 = !{!22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123}
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
!61 = distinct !{!61, !18, !"OMPAliasScope"}
!62 = distinct !{!62, !18, !"OMPAliasScope"}
!63 = distinct !{!63, !18, !"OMPAliasScope"}
!64 = distinct !{!64, !18, !"OMPAliasScope"}
!65 = distinct !{!65, !18, !"OMPAliasScope"}
!66 = distinct !{!66, !18, !"OMPAliasScope"}
!67 = distinct !{!67, !18, !"OMPAliasScope"}
!68 = distinct !{!68, !20, !"OMPAliasScope"}
!69 = distinct !{!69, !20, !"OMPAliasScope"}
!70 = distinct !{!70, !20, !"OMPAliasScope"}
!71 = distinct !{!71, !20, !"OMPAliasScope"}
!72 = distinct !{!72, !20, !"OMPAliasScope"}
!73 = distinct !{!73, !20, !"OMPAliasScope"}
!74 = distinct !{!74, !20, !"OMPAliasScope"}
!75 = distinct !{!75, !20, !"OMPAliasScope"}
!76 = distinct !{!76, !20, !"OMPAliasScope"}
!77 = distinct !{!77, !20, !"OMPAliasScope"}
!78 = distinct !{!78, !20, !"OMPAliasScope"}
!79 = distinct !{!79, !20, !"OMPAliasScope"}
!80 = distinct !{!80, !20, !"OMPAliasScope"}
!81 = distinct !{!81, !20, !"OMPAliasScope"}
!82 = distinct !{!82, !20, !"OMPAliasScope"}
!83 = distinct !{!83, !20, !"OMPAliasScope"}
!84 = distinct !{!84, !20, !"OMPAliasScope"}
!85 = distinct !{!85, !20, !"OMPAliasScope"}
!86 = distinct !{!86, !20, !"OMPAliasScope"}
!87 = distinct !{!87, !20, !"OMPAliasScope"}
!88 = distinct !{!88, !20, !"OMPAliasScope"}
!89 = distinct !{!89, !20, !"OMPAliasScope"}
!90 = distinct !{!90, !20, !"OMPAliasScope"}
!91 = distinct !{!91, !20, !"OMPAliasScope"}
!92 = distinct !{!92, !20, !"OMPAliasScope"}
!93 = distinct !{!93, !20, !"OMPAliasScope"}
!94 = distinct !{!94, !20, !"OMPAliasScope"}
!95 = distinct !{!95, !20, !"OMPAliasScope"}
!96 = distinct !{!96, !20, !"OMPAliasScope"}
!97 = distinct !{!97, !20, !"OMPAliasScope"}
!98 = distinct !{!98, !20, !"OMPAliasScope"}
!99 = distinct !{!99, !20, !"OMPAliasScope"}
!100 = distinct !{!100, !20, !"OMPAliasScope"}
!101 = distinct !{!101, !20, !"OMPAliasScope"}
!102 = distinct !{!102, !20, !"OMPAliasScope"}
!103 = distinct !{!103, !20, !"OMPAliasScope"}
!104 = distinct !{!104, !20, !"OMPAliasScope"}
!105 = distinct !{!105, !20, !"OMPAliasScope"}
!106 = distinct !{!106, !20, !"OMPAliasScope"}
!107 = distinct !{!107, !20, !"OMPAliasScope"}
!108 = distinct !{!108, !20, !"OMPAliasScope"}
!109 = distinct !{!109, !20, !"OMPAliasScope"}
!110 = distinct !{!110, !20, !"OMPAliasScope"}
!111 = distinct !{!111, !20, !"OMPAliasScope"}
!112 = distinct !{!112, !20, !"OMPAliasScope"}
!113 = distinct !{!113, !20, !"OMPAliasScope"}
!114 = distinct !{!114, !20, !"OMPAliasScope"}
!115 = distinct !{!115, !20, !"OMPAliasScope"}
!116 = distinct !{!116, !20, !"OMPAliasScope"}
!117 = distinct !{!117, !20, !"OMPAliasScope"}
!118 = distinct !{!118, !20, !"OMPAliasScope"}
!119 = distinct !{!119, !20, !"OMPAliasScope"}
!120 = distinct !{!120, !20, !"OMPAliasScope"}
!121 = distinct !{!121, !20, !"OMPAliasScope"}
!122 = distinct !{!122, !20, !"OMPAliasScope"}
!123 = distinct !{!123, !20, !"OMPAliasScope"}
!124 = !{!125, !126}
!125 = distinct !{}
!126 = distinct !{}
!127 = !{!128, !129, i64 0}
!128 = !{!"ifx$descr$2", !129, i64 0, !129, i64 8, !129, i64 16, !129, i64 24, !129, i64 32, !129, i64 40, !129, i64 48, !129, i64 56, !129, i64 64, !129, i64 72, !129, i64 80, !129, i64 88, !129, i64 96, !129, i64 104, !129, i64 112}
!129 = !{!"ifx$descr$field", !14, i64 0}
!130 = !{!61, !117}
!131 = !{!24, !26, !28, !30, !32, !34, !37, !38, !36, !40, !41, !42, !35, !43, !44, !45, !46, !47, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !17, !69, !71, !72, !73, !75, !76, !77, !78, !79, !80, !81, !82, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !19}
!132 = !{!128, !129, i64 64}
!133 = !{!62, !118}
!134 = !{!128, !129, i64 56}
!135 = !{i64 1, i64 -9223372036854775808}
!136 = !{!63, !119}
!137 = !{!64, !120}
!138 = !{!65, !121}
!139 = !{!66, !122}
!140 = !{!141, !141, i64 0}
!141 = !{!"ifx$unique_sym$9", !14, i64 0}
!142 = !{!56, !54, !112, !110}
!143 = !{!24, !26, !28, !30, !32, !34, !37, !38, !36, !40, !41, !42, !35, !43, !44, !45, !46, !47, !49, !50, !51, !52, !17, !55, !59, !53, !60, !69, !71, !72, !73, !75, !76, !77, !78, !79, !80, !81, !82, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !105, !106, !107, !108, !19, !111, !115, !109, !116}
!144 = !{!145, !129, i64 0}
!145 = !{!"ifx$descr$1", !129, i64 0, !129, i64 8, !129, i64 16, !129, i64 24, !129, i64 32, !129, i64 40, !129, i64 48, !129, i64 56, !129, i64 64, !129, i64 72, !129, i64 80, !129, i64 88, !129, i64 96, !129, i64 104, !129, i64 112}
!146 = !{!67, !123}
!147 = !{!148, !148, i64 0}
!148 = !{!"ifx$unique_sym$8", !14, i64 0}
!149 = !{!55, !59, !53, !60, !111, !115, !109, !116}
!150 = !{!22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !17, !61, !62, !63, !64, !65, !66, !56, !54, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !19, !117, !118, !119, !120, !121, !122, !112, !110, !123}
!151 = distinct !{!151, !152, !153}
!152 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!153 = !{!"llvm.loop.parallel_accesses", !126}
!154 = !{!155, !155, i64 0}
!155 = !{!"ifx$unique_sym$1", !14, i64 0}
!156 = !{!48, !104}
!157 = !{i64 0, i64 9223372036854775807}
!158 = !{!"branch_weights", i32 99, i32 1}
!159 = distinct !{!159, !152, !160}
!160 = !{!"llvm.loop.parallel_accesses", !125}

; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s 

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.__tgt_offload_entry = type { i8 addrspace(4)*, i8 addrspace(2)*, i64, i32, i32, i64 }

@.omp_offloading.entry_name = internal target_declare unnamed_addr addrspace(2) constant [42 x i8] c"__omp_offloading_fd00_3d44cdb__Z4main_l10\00"
@.omp_offloading.entry.__omp_offloading_fd00_3d44cdb__Z4main_l10 = weak target_declare local_unnamed_addr addrspace(2) constant %struct.__tgt_offload_entry { i8 addrspace(4)* null, i8 addrspace(2)* getelementptr inbounds ([42 x i8], [42 x i8] addrspace(2)* @.omp_offloading.entry_name, i32 0, i32 0), i64 0, i32 0, i32 0, i64 42 }, section "omp_offloading_entries"

declare spir_func i64 @_Z29__spirv_NumWorkgroups_xv() local_unnamed_addr

declare spir_func i64 @_Z21__spirv_WorkgroupId_xv() local_unnamed_addr

declare spir_func i64 @_Z22__spirv_WorkgroupSize_xv() local_unnamed_addr

declare spir_func i64 @_Z27__spirv_LocalInvocationId_xv() local_unnamed_addr

; Function Attrs: noinline norecurse nounwind
define weak dso_local spir_kernel void @__omp_offloading_fd00_3d44cdb__Z4main_l10([128 x i32] addrspace(1)* %a.ascast, i64 %.omp.lb.ascast.val136.zext, i64 %.omp.ub.ascast.val.zext) local_unnamed_addr #0 !omp_simd_kernel !2 !intel_reqd_sub_group_size !5 {
DIR.OMP.DISTRIBUTE.PARLOOP.7:
  %b.ascast.priv = alloca [16 x i32], align 32
  %.omp.ub.ascast.val.zext.trunc = trunc i64 %.omp.ub.ascast.val.zext to i32
  %.omp.lb.ascast.val136.zext.trunc = trunc i64 %.omp.lb.ascast.val136.zext to i32
  %cmp1115 = icmp slt i32 %.omp.ub.ascast.val.zext.trunc, %.omp.lb.ascast.val136.zext.trunc
  br i1 %cmp1115, label %DIR.OMP.END.TARGET.17.exitStub, label %omp.inner.for.body.lr.ph

DIR.OMP.END.TARGET.17.exitStub:                   ; preds = %pred.load.continue.1, %omp.inner.for.body.lr.ph, %DIR.OMP.DISTRIBUTE.PARLOOP.7
  ret void

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.DISTRIBUTE.PARLOOP.7
  %0 = tail call spir_func i64 @_Z29__spirv_NumWorkgroups_xv() #3
  %1 = sub i32 %.omp.ub.ascast.val.zext.trunc, %.omp.lb.ascast.val136.zext.trunc
  %2 = trunc i64 %0 to i32
  %3 = add i32 %1, %2
  %4 = sdiv i32 %3, %2
  %5 = tail call spir_func i64 @_Z21__spirv_WorkgroupId_xv() #3
  %6 = trunc i64 %5 to i32
  %7 = mul i32 %4, %6
  %8 = add i32 %7, %.omp.lb.ascast.val136.zext.trunc
  %9 = add i32 %4, -1
  %10 = add i32 %9, %8
  %11 = icmp slt i32 %10, %.omp.ub.ascast.val.zext.trunc
  %spec.select = select i1 %11, i32 %10, i32 %.omp.ub.ascast.val.zext.trunc, !prof !6
  %12 = tail call spir_func i64 @_Z22__spirv_WorkgroupSize_xv() #3
  %13 = tail call spir_func i64 @_Z27__spirv_LocalInvocationId_xv() #3
  %14 = trunc i64 %13 to i32
  %15 = add i32 %8, %14
  %16 = icmp sgt i32 %15, %spec.select
  br i1 %16, label %DIR.OMP.END.TARGET.17.exitStub, label %omp.inner.for.body.preheader

omp.inner.for.body.preheader:                     ; preds = %omp.inner.for.body.lr.ph
  %17 = sext i32 %15 to i64
  %18 = shl i64 %12, 32
  %19 = ashr exact i64 %18, 32
  %20 = sext i32 %spec.select to i64
  %21 = bitcast [16 x i32]* %b.ascast.priv to <8 x i32>*
  %scalar.gep.1 = getelementptr inbounds [16 x i32], [16 x i32]* %b.ascast.priv, i64 0, i64 8
  %22 = bitcast i32* %scalar.gep.1 to <8 x i32>*
  br label %pred.load.continue.1

pred.load.continue.1:                             ; preds = %pred.load.continue.1, %omp.inner.for.body.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %pred.load.continue.1 ], [ %17, %omp.inner.for.body.preheader ]
  %23 = getelementptr inbounds [128 x i32], [128 x i32] addrspace(1)* %a.ascast, i64 0, i64 %indvars.iv
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32>* nonnull %21, i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
; CHECK: %20 = load <8 x i32>, <8 x i32>* %17, align 32
; CHECK: %21 = select <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>, <8 x i32> <i32 undef, i32 1, i32 2, i32 3, i32 undef, i32 5, i32 6, i32 7>, <8 x i32> %20
; CHECK: store <8 x i32> %21, <8 x i32>* %17, align 32
  %24 = load i32, i32 addrspace(1)* %23, align 4
; CHECK: %22 = bitcast i32 addrspace(1)* %19 to <1 x i32 addrspace(1)*>
; CHECK: %svm.gather = call <1 x i32> @llvm.genx.svm.gather.v1i32.v1i1.v1p1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)*> %22, <1 x i32> undef) 
  %25 = add i32 %24, 100
  %broadcast.splatinsert7 = insertelement <8 x i32> undef, i32 %25, i32 0
  %broadcast.splat8 = shufflevector <8 x i32> %broadcast.splatinsert7, <8 x i32> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 0, i32 undef, i32 undef, i32 undef>
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> %broadcast.splat8, <8 x i32>* nonnull %21, i32 4, <8 x i1> <i1 true, i1 false, i1 false, i1 false, i1 true, i1 false, i1 false, i1 false>)
  %wide.load = load <8 x i32>, <8 x i32>* %21, align 32
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> <i32 undef, i32 9, i32 10, i32 11, i32 undef, i32 13, i32 14, i32 15>, <8 x i32>* nonnull %22, i32 4, <8 x i1> <i1 false, i1 true, i1 true, i1 true, i1 false, i1 true, i1 true, i1 true>)
  %26 = load i32, i32 addrspace(1)* %23, align 4
  %27 = add i32 %26, 100
  %broadcast.splatinsert7.1 = insertelement <8 x i32> undef, i32 %27, i32 0
  %broadcast.splat8.1 = shufflevector <8 x i32> %broadcast.splatinsert7.1, <8 x i32> undef, <8 x i32> <i32 0, i32 undef, i32 undef, i32 undef, i32 0, i32 undef, i32 undef, i32 undef>
  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> %broadcast.splat8.1, <8 x i32>* nonnull %22, i32 4, <8 x i1> <i1 true, i1 false, i1 false, i1 false, i1 true, i1 false, i1 false, i1 false>)
  %wide.load.1 = load <8 x i32>, <8 x i32>* %22, align 32
  %28 = add nsw <8 x i32> %wide.load.1, %wide.load
  %29 = call i32 @llvm.vector.reduce.add.v8i32(<8 x i32> %28)
  %add25 = add nsw i32 %26, %29
  store i32 %add25, i32 addrspace(1)* %23, align 4, !tbaa !7, !alias.scope !12, !noalias !19, !llvm.access.group !44
; CHECK: %36 = bitcast i32 addrspace(1)* %19 to <1 x i32 addrspace(1)*>
; CHECK: %add254 = insertelement <1 x i32> undef, i32 %add25, i32 0
; CHECK: call void @llvm.genx.svm.scatter.v1i1.v1p1i32.v1i32(<1 x i1> <i1 true>, i32 0, <1 x i32 addrspace(1)*> %36, <1 x i32> %add254)
  %indvars.iv.next = add i64 %indvars.iv, %19
  %cmp1 = icmp sgt i64 %indvars.iv.next, %20
  br i1 %cmp1, label %DIR.OMP.END.TARGET.17.exitStub, label %pred.load.continue.1, !llvm.loop !45
}

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.masked.store.v8i32.p0v8i32(<8 x i32>, <8 x i32>*, i32 immarg, <8 x i1>) #1

; Function Attrs: nounwind readnone willreturn
declare i32 @llvm.vector.reduce.add.v8i32(<8 x i32>) #2

attributes #0 = { noinline norecurse nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn writeonly }
attributes #2 = { nounwind readnone willreturn }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!spirv.Source = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!4 = !{i32 4, i32 200000}
!5 = !{i32 1}
!6 = !{!"branch_weights", i32 99999, i32 100000}
!7 = !{!8, !9, i64 0}
!8 = !{!"array@_ZTSA128_i", !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !15, !16, !18}
!13 = distinct !{!13, !14, !"OMPAliasScope"}
!14 = distinct !{!14, !"OMPDomain"}
!15 = distinct !{!15, !14, !"OMPAliasScope"}
!16 = distinct !{!16, !17, !"OMPAliasScope"}
!17 = distinct !{!17, !"OMPDomain"}
!18 = distinct !{!18, !17, !"OMPAliasScope"}
!19 = !{!20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43}
!20 = distinct !{!20, !14, !"OMPAliasScope"}
!21 = distinct !{!21, !14, !"OMPAliasScope"}
!22 = distinct !{!22, !14, !"OMPAliasScope"}
!23 = distinct !{!23, !14, !"OMPAliasScope"}
!24 = distinct !{!24, !14, !"OMPAliasScope"}
!25 = distinct !{!25, !14, !"OMPAliasScope"}
!26 = distinct !{!26, !14, !"OMPAliasScope"}
!27 = distinct !{!27, !14, !"OMPAliasScope"}
!28 = distinct !{!28, !14, !"OMPAliasScope"}
!29 = distinct !{!29, !14, !"OMPAliasScope"}
!30 = distinct !{!30, !14, !"OMPAliasScope"}
!31 = distinct !{!31, !14, !"OMPAliasScope"}
!32 = distinct !{!32, !17, !"OMPAliasScope"}
!33 = distinct !{!33, !17, !"OMPAliasScope"}
!34 = distinct !{!34, !17, !"OMPAliasScope"}
!35 = distinct !{!35, !17, !"OMPAliasScope"}
!36 = distinct !{!36, !17, !"OMPAliasScope"}
!37 = distinct !{!37, !17, !"OMPAliasScope"}
!38 = distinct !{!38, !17, !"OMPAliasScope"}
!39 = distinct !{!39, !17, !"OMPAliasScope"}
!40 = distinct !{!40, !17, !"OMPAliasScope"}
!41 = distinct !{!41, !17, !"OMPAliasScope"}
!42 = distinct !{!42, !17, !"OMPAliasScope"}
!43 = distinct !{!43, !17, !"OMPAliasScope"}
!44 = distinct !{}
!45 = distinct !{!45, !46}
!46 = !{!"llvm.loop.parallel_accesses", !44}

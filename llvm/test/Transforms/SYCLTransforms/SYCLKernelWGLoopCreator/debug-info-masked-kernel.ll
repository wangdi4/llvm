; This test checks that implicit GIDs are preserved after inlining vector kernel
; into masked kernel and inlining masked kernel into scalar kernel.
;
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(ptr addrspace(1) noalias %src) local_unnamed_addr #0 !dbg !6 !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !kernel_arg_name !17 !kernel_arg_host_accessible !18 !kernel_arg_pipe_depth !19 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !16 !vectorized_kernel !20 !vectorized_masked_kernel !21 !no_barrier_path !22 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !13 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !19 !vectorized_width !13 !scalar_kernel !23 !kernel_execution_length !24 !kernel_has_barrier !18 !kernel_has_global_sync !18 !arg_type_null_val !59 {
entry:
; CHECK-LABEL: @test
; CHECK: %__ocl_dbg_gid0{{.*}} = alloca
; CHECK: %__ocl_dbg_gid1{{.*}} = alloca
; CHECK: %__ocl_dbg_gid2{{.*}} = alloca

; CHECK: entryvector_func{{.*}}:
; CHECK: store volatile i64 %dim_0_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid0
; CHECK-NEXT: store volatile i64 %dim_1_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid1
; CHECK-NEXT: store volatile i64 %dim_2_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid2

; CHECK: masked_kernel_entry{{.*}}:
; CHECK: store volatile i64 %dim_0_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid0
; CHECK-NEXT: store volatile i64 %dim_1_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid1
; CHECK-NEXT: store volatile i64 %dim_2_vector_ind_var{{.*}}, ptr %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !25, metadata !DIExpression()), !dbg !28
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !29, metadata !DIExpression()), !dbg !28
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !30, metadata !DIExpression()), !dbg !28
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata ptr addrspace(1) %src, metadata !12, metadata !DIExpression()), !dbg !28
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !31
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !32
  store i32 %subgroup.size, ptr addrspace(1) %ptridx, align 4, !dbg !33, !tbaa !34
  ret void, !dbg !38
}

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

define [7 x i64] @WG.boundaries.test(ptr addrspace(1) %0) !recommended_vector_length !39 {
entry:
  %1 = call i64 @_Z14get_local_sizej(i32 0)
  %2 = call i64 @get_base_global_id.(i32 0)
  %3 = call i64 @_Z14get_local_sizej(i32 1)
  %4 = call i64 @get_base_global_id.(i32 1)
  %5 = call i64 @_Z14get_local_sizej(i32 2)
  %6 = call i64 @get_base_global_id.(i32 2)
  %7 = insertvalue [7 x i64] undef, i64 %1, 2
  %8 = insertvalue [7 x i64] %7, i64 %2, 1
  %9 = insertvalue [7 x i64] %8, i64 %3, 4
  %10 = insertvalue [7 x i64] %9, i64 %4, 3
  %11 = insertvalue [7 x i64] %10, i64 %5, 6
  %12 = insertvalue [7 x i64] %11, i64 %6, 5
  %13 = insertvalue [7 x i64] %12, i64 1, 0
  ret [7 x i64] %13
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_test(ptr addrspace(1) noalias %src) local_unnamed_addr #0 !dbg !40 !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !kernel_arg_name !17 !kernel_arg_host_accessible !18 !kernel_arg_pipe_depth !19 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !16 !vectorized_kernel !23 !no_barrier_path !22 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !13 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !19 !vectorized_width !39 !scalar_kernel !5 !kernel_execution_length !43 !kernel_has_barrier !18 !kernel_has_global_sync !18 !recommended_vector_length !39 !vectorization_dimension !19 !can_unite_workgroups !18 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !44
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert2 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !45
  %broadcast.splat3 = shufflevector <16 x i32> %broadcast.splatinsert2, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !45
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !46
  store <16 x i32> %broadcast.splat3, ptr addrspace(1) %scalar.gep, align 4, !dbg !47, !intel.preferred_alignment !48
  ret void, !dbg !49
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeM16u_test(ptr addrspace(1) noalias %src, <16 x i32> %mask) local_unnamed_addr #0 !dbg !50 !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !kernel_arg_name !17 !kernel_arg_host_accessible !18 !kernel_arg_pipe_depth !19 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !16 !vectorized_kernel !23 !no_barrier_path !22 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !13 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !19 !vectorized_width !39 !scalar_kernel !5 !kernel_execution_length !53 !kernel_has_barrier !18 !kernel_has_global_sync !18 !recommended_vector_length !39 !vectorization_dimension !19 !can_unite_workgroups !18 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %vec.mask = alloca <16 x i32>, align 64
  store <16 x i32> %mask, ptr %vec.mask, align 64
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !54
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert3 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !55
  %broadcast.splat4 = shufflevector <16 x i32> %broadcast.splatinsert3, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !55
  %5 = icmp ne <16 x i32> %mask, zeroinitializer
  %scalar.gep2 = getelementptr inbounds i32, ptr addrspace(1) %src, i64 %call1, !dbg !56
  call void @llvm.masked.store.v16i32.p1(<16 x i32> %broadcast.splat4, ptr addrspace(1) %scalar.gep2, i32 4, <16 x i1> %5), !dbg !57, !intel.preferred_alignment !48
  ret void, !dbg !58
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.store.v16i32.p1(<16 x i32>, ptr addrspace(1), i32 immarg, <16 x i1>) #5

declare i64 @_Z12get_local_idj(i32)

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #3

attributes #0 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_test,_ZGVeM16u_test" }
attributes #1 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { nounwind }
attributes #5 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #6 = { convergent nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "subgroup.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{ptr @test}
!6 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 2, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !11)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12}
!12 = !DILocalVariable(name: "src", arg: 1, scope: !6, file: !1, line: 2, type: !9)
!13 = !{i32 1}
!14 = !{!"none"}
!15 = !{!"int*"}
!16 = !{!""}
!17 = !{!"src"}
!18 = !{i1 false}
!19 = !{i32 0}
!20 = !{ptr @_ZGVeN16u_test}
!21 = !{ptr @_ZGVeM16u_test}
!22 = !{i1 true}
!23 = !{null}
!24 = !{i32 12}
!25 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, file: !26, line: 1, type: !27, flags: DIFlagArtificial)
!26 = !DIFile(filename: "", directory: "")
!27 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!28 = !DILocation(line: 0, scope: !6)
!29 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, file: !26, line: 1, type: !27, flags: DIFlagArtificial)
!30 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, file: !26, line: 1, type: !27, flags: DIFlagArtificial)
!31 = !DILocation(line: 4, column: 9, scope: !6)
!32 = !DILocation(line: 4, column: 5, scope: !6)
!33 = !DILocation(line: 4, column: 27, scope: !6)
!34 = !{!35, !35, i64 0}
!35 = !{!"int", !36, i64 0}
!36 = !{!"omnipotent char", !37, i64 0}
!37 = !{!"Simple C/C++ TBAA"}
!38 = !DILocation(line: 5, column: 1, scope: !6)
!39 = !{i32 16}
!40 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 2, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !41)
!41 = !{!42}
!42 = !DILocalVariable(name: "src", arg: 1, scope: !40, file: !1, line: 2, type: !9)
!43 = !{i32 14}
!44 = !DILocation(line: 4, column: 9, scope: !40)
!45 = !DILocation(line: 4, column: 29, scope: !40)
!46 = !DILocation(line: 4, column: 5, scope: !40)
!47 = !DILocation(line: 4, column: 27, scope: !40)
!48 = !{i32 64}
!49 = !DILocation(line: 5, column: 1, scope: !40)
!50 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 2, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !51)
!51 = !{!52}
!52 = !DILocalVariable(name: "src", arg: 1, scope: !50, file: !1, line: 2, type: !9)
!53 = !{i32 17}
!54 = !DILocation(line: 4, column: 9, scope: !50)
!55 = !DILocation(line: 4, column: 29, scope: !50)
!56 = !DILocation(line: 4, column: 5, scope: !50)
!57 = !DILocation(line: 4, column: 27, scope: !50)
!58 = !DILocation(line: 5, column: 1, scope: !50)
!59 = !{ptr addrspace(1) null}

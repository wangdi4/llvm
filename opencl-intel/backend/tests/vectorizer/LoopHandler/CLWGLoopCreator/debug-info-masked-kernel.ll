; This test checks that implicit GIDs are preserved after inlining vector kernel
; into masked kernel and inlining masked kernel into scalar kernel.
;
; The IR is dumped at the beginning of CLWGLoopCreator::runOnModule()
; from source with build option "-g" and env CL_CONFIG_CPU_VECTORIZER_TYPE=vpo
; VOLCANO_LLVM_OPTIONS=-vplan-enable-peeling:
; #pragma OPENCL EXTENSION cl_intel_subgroups: enable
; kernel void test(global int* src) {
;   src[get_global_id(0)] = get_sub_group_size();
; }
;
; RUN: %oclopt -cl-loop-creator -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test(i32 addrspace(1)* noalias %src) local_unnamed_addr #0 !dbg !11 !kernel_arg_addr_space !19 !kernel_arg_access_qual !20 !kernel_arg_type !21 !kernel_arg_base_type !21 !kernel_arg_type_qual !22 !kernel_arg_name !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !22 !kernel_arg_buffer_location !22 !vectorized_kernel !26 !vectorized_masked_kernel !27 !no_barrier_path !28 !kernel_has_sub_groups !28 !opencl.stats.Vectorizer.CanVect !19 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !25 !vectorized_width !19 !scalarized_kernel !29 !kernel_execution_length !30 !kernel_has_barrier !24 !kernel_has_global_sync !24 {
entry:
; CHECK-LABEL: @test
; CHECK: %__ocl_dbg_gid0{{.*}} = alloca
; CHECK: %__ocl_dbg_gid1{{.*}} = alloca
; CHECK: %__ocl_dbg_gid2{{.*}} = alloca

; CHECK: entryvector_func{{.*}}:
; CHECK: store volatile i64 %dim_0_vector_tid{{.*}}, i64* %__ocl_dbg_gid0
; CHECK-NEXT: store volatile i64 %dim_1_vector_tid{{.*}}, i64* %__ocl_dbg_gid1
; CHECK-NEXT: store volatile i64 %dim_2_vector_tid{{.*}}, i64* %__ocl_dbg_gid2

; CHECK: masked_kernel_entry{{.*}}:
; CHECK: store volatile i64 %dim_0_vector_tid{{.*}}, i64* %__ocl_dbg_gid0
; CHECK-NEXT: store volatile i64 %dim_1_vector_tid{{.*}}, i64* %__ocl_dbg_gid1
; CHECK-NEXT: store volatile i64 %dim_2_vector_tid{{.*}}, i64* %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0, metadata !31, metadata !DIExpression()), !dbg !34
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, i64* %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1, metadata !35, metadata !DIExpression()), !dbg !34
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2, metadata !36, metadata !DIExpression()), !dbg !34
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %src, metadata !18, metadata !DIExpression()), !dbg !34
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !37
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call1, !dbg !38
  store i32 %subgroup.size, i32 addrspace(1)* %ptridx, align 4, !dbg !39, !tbaa !40
  ret void, !dbg !44
}

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

define [7 x i64] @WG.boundaries.test(i32 addrspace(1)* %0) !ocl_recommended_vector_length !45 {
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
define void @_ZGVeN16u_test(i32 addrspace(1)* noalias %src) local_unnamed_addr #0 !dbg !46 !kernel_arg_addr_space !19 !kernel_arg_access_qual !20 !kernel_arg_type !21 !kernel_arg_base_type !21 !kernel_arg_type_qual !22 !kernel_arg_name !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !22 !kernel_arg_buffer_location !22 !vectorized_kernel !29 !no_barrier_path !28 !kernel_has_sub_groups !28 !ocl_recommended_vector_length !45 !opencl.stats.Vectorizer.CanVect !19 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !25 !vectorized_width !45 !vectorization_dimension !25 !scalarized_kernel !8 !can_unite_workgroups !24 !kernel_execution_length !49 !kernel_has_barrier !24 !kernel_has_global_sync !24 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, i64* %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2, align 8
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !50
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert2 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !51
  %broadcast.splat3 = shufflevector <16 x i32> %broadcast.splatinsert2, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !51
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call1, !dbg !52
  %5 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*, !dbg !53
  store <16 x i32> %broadcast.splat3, <16 x i32> addrspace(1)* %5, align 4, !dbg !53, !intel.preferred_alignment !54
  ret void, !dbg !55
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeM16u_test(i32 addrspace(1)* noalias %src, <16 x i32> %mask) local_unnamed_addr #0 !dbg !56 !kernel_arg_addr_space !19 !kernel_arg_access_qual !20 !kernel_arg_type !21 !kernel_arg_base_type !21 !kernel_arg_type_qual !22 !kernel_arg_name !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !22 !kernel_arg_buffer_location !22 !vectorized_kernel !29 !no_barrier_path !28 !kernel_has_sub_groups !28 !ocl_recommended_vector_length !45 !opencl.stats.Vectorizer.CanVect !19 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !25 !vectorized_width !45 !vectorization_dimension !25 !scalarized_kernel !8 !can_unite_workgroups !24 !kernel_execution_length !59 !kernel_has_barrier !24 !kernel_has_global_sync !24 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, i64* %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2, align 8
  %vec.mask = alloca <16 x i32>, align 64
  store <16 x i32> %mask, <16 x i32>* %vec.mask, align 64
  %call1 = tail call i64 @_Z13get_global_idj(i32 0) #6, !dbg !60
  %0 = call i64 @_Z14get_local_sizej(i32 0) #4
  %uniform.id.max = and i64 %0, -16
  %1 = call i64 @_Z12get_local_idj(i32 0) #4
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = trunc i64 %0 to i32
  %4 = and i32 %3, 15
  %subgroup.size = select i1 %2, i32 16, i32 %4
  %broadcast.splatinsert3 = insertelement <16 x i32> undef, i32 %subgroup.size, i32 0, !dbg !61
  %broadcast.splat4 = shufflevector <16 x i32> %broadcast.splatinsert3, <16 x i32> undef, <16 x i32> zeroinitializer, !dbg !61
  %5 = icmp ne <16 x i32> %mask, zeroinitializer
  %scalar.gep2 = getelementptr inbounds i32, i32 addrspace(1)* %src, i64 %call1, !dbg !62
  %6 = bitcast i32 addrspace(1)* %scalar.gep2 to <16 x i32> addrspace(1)*, !dbg !63
  call void @llvm.masked.store.v16i32.p1v16i32(<16 x i32> %broadcast.splat4, <16 x i32> addrspace(1)* %6, i32 4, <16 x i1> %5), !dbg !63, !intel.preferred_alignment !54
  ret void, !dbg !64
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.store.v16i32.p1v16i32(<16 x i32>, <16 x i32> addrspace(1)*, i32 immarg, <16 x i1>) #5

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
!llvm.linker.options = !{}
!llvm.module.flags = !{!3, !4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!opencl.kernels = !{!8}
!opencl.stats.Vectorizer.CanVect = !{!9}
!opencl.stats.Vectorizer.Chosen_Vectorization_Dim = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "subgroup.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{void (i32 addrspace(1)*)* @test}
!9 = !{!"Code is vectorizable"}
!10 = !{!"The chosen vectorization dimension"}
!11 = distinct !DISubprogram(name: "test", scope: !12, file: !12, line: 2, type: !13, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !17)
!12 = !DIFile(filename: "subgroup.cl", directory: "")
!13 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !14)
!14 = !{null, !15}
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64)
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !{!18}
!18 = !DILocalVariable(name: "src", arg: 1, scope: !11, file: !12, line: 2, type: !15)
!19 = !{i32 1}
!20 = !{!"none"}
!21 = !{!"int*"}
!22 = !{!""}
!23 = !{!"src"}
!24 = !{i1 false}
!25 = !{i32 0}
!26 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_test}
!27 = !{void (i32 addrspace(1)*, <16 x i32>)* @_ZGVeM16u_test}
!28 = !{i1 true}
!29 = !{null}
!30 = !{i32 12}
!31 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !11, file: !32, line: 1, type: !33, flags: DIFlagArtificial)
!32 = !DIFile(filename: "", directory: "")
!33 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!34 = !DILocation(line: 0, scope: !11)
!35 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !11, file: !32, line: 1, type: !33, flags: DIFlagArtificial)
!36 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !11, file: !32, line: 1, type: !33, flags: DIFlagArtificial)
!37 = !DILocation(line: 4, column: 9, scope: !11)
!38 = !DILocation(line: 4, column: 5, scope: !11)
!39 = !DILocation(line: 4, column: 27, scope: !11)
!40 = !{!41, !41, i64 0}
!41 = !{!"int", !42, i64 0}
!42 = !{!"omnipotent char", !43, i64 0}
!43 = !{!"Simple C/C++ TBAA"}
!44 = !DILocation(line: 5, column: 1, scope: !11)
!45 = !{i32 16}
!46 = distinct !DISubprogram(name: "test", scope: !12, file: !12, line: 2, type: !13, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !47)
!47 = !{!48}
!48 = !DILocalVariable(name: "src", arg: 1, scope: !46, file: !12, line: 2, type: !15)
!49 = !{i32 14}
!50 = !DILocation(line: 4, column: 9, scope: !46)
!51 = !DILocation(line: 4, column: 29, scope: !46)
!52 = !DILocation(line: 4, column: 5, scope: !46)
!53 = !DILocation(line: 4, column: 27, scope: !46)
!54 = !{i32 64}
!55 = !DILocation(line: 5, column: 1, scope: !46)
!56 = distinct !DISubprogram(name: "test", scope: !12, file: !12, line: 2, type: !13, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !57)
!57 = !{!58}
!58 = !DILocalVariable(name: "src", arg: 1, scope: !56, file: !12, line: 2, type: !15)
!59 = !{i32 17}
!60 = !DILocation(line: 4, column: 9, scope: !56)
!61 = !DILocation(line: 4, column: 29, scope: !56)
!62 = !DILocation(line: 4, column: 5, scope: !56)
!63 = !DILocation(line: 4, column: 27, scope: !56)
!64 = !DILocation(line: 5, column: 1, scope: !56)

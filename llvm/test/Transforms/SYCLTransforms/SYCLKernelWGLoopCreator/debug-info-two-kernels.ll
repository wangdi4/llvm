; This test checks that implicit GIDs are preserved if there are more than one
; kernels.
;
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @foo(ptr addrspace(1) noalias %dst) local_unnamed_addr #0 !dbg !6 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_name !21 !kernel_arg_host_accessible !22 !kernel_arg_pipe_depth !23 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !vectorized_kernel !24 !no_barrier_path !25 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !17 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !23 !vectorized_width !17 !scalar_kernel !26 !kernel_execution_length !27 !kernel_has_barrier !22 !kernel_has_global_sync !22 !max_wg_dimensions !17 {
entry:
; CHECK-LABEL: @foo
; CHECK: %__ocl_dbg_gid0 = alloca
; CHECK: %__ocl_dbg_gid1 = alloca
; CHECK: %__ocl_dbg_gid2 = alloca

; CHECK: entryvector_func:
; CHECK: store volatile i64 %dim_0_vector_ind_var, ptr %__ocl_dbg_gid0
; CHECK: store volatile i64 %init.gid.dim1, ptr %__ocl_dbg_gid1
; CHECK: store volatile i64 %init.gid.dim2, ptr %__ocl_dbg_gid2

; CHECK: scalar_kernel_entry:
; CHECK: store volatile i64 %dim_0_ind_var, ptr %__ocl_dbg_gid0
; CHECK: store volatile i64 %init.gid.dim1, ptr %__ocl_dbg_gid1
; CHECK: store volatile i64 %init.gid.dim2, ptr %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !28, metadata !DIExpression()), !dbg !31
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !32, metadata !DIExpression()), !dbg !31
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !33, metadata !DIExpression()), !dbg !31
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata ptr addrspace(1) %dst, metadata !12, metadata !DIExpression()), !dbg !31
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !34
  call void @llvm.dbg.value(metadata i64 %call, metadata !13, metadata !DIExpression()), !dbg !31
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call, !dbg !35
  store i32 0, ptr addrspace(1) %ptridx, align 4, !dbg !36, !tbaa !37
  ret void, !dbg !41
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define void @bar(ptr addrspace(1) noalias %dst) local_unnamed_addr #2 !dbg !42 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_name !21 !kernel_arg_host_accessible !22 !kernel_arg_pipe_depth !23 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !vectorized_kernel !46 !no_barrier_path !25 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !17 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !23 !vectorized_width !17 !scalar_kernel !26 !kernel_execution_length !27 !kernel_has_barrier !22 !kernel_has_global_sync !22 !max_wg_dimensions !17 {
entry:
; CHECK-LABEL: @bar
; CHECK: %__ocl_dbg_gid0 = alloca
; CHECK: %__ocl_dbg_gid1 = alloca
; CHECK: %__ocl_dbg_gid2 = alloca

; CHECK: entryvector_func:
; CHECK: store volatile i64 %dim_0_vector_ind_var, ptr %__ocl_dbg_gid0
; CHECK: store volatile i64 %init.gid.dim1, ptr %__ocl_dbg_gid1
; CHECK: store volatile i64 %init.gid.dim2, ptr %__ocl_dbg_gid2

; CHECK: scalar_kernel_entry:
; CHECK: store volatile i64 %dim_0_ind_var, ptr %__ocl_dbg_gid0
; CHECK: store volatile i64 %init.gid.dim1, ptr %__ocl_dbg_gid1
; CHECK: store volatile i64 %init.gid.dim2, ptr %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !47, metadata !DIExpression()), !dbg !48
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !49, metadata !DIExpression()), !dbg !48
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !50, metadata !DIExpression()), !dbg !48
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata ptr addrspace(1) %dst, metadata !44, metadata !DIExpression()), !dbg !48
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !51
  call void @llvm.dbg.value(metadata i64 %call, metadata !45, metadata !DIExpression()), !dbg !48
  %ptridx = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call, !dbg !52
  store i32 1, ptr addrspace(1) %ptridx, align 4, !dbg !53, !tbaa !37
  ret void, !dbg !54
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

define [7 x i64] @WG.boundaries.foo(ptr addrspace(1) %0) !recommended_vector_length !55 {
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

define [7 x i64] @WG.boundaries.bar(ptr addrspace(1) %0) !recommended_vector_length !55 {
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

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_foo(ptr addrspace(1) noalias %dst) local_unnamed_addr #0 !dbg !56 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_name !21 !kernel_arg_host_accessible !22 !kernel_arg_pipe_depth !23 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !vectorized_kernel !26 !no_barrier_path !25 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !17 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !23 !vectorized_width !55 !scalar_kernel !60 !kernel_execution_length !61 !kernel_has_barrier !22 !kernel_has_global_sync !22 !max_wg_dimensions !17 !recommended_vector_length !55 !vectorization_dimension !23 !can_unite_workgroups !25 {
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
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !62
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call, !dbg !63
  store <16 x i32> zeroinitializer, ptr addrspace(1) %scalar.gep, align 4, !dbg !64
  ret void, !dbg !65
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_bar(ptr addrspace(1) noalias %dst) local_unnamed_addr #2 !dbg !66 !kernel_arg_addr_space !17 !kernel_arg_access_qual !18 !kernel_arg_type !19 !kernel_arg_base_type !19 !kernel_arg_type_qual !20 !kernel_arg_name !21 !kernel_arg_host_accessible !22 !kernel_arg_pipe_depth !23 !kernel_arg_pipe_io !20 !kernel_arg_buffer_location !20 !vectorized_kernel !26 !no_barrier_path !25 !kernel_has_sub_groups !22 !opencl.stats.Vectorizer.CanVect !17 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !23 !vectorized_width !55 !scalar_kernel !70 !kernel_execution_length !61 !kernel_has_barrier !22 !kernel_has_global_sync !22 !max_wg_dimensions !17 !recommended_vector_length !55 !vectorization_dimension !23 !can_unite_workgroups !25 {
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
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !71
  %scalar.gep = getelementptr inbounds i32, ptr addrspace(1) %dst, i64 %call, !dbg !72
  store <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, ptr addrspace(1) %scalar.gep, align 4, !dbg !73
  ret void, !dbg !74
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #3

attributes #0 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_foo" }
attributes #1 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVeN16u_bar" }
attributes #3 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "two-kernels.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{ptr @foo, ptr @bar}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !11)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12, !13}
!12 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!13 = !DILocalVariable(name: "gid", scope: !6, file: !1, line: 2, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !15, line: 19, baseType: !16)
!15 = !DIFile(filename: "opencl-c-platform.h", directory: "")
!16 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!17 = !{i32 1}
!18 = !{!"none"}
!19 = !{!"int*"}
!20 = !{!""}
!21 = !{!"dst"}
!22 = !{i1 false}
!23 = !{i32 0}
!24 = !{ptr @_ZGVeN16u_foo}
!25 = !{i1 true}
!26 = !{null}
!27 = !{i32 6}
!28 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!29 = !DIFile(filename: "", directory: "")
!30 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!31 = !DILocation(line: 0, scope: !6)
!32 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!33 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!34 = !DILocation(line: 2, column: 16, scope: !6)
!35 = !DILocation(line: 3, column: 3, scope: !6)
!36 = !DILocation(line: 3, column: 12, scope: !6)
!37 = !{!38, !38, i64 0}
!38 = !{!"int", !39, i64 0}
!39 = !{!"omnipotent char", !40, i64 0}
!40 = !{!"Simple C/C++ TBAA"}
!41 = !DILocation(line: 4, column: 1, scope: !6)
!42 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 6, type: !7, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !43)
!43 = !{!44, !45}
!44 = !DILocalVariable(name: "dst", arg: 1, scope: !42, file: !1, line: 6, type: !9)
!45 = !DILocalVariable(name: "gid", scope: !42, file: !1, line: 7, type: !14)
!46 = !{ptr @_ZGVeN16u_bar}
!47 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !42, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!48 = !DILocation(line: 0, scope: !42)
!49 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !42, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!50 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !42, file: !29, line: 1, type: !30, flags: DIFlagArtificial)
!51 = !DILocation(line: 7, column: 16, scope: !42)
!52 = !DILocation(line: 8, column: 3, scope: !42)
!53 = !DILocation(line: 8, column: 12, scope: !42)
!54 = !DILocation(line: 9, column: 1, scope: !42)
!55 = !{i32 16}
!56 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !57)
!57 = !{!58, !59}
!58 = !DILocalVariable(name: "dst", arg: 1, scope: !56, file: !1, line: 1, type: !9)
!59 = !DILocalVariable(name: "gid", scope: !56, file: !1, line: 2, type: !14)
!60 = !{ptr @foo}
!61 = !{i32 5}
!62 = !DILocation(line: 2, column: 16, scope: !56)
!63 = !DILocation(line: 3, column: 3, scope: !56)
!64 = !DILocation(line: 3, column: 12, scope: !56)
!65 = !DILocation(line: 4, column: 1, scope: !56)
!66 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 6, type: !7, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !67)
!67 = !{!68, !69}
!68 = !DILocalVariable(name: "dst", arg: 1, scope: !66, file: !1, line: 6, type: !9)
!69 = !DILocalVariable(name: "gid", scope: !66, file: !1, line: 7, type: !14)
!70 = !{ptr @bar}
!71 = !DILocation(line: 7, column: 16, scope: !66)
!72 = !DILocation(line: 8, column: 3, scope: !66)
!73 = !DILocation(line: 8, column: 12, scope: !66)
!74 = !DILocation(line: 9, column: 1, scope: !66)

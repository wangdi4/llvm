; This test checks that implicit GIDs are preserved if there are more than one
; kernels.
;
; The IR is dumped at the beginning of CLWGLoopCreator::runOnModule()
; from source with build option "-g" and env CL_CONFIG_CPU_VECTORIZER_TYPE=vpo:
; kernel void foo(global int *dst) {
;   size_t gid = get_global_id(0);
;   dst[gid] = 0;
; }
; kernel void bar(global int *dst) {
;   size_t gid = get_global_id(0);
;   dst[gid] = 1;
; }
;
; RUN: %oclopt -cl-loop-creator -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @foo(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !dbg !11 !kernel_arg_addr_space !23 !kernel_arg_access_qual !24 !kernel_arg_type !25 !kernel_arg_base_type !25 !kernel_arg_type_qual !26 !kernel_arg_name !27 !kernel_arg_host_accessible !28 !kernel_arg_pipe_depth !29 !kernel_arg_pipe_io !26 !kernel_arg_buffer_location !26 !vectorized_kernel !30 !no_barrier_path !31 !kernel_has_sub_groups !28 !opencl.stats.Vectorizer.CanVect !23 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !29 !vectorized_width !23 !scalarized_kernel !32 !kernel_execution_length !33 !kernel_has_barrier !28 !kernel_has_global_sync !28 !max_wg_dimensions !23 {
entry:
; CHECK-LABEL: @foo
; CHECK: %__ocl_dbg_gid0 = alloca
; CHECK: %__ocl_dbg_gid1 = alloca
; CHECK: %__ocl_dbg_gid2 = alloca

; CHECK: entryvector_func:
; CHECK: store volatile i64 %dim_0_vector_tid, i64* %__ocl_dbg_gid0
; CHECK: store volatile i64 %GlobalID_1vector_func, i64* %__ocl_dbg_gid1
; CHECK: store volatile i64 %GlobalID_2vector_func, i64* %__ocl_dbg_gid2

; CHECK: scalar_kernel_entry:
; CHECK: store volatile i64 %dim_0_tid, i64* %__ocl_dbg_gid0
; CHECK: store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1
; CHECK: store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0, metadata !34, metadata !DIExpression()), !dbg !37
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, i64* %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1, metadata !38, metadata !DIExpression()), !dbg !37
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2, metadata !39, metadata !DIExpression()), !dbg !37
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %dst, metadata !18, metadata !DIExpression()), !dbg !37
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !40
  call void @llvm.dbg.value(metadata i64 %call, metadata !19, metadata !DIExpression()), !dbg !37
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call, !dbg !41
  store i32 0, i32 addrspace(1)* %ptridx, align 4, !dbg !42, !tbaa !43
  ret void, !dbg !47
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define void @bar(i32 addrspace(1)* noalias %dst) local_unnamed_addr #2 !dbg !48 !kernel_arg_addr_space !23 !kernel_arg_access_qual !24 !kernel_arg_type !25 !kernel_arg_base_type !25 !kernel_arg_type_qual !26 !kernel_arg_name !27 !kernel_arg_host_accessible !28 !kernel_arg_pipe_depth !29 !kernel_arg_pipe_io !26 !kernel_arg_buffer_location !26 !vectorized_kernel !52 !no_barrier_path !31 !kernel_has_sub_groups !28 !opencl.stats.Vectorizer.CanVect !23 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !29 !vectorized_width !23 !scalarized_kernel !32 !kernel_execution_length !33 !kernel_has_barrier !28 !kernel_has_global_sync !28 !max_wg_dimensions !23 {
entry:
; CHECK-LABEL: @bar
; CHECK: %__ocl_dbg_gid0 = alloca
; CHECK: %__ocl_dbg_gid1 = alloca
; CHECK: %__ocl_dbg_gid2 = alloca

; CHECK: entryvector_func:
; CHECK: store volatile i64 %dim_0_vector_tid, i64* %__ocl_dbg_gid0
; CHECK: store volatile i64 %GlobalID_1vector_func, i64* %__ocl_dbg_gid1
; CHECK: store volatile i64 %GlobalID_2vector_func, i64* %__ocl_dbg_gid2

; CHECK: scalar_kernel_entry:
; CHECK: store volatile i64 %dim_0_tid, i64* %__ocl_dbg_gid0
; CHECK: store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1
; CHECK: store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0, metadata !53, metadata !DIExpression()), !dbg !54
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, i64* %__ocl_dbg_gid0, align 8
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1, metadata !55, metadata !DIExpression()), !dbg !54
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, i64* %__ocl_dbg_gid1, align 8
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2, metadata !56, metadata !DIExpression()), !dbg !54
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, i64* %__ocl_dbg_gid2, align 8
  call void @llvm.dbg.value(metadata i32 addrspace(1)* %dst, metadata !50, metadata !DIExpression()), !dbg !54
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !57
  call void @llvm.dbg.value(metadata i64 %call, metadata !51, metadata !DIExpression()), !dbg !54
  %ptridx = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call, !dbg !58
  store i32 1, i32 addrspace(1)* %ptridx, align 4, !dbg !59, !tbaa !43
  ret void, !dbg !60
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

define [7 x i64] @WG.boundaries.foo(i32 addrspace(1)* %0) !ocl_recommended_vector_length !61 {
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

define [7 x i64] @WG.boundaries.bar(i32 addrspace(1)* %0) !ocl_recommended_vector_length !61 {
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
define void @_ZGVeN16u_foo(i32 addrspace(1)* noalias %dst) local_unnamed_addr #0 !dbg !62 !kernel_arg_addr_space !23 !kernel_arg_access_qual !24 !kernel_arg_type !25 !kernel_arg_base_type !25 !kernel_arg_type_qual !26 !kernel_arg_name !27 !kernel_arg_host_accessible !28 !kernel_arg_pipe_depth !29 !kernel_arg_pipe_io !26 !kernel_arg_buffer_location !26 !vectorized_kernel !32 !no_barrier_path !31 !kernel_has_sub_groups !28 !ocl_recommended_vector_length !61 !opencl.stats.Vectorizer.CanVect !23 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !29 !vectorized_width !61 !vectorization_dimension !29 !scalarized_kernel !66 !can_unite_workgroups !31 !kernel_execution_length !67 !kernel_has_barrier !28 !kernel_has_global_sync !28 !max_wg_dimensions !23 {
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
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !68
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call, !dbg !69
  %0 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*, !dbg !70
  store <16 x i32> zeroinitializer, <16 x i32> addrspace(1)* %0, align 4, !dbg !70
  ret void, !dbg !71
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: convergent norecurse nounwind
define void @_ZGVeN16u_bar(i32 addrspace(1)* noalias %dst) local_unnamed_addr #2 !dbg !72 !kernel_arg_addr_space !23 !kernel_arg_access_qual !24 !kernel_arg_type !25 !kernel_arg_base_type !25 !kernel_arg_type_qual !26 !kernel_arg_name !27 !kernel_arg_host_accessible !28 !kernel_arg_pipe_depth !29 !kernel_arg_pipe_io !26 !kernel_arg_buffer_location !26 !vectorized_kernel !32 !no_barrier_path !31 !kernel_has_sub_groups !28 !ocl_recommended_vector_length !61 !opencl.stats.Vectorizer.CanVect !23 !opencl.stats.Vectorizer.Chosen_Vectorization_Dim !29 !vectorized_width !61 !vectorization_dimension !29 !scalarized_kernel !76 !can_unite_workgroups !31 !kernel_execution_length !67 !kernel_has_barrier !28 !kernel_has_global_sync !28 !max_wg_dimensions !23 {
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
  %call = tail call i64 @_Z13get_global_idj(i32 0) #5, !dbg !77
  %scalar.gep = getelementptr inbounds i32, i32 addrspace(1)* %dst, i64 %call, !dbg !78
  %0 = bitcast i32 addrspace(1)* %scalar.gep to <16 x i32> addrspace(1)*, !dbg !79
  store <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>, <16 x i32> addrspace(1)* %0, align 4, !dbg !79
  ret void, !dbg !80
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
!1 = !DIFile(filename: "two-kernels.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{void (i32 addrspace(1)*)* @foo, void (i32 addrspace(1)*)* @bar}
!9 = !{!"Code is vectorizable"}
!10 = !{!"The chosen vectorization dimension"}
!11 = distinct !DISubprogram(name: "foo", scope: !12, file: !12, line: 1, type: !13, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !17)
!12 = !DIFile(filename: "two-kernels.cl", directory: "")
!13 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !14)
!14 = !{null, !15}
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64)
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !{!18, !19}
!18 = !DILocalVariable(name: "dst", arg: 1, scope: !11, file: !12, line: 1, type: !15)
!19 = !DILocalVariable(name: "gid", scope: !11, file: !12, line: 2, type: !20)
!20 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !21, line: 19, baseType: !22)
!21 = !DIFile(filename: "opencl-c-platform.h", directory: "")
!22 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!23 = !{i32 1}
!24 = !{!"none"}
!25 = !{!"int*"}
!26 = !{!""}
!27 = !{!"dst"}
!28 = !{i1 false}
!29 = !{i32 0}
!30 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_foo}
!31 = !{i1 true}
!32 = !{null}
!33 = !{i32 6}
!34 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !11, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!35 = !DIFile(filename: "", directory: "")
!36 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!37 = !DILocation(line: 0, scope: !11)
!38 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !11, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!39 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !11, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!40 = !DILocation(line: 2, column: 16, scope: !11)
!41 = !DILocation(line: 3, column: 3, scope: !11)
!42 = !DILocation(line: 3, column: 12, scope: !11)
!43 = !{!44, !44, i64 0}
!44 = !{!"int", !45, i64 0}
!45 = !{!"omnipotent char", !46, i64 0}
!46 = !{!"Simple C/C++ TBAA"}
!47 = !DILocation(line: 4, column: 1, scope: !11)
!48 = distinct !DISubprogram(name: "bar", scope: !12, file: !12, line: 6, type: !13, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !49)
!49 = !{!50, !51}
!50 = !DILocalVariable(name: "dst", arg: 1, scope: !48, file: !12, line: 6, type: !15)
!51 = !DILocalVariable(name: "gid", scope: !48, file: !12, line: 7, type: !20)
!52 = !{void (i32 addrspace(1)*)* @_ZGVeN16u_bar}
!53 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !48, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!54 = !DILocation(line: 0, scope: !48)
!55 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !48, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!56 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !48, file: !35, line: 1, type: !36, flags: DIFlagArtificial)
!57 = !DILocation(line: 7, column: 16, scope: !48)
!58 = !DILocation(line: 8, column: 3, scope: !48)
!59 = !DILocation(line: 8, column: 12, scope: !48)
!60 = !DILocation(line: 9, column: 1, scope: !48)
!61 = !{i32 16}
!62 = distinct !DISubprogram(name: "foo", scope: !12, file: !12, line: 1, type: !13, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !63)
!63 = !{!64, !65}
!64 = !DILocalVariable(name: "dst", arg: 1, scope: !62, file: !12, line: 1, type: !15)
!65 = !DILocalVariable(name: "gid", scope: !62, file: !12, line: 2, type: !20)
!66 = !{void (i32 addrspace(1)*)* @foo}
!67 = !{i32 5}
!68 = !DILocation(line: 2, column: 16, scope: !62)
!69 = !DILocation(line: 3, column: 3, scope: !62)
!70 = !DILocation(line: 3, column: 12, scope: !62)
!71 = !DILocation(line: 4, column: 1, scope: !62)
!72 = distinct !DISubprogram(name: "bar", scope: !12, file: !12, line: 6, type: !13, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !73)
!73 = !{!74, !75}
!74 = !DILocalVariable(name: "dst", arg: 1, scope: !72, file: !12, line: 6, type: !15)
!75 = !DILocalVariable(name: "gid", scope: !72, file: !12, line: 7, type: !20)
!76 = !{void (i32 addrspace(1)*)* @bar}
!77 = !DILocation(line: 7, column: 16, scope: !72)
!78 = !DILocation(line: 8, column: 3, scope: !72)
!79 = !DILocation(line: 8, column: 12, scope: !72)
!80 = !DILocation(line: 9, column: 1, scope: !72)

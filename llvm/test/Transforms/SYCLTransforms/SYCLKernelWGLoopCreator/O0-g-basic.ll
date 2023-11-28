; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

; This test checks that WG loops are created in O0 and -g mode.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @__LocalIds = internal thread_local global [3 x i64] undef, align 16

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #0 !dbg !6 !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !kernel_arg_name !17 !kernel_arg_host_accessible !18 !kernel_arg_pipe_depth !19 !kernel_arg_pipe_io !16 !kernel_arg_buffer_location !16 !no_barrier_path !20 !kernel_has_sub_groups !18 !kernel_execution_length !21 !kernel_has_global_sync !18 !recommended_vector_length !13 {
entry:
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK: store volatile i64 %base.gid.dim0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: store volatile i64 %base.gid.dim1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: store volatile i64 %base.gid.dim2, ptr %__ocl_dbg_gid2, align 8
; CHECK: dim_2_pre_head:
; CHECK: dim_1_pre_head:
; CHECK-NEXT: %dim_2_ind_var = phi i64 [ %base.gid.dim2, %dim_2_pre_head ], [ %dim_2_inc_ind_var, %dim_1_exit ]
; CHECK-NEXT: %dim_2_tid = phi i64 [ %dim_2_sub_lid, %dim_2_pre_head ], [ %dim_2_inc_tid, %dim_1_exit ]
; CHECK-NEXT: store i64 %dim_2_tid, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2), align 8
; CHECK-NEXT: br label %dim_0_pre_head
; CHECK: dim_0_pre_head:
; CHECK-NEXT: %dim_1_ind_var = phi i64 [ %base.gid.dim1, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
; CHECK-NEXT: %dim_1_tid = phi i64 [ %dim_1_sub_lid, %dim_1_pre_head ], [ %dim_1_inc_tid, %dim_0_exit ]
; CHECK-NEXT: store i64 %dim_1_tid, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1), align 8
; CHECK-NEXT: br label %scalar_kernel_entry
; CHECK: scalar_kernel_entry:
; CHECK-NEXT: %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
; CHECK-NEXT: %dim_0_tid = phi i64 [ %dim_0_sub_lid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
; CHECK-NEXT: store i64 %dim_0_tid, ptr @__LocalIds, align 8, !dbg [[DILocGID:![0-9]+]]
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata [[DIVarGID0:![0-9]+]], metadata !DIExpression()), !dbg [[DILocGID]]
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata [[DIVarGID1:![0-9]+]], metadata !DIExpression()), !dbg [[DILocGID]]
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata [[DIVarGID2:![0-9]+]], metadata !DIExpression()), !dbg [[DILocGID]]
; CHECK-NEXT: store volatile i64 %dim_0_ind_var, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: store volatile i64 %dim_1_ind_var, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: store volatile i64 %dim_2_ind_var, ptr %__ocl_dbg_gid2, align 8

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !22, metadata !DIExpression()), !dbg !24
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !25, metadata !DIExpression()), !dbg !24
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !26, metadata !DIExpression()), !dbg !24
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !27, metadata !DIExpression()), !dbg !28
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !29
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #3, !dbg !30
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call, !dbg !29
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !31
  ret void, !dbg !32
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent nounwind readnone willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

; CHECK-DAG: [[DILocGID]] = !DILocation(line: 1, scope: [[SCOPE:![0-9]+]])
; CHECK-DAG: [[DIVarGID0]] = !DILocalVariable(name: "__ocl_dbg_gid0", scope: [[SCOPE]], line: 1, type: [[IndTy:![0-9]+]], flags: DIFlagArtificial)
; CHECK-DAG: [[DIVarGID1]] = !DILocalVariable(name: "__ocl_dbg_gid1", scope: [[SCOPE]], line: 1, type: [[IndTy]], flags: DIFlagArtificial)
; CHECK-DAG: [[DIVarGID2]] = !DILocalVariable(name: "__ocl_dbg_gid2", scope: [[SCOPE]], line: 1, type: [[IndTy]], flags: DIFlagArtificial)

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{ptr @test}
!6 = distinct !DISubprogram(name: "test", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!7 = !DIFile(filename: "simple.cl", directory: "")
!8 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !9)
!9 = !{null, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{}
!13 = !{i32 1}
!14 = !{!"none"}
!15 = !{!"int*"}
!16 = !{!""}
!17 = !{!"dst"}
!18 = !{i1 false}
!19 = !{i32 0}
!20 = !{i1 true}
!21 = !{i32 8}
!22 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, line: 1, type: !23, flags: DIFlagArtificial)
!23 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!24 = !DILocation(line: 1, scope: !6)
!25 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, line: 1, type: !23, flags: DIFlagArtificial)
!26 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, line: 1, type: !23, flags: DIFlagArtificial)
!27 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !7, line: 1, type: !10)
!28 = !DILocation(line: 1, column: 30, scope: !6)
!29 = !DILocation(line: 2, column: 3, scope: !6)
!30 = !DILocation(line: 2, column: 7, scope: !6)
!31 = !DILocation(line: 2, column: 25, scope: !6)
!32 = !DILocation(line: 3, column: 1, scope: !6)

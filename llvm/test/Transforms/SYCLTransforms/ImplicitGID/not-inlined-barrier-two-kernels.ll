; RUN: opt -passes=sycl-kernel-implicit-gid -S %s | FileCheck %s

; This test checks implicit gids are added to not-inlined function which is only
; used in kernel with barrier path.
; Old implicits gids in no-barrier path functions remains unchanged.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@LocalIds = linkonce_odr thread_local global [3 x i64] undef, align 16

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @bar(ptr addrspace(1) noalias noundef %dst, i64 noundef %gid) #1 !dbg !6 !kernel_arg_base_type !75 !arg_type_null_val !76 {
entry:
; CHECK-LABEL: @bar
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid.addr = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !16, metadata !DIExpression()), !dbg !17
  store i64 %gid, ptr %gid.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %gid.addr, metadata !18, metadata !DIExpression()), !dbg !19
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !20
  %1 = load i64, ptr %gid.addr, align 8, !dbg !21
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %1, !dbg !20
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !22
  %2 = load i32, ptr %retval, align 4, !dbg !23
  ret i32 %2, !dbg !23
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #2 !dbg !24 !kernel_arg_addr_space !27 !kernel_arg_access_qual !28 !kernel_arg_type !29 !kernel_arg_base_type !29 !kernel_arg_type_qual !30 !kernel_arg_name !31 !kernel_arg_host_accessible !32 !kernel_arg_pipe_depth !33 !kernel_arg_pipe_io !34 !kernel_arg_buffer_location !34 !no_barrier_path !35 !kernel_has_sub_groups !32 !kernel_execution_length !36 !kernel_has_global_sync !32 !recommended_vector_length !27 !arg_type_null_val !77 {
; CHECK-LABEL: @test(
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NOT: %__ocl_dbg_gid{{.*}} = alloca i64

  %local.ids = alloca [3 x i64], align 8
  %local.id1 = getelementptr inbounds [3 x i64], ptr %local.ids, i64 0, i32 1
  %local.id2 = getelementptr inbounds [3 x i64], ptr %local.ids, i64 0, i32 2
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !37, metadata !DIExpression()), !dbg !39
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !40, metadata !DIExpression()), !dbg !39
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !41, metadata !DIExpression()), !dbg !39
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !42, metadata !DIExpression()), !dbg !43
  call void @llvm.dbg.declare(metadata ptr %gid, metadata !44, metadata !DIExpression()), !dbg !45
  %base.gid.dim0 = call i64 @get_base_global_id.(i32 0)
  %local.size.dim0 = call i64 @_Z14get_local_sizej(i32 0)
  %max.gid.dim0 = add i64 %base.gid.dim0, %local.size.dim0
  %base.gid.dim1 = call i64 @get_base_global_id.(i32 1)
  %local.size.dim1 = call i64 @_Z14get_local_sizej(i32 1)
  %max.gid.dim1 = add i64 %base.gid.dim1, %local.size.dim1
  %base.gid.dim2 = call i64 @get_base_global_id.(i32 2)
  %local.size.dim2 = call i64 @_Z14get_local_sizej(i32 2)
  %max.gid.dim2 = add i64 %base.gid.dim2, %local.size.dim2
  store volatile i64 %base.gid.dim0, ptr %__ocl_dbg_gid0, align 8
  store volatile i64 %base.gid.dim1, ptr %__ocl_dbg_gid1, align 8
  store volatile i64 %base.gid.dim2, ptr %__ocl_dbg_gid2, align 8
  %dim_0_sub_lid = sub nuw nsw i64 %base.gid.dim0, %base.gid.dim0
  %dim_1_sub_lid = sub nuw nsw i64 %base.gid.dim1, %base.gid.dim1
  %dim_2_sub_lid = sub nuw nsw i64 %base.gid.dim2, %base.gid.dim2
  br label %dim_2_pre_head

dim_2_pre_head:                                   ; preds = %0
  br label %dim_1_pre_head

dim_1_pre_head:                                   ; preds = %dim_1_exit, %dim_2_pre_head
  %dim_2_ind_var = phi i64 [ %base.gid.dim2, %dim_2_pre_head ], [ %dim_2_inc_ind_var, %dim_1_exit ]
  %dim_2_tid = phi i64 [ %dim_2_sub_lid, %dim_2_pre_head ], [ %dim_2_inc_tid, %dim_1_exit ]
  br label %dim_0_pre_head

dim_0_pre_head:                                   ; preds = %dim_0_exit, %dim_1_pre_head
  %dim_1_ind_var = phi i64 [ %base.gid.dim1, %dim_1_pre_head ], [ %dim_1_inc_ind_var, %dim_0_exit ]
  %dim_1_tid = phi i64 [ %dim_1_sub_lid, %dim_1_pre_head ], [ %dim_1_inc_tid, %dim_0_exit ]
  br label %scalar_kernel_entry

scalar_kernel_entry:                              ; preds = %scalar_kernel_entry, %dim_0_pre_head
  %dim_0_ind_var = phi i64 [ %base.gid.dim0, %dim_0_pre_head ], [ %dim_0_inc_ind_var, %scalar_kernel_entry ]
  %dim_0_tid = phi i64 [ %dim_0_sub_lid, %dim_0_pre_head ], [ %dim_0_inc_tid, %scalar_kernel_entry ]
  store volatile i64 %dim_0_ind_var, ptr %__ocl_dbg_gid0, align 8
  store volatile i64 %dim_1_ind_var, ptr %__ocl_dbg_gid1, align 8
  store volatile i64 %dim_2_ind_var, ptr %__ocl_dbg_gid2, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  store i64 %dim_0_ind_var, ptr %gid, align 8, !dbg !45
  %1 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !46
  %2 = load i64, ptr %gid, align 8, !dbg !47
  store i64 %dim_0_tid, ptr %local.ids, align 8, !dbg !48
  store i64 %dim_1_tid, ptr %local.id1, align 8, !dbg !48
  store i64 %dim_2_tid, ptr %local.id2, align 8, !dbg !48
  %3 = call i32 @foo(ptr addrspace(1) noalias noundef %1, i64 noundef %2, ptr noalias %local.ids) #1, !dbg !48
  %dim_0_inc_ind_var = add nuw nsw i64 %dim_0_ind_var, 1
  %dim_0_cmp.to.max = icmp eq i64 %dim_0_inc_ind_var, %max.gid.dim0
  %dim_0_inc_tid = add nuw nsw i64 %dim_0_tid, 1
  br i1 %dim_0_cmp.to.max, label %dim_0_exit, label %scalar_kernel_entry

dim_0_exit:                                       ; preds = %scalar_kernel_entry
  %dim_1_inc_ind_var = add nuw nsw i64 %dim_1_ind_var, 1
  %dim_1_cmp.to.max = icmp eq i64 %dim_1_inc_ind_var, %max.gid.dim1
  %dim_1_inc_tid = add nuw nsw i64 %dim_1_tid, 1
  br i1 %dim_1_cmp.to.max, label %dim_1_exit, label %dim_0_pre_head

dim_1_exit:                                       ; preds = %dim_0_exit
  %dim_2_inc_ind_var = add nuw nsw i64 %dim_2_ind_var, 1
  %dim_2_cmp.to.max = icmp eq i64 %dim_2_inc_ind_var, %max.gid.dim2
  %dim_2_inc_tid = add nuw nsw i64 %dim_2_tid, 1
  br i1 %dim_2_cmp.to.max, label %dim_2_exit, label %dim_1_pre_head

dim_2_exit:                                       ; preds = %dim_1_exit
  br label %exit

exit:                                             ; preds = %dim_2_exit
  ret void, !dbg !49
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #3

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test_barrier(ptr addrspace(1) noalias noundef align 4 %dst) #4 !dbg !50 !kernel_arg_addr_space !27 !kernel_arg_access_qual !28 !kernel_arg_type !29 !kernel_arg_base_type !29 !kernel_arg_type_qual !30 !kernel_arg_name !31 !kernel_arg_host_accessible !32 !kernel_arg_pipe_depth !33 !kernel_arg_pipe_io !34 !kernel_arg_buffer_location !34 !no_barrier_path !32 !kernel_has_sub_groups !32 !kernel_execution_length !51 !kernel_has_global_sync !32 !recommended_vector_length !27 !arg_type_null_val !77 {
entry:
; CHECK-LABEL: @test_barrier
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  call void @dummy_barrier.()
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !52, metadata !DIExpression()), !dbg !53
  call void @llvm.dbg.declare(metadata ptr %gid, metadata !54, metadata !DIExpression()), !dbg !55
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #7, !dbg !56
  store i64 %call, ptr %gid, align 8, !dbg !55
  call void @_Z7barrierj(i32 noundef 1) #8, !dbg !57
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !58
  %1 = load i64, ptr %gid, align 8, !dbg !59
  %call1 = call i32 @bar(ptr addrspace(1) noundef %0, i64 noundef %1) #6, !dbg !60
  call void @_Z18work_group_barrierj(i32 1), !dbg !61
  ret void, !dbg !61
}

; Function Attrs: convergent
declare void @_Z7barrierj(i32 noundef) #5

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @foo(ptr addrspace(1) noalias noundef %dst, i64 noundef %gid, ptr noalias %local.ids) #1 !dbg !62 {
entry:
; CHECK-LABEL: @foo
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NOT: %__ocl_dbg_gid{{.*}} = alloca i64

  %base.gid0 = call i64 @get_base_global_id.(i32 0)
  %base.gid1 = call i64 @get_base_global_id.(i32 1)
  %base.gid2 = call i64 @get_base_global_id.(i32 2)
  %local.id0 = load i64, ptr @LocalIds, align 8
  %global.id0 = add i64 %local.id0, %base.gid0
  %local.id1 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @LocalIds, i64 0, i32 1), align 8
  %global.id1 = add i64 %local.id1, %base.gid1
  %local.id2 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @LocalIds, i64 0, i32 2), align 8
  %global.id2 = add i64 %local.id2, %base.gid2
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !63, metadata !DIExpression()), !dbg !64
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !65, metadata !DIExpression()), !dbg !64
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !66, metadata !DIExpression()), !dbg !64
  store volatile i64 %global.id0, ptr %__ocl_dbg_gid0, align 8
  store volatile i64 %global.id1, ptr %__ocl_dbg_gid1, align 8
  store volatile i64 %global.id2, ptr %__ocl_dbg_gid2, align 8
  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  %gid.addr = alloca i64, align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !67, metadata !DIExpression()), !dbg !68
  store i64 %gid, ptr %gid.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %gid.addr, metadata !69, metadata !DIExpression()), !dbg !70
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !71
  %1 = load i64, ptr %gid.addr, align 8, !dbg !72
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %1, !dbg !71
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !73
  %2 = load i32, ptr %retval, align 4, !dbg !74
  ret i32 %2, !dbg !74
}

declare i64 @_Z12get_local_idj(i32)

declare i64 @get_base_global_id.(i32)

declare i64 @_Z14get_local_sizej(i32)

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #6

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #1 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #3 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #4 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #5 = { convergent "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #6 = { convergent }
attributes #7 = { convergent nounwind readnone willreturn }
attributes #8 = { convergent "kernel-call-once" "kernel-convergent-call" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{ptr @test, ptr @test_barrier}
!6 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 5, type: !7, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!7 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !8)
!8 = !{!9, !10, !12}
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !13, line: 116, baseType: !14)
!13 = !DIFile(filename: "opencl-c-base.h", directory: "")
!14 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!15 = !{}
!16 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 5, type: !10)
!17 = !DILocation(line: 5, column: 57, scope: !6)
!18 = !DILocalVariable(name: "gid", arg: 2, scope: !6, file: !1, line: 5, type: !12)
!19 = !DILocation(line: 5, column: 69, scope: !6)
!20 = !DILocation(line: 6, column: 3, scope: !6)
!21 = !DILocation(line: 6, column: 7, scope: !6)
!22 = !DILocation(line: 6, column: 12, scope: !6)
!23 = !DILocation(line: 7, column: 1, scope: !6)
!24 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 9, type: !25, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!25 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !26)
!26 = !{null, !10}
!27 = !{i32 1}
!28 = !{!"none"}
!29 = !{!"int*"}
!30 = !{!"restrict"}
!31 = !{!"dst"}
!32 = !{i1 false}
!33 = !{i32 0}
!34 = !{!""}
!35 = !{i1 true}
!36 = !{i32 11}
!37 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !24, line: 1, type: !38, flags: DIFlagArtificial)
!38 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!39 = !DILocation(line: 9, scope: !24)
!40 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !24, line: 1, type: !38, flags: DIFlagArtificial)
!41 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !24, line: 1, type: !38, flags: DIFlagArtificial)
!42 = !DILocalVariable(name: "dst", arg: 1, scope: !24, file: !1, line: 9, type: !10)
!43 = !DILocation(line: 9, column: 40, scope: !24)
!44 = !DILocalVariable(name: "gid", scope: !24, file: !1, line: 10, type: !12)
!45 = !DILocation(line: 10, column: 10, scope: !24)
!46 = !DILocation(line: 11, column: 7, scope: !24)
!47 = !DILocation(line: 11, column: 12, scope: !24)
!48 = !DILocation(line: 11, column: 3, scope: !24)
!49 = !DILocation(line: 12, column: 1, scope: !24)
!50 = distinct !DISubprogram(name: "test_barrier", scope: !1, file: !1, line: 14, type: !25, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!51 = !{i32 12}
!52 = !DILocalVariable(name: "dst", arg: 1, scope: !50, file: !1, line: 14, type: !10)
!53 = !DILocation(line: 14, column: 48, scope: !50)
!54 = !DILocalVariable(name: "gid", scope: !50, file: !1, line: 15, type: !12)
!55 = !DILocation(line: 15, column: 10, scope: !50)
!56 = !DILocation(line: 15, column: 16, scope: !50)
!57 = !DILocation(line: 16, column: 3, scope: !50)
!58 = !DILocation(line: 17, column: 7, scope: !50)
!59 = !DILocation(line: 17, column: 12, scope: !50)
!60 = !DILocation(line: 17, column: 3, scope: !50)
!61 = !DILocation(line: 18, column: 1, scope: !50)
!62 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !15)
!63 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !62, line: 1, type: !38, flags: DIFlagArtificial)
!64 = !DILocation(line: 1, scope: !62)
!65 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !62, line: 1, type: !38, flags: DIFlagArtificial)
!66 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !62, line: 1, type: !38, flags: DIFlagArtificial)
!67 = !DILocalVariable(name: "dst", arg: 1, scope: !62, file: !1, line: 1, type: !10)
!68 = !DILocation(line: 1, column: 57, scope: !62)
!69 = !DILocalVariable(name: "gid", arg: 2, scope: !62, file: !1, line: 1, type: !12)
!70 = !DILocation(line: 1, column: 69, scope: !62)
!71 = !DILocation(line: 2, column: 3, scope: !62)
!72 = !DILocation(line: 2, column: 7, scope: !62)
!73 = !DILocation(line: 2, column: 12, scope: !62)
!74 = !DILocation(line: 3, column: 1, scope: !62)
!75 = !{!"int*", !"long"}
!76 = !{ptr addrspace(1) null, i64 0}
!77 = !{ptr addrspace(1) null}

; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

; This test checks that
;   * gid calls in foo are fixed. foo is used in kernel without barrier path,
;   * bar is not changed. bar is used in kernel with barrier path.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @foo(ptr addrspace(1) noalias noundef %dst) #0 !dbg !6 !kernel_arg_base_type !36 !arg_type_null_val !61 {
entry:
; CHECK-LABEL: @foo
; CHECK: %lid1.addr = alloca i64, align 8
; CHECK-NEXT: %lid2.addr = alloca i64, align 8
; CHECK-NEXT: %gid0.addr = alloca i64, align 8
; CHECK-NEXT: %gid1.addr = alloca i64, align 8
; CHECK-NEXT: %gid2.addr = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: %retval = alloca i32, align 4
; CHECK-NEXT: %dst.addr = alloca ptr addrspace(1), align 8
; CHECK-NEXT: %lid0.addr = alloca i64, align 8
; CHECK-NEXT: %lid0 = load i64, ptr @__LocalIds, align 8
; CHECK-NEXT: store i64 %lid0, ptr %lid0.addr, align 8
; CHECK-NEXT: %lid1 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1), align 8
; CHECK-NEXT: store i64 %lid1, ptr %lid1.addr, align 8
; CHECK-NEXT: %lid2 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2), align 8
; CHECK-NEXT: store i64 %lid2, ptr %lid2.addr, align 8
; CHECK-NEXT: %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT: %gid0 = add i64 %lid0, %base.gid0
; CHECK-NEXT: store i64 %gid0, ptr %gid0.addr, align 8
; CHECK-NEXT: %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT: %gid1 = add i64 %lid1, %base.gid1
; CHECK-NEXT: store i64 %gid1, ptr %gid1.addr, align 8
; CHECK-NEXT: %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT: %gid2 = add i64 %lid2, %base.gid2
; CHECK-NEXT: store i64 %gid2, ptr %gid2.addr, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: %gid0.ld = load i64, ptr %gid0.addr, align 8
; CHECK-NEXT: store volatile i64 %gid0.ld, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %gid1.ld = load i64, ptr %gid1.addr, align 8
; CHECK-NEXT: store volatile i64 %gid1.ld, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %gid2.ld = load i64, ptr %gid2.addr, align 8
; CHECK-NEXT: store volatile i64 %gid2.ld, ptr %__ocl_dbg_gid2, align 8
; CHECK-NEXT: store ptr addrspace(1) %dst, ptr %dst.addr, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata {{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT: [[PTR:%[0-9]+]] = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg
; CHECK-NEXT: [[GIDLoad:%gid0.ld[0-9]+]] = load i64, ptr %gid0.addr, align 8, !dbg [[DBGGidFoo:![0-9]+]]
; CHECK: %arrayidx = getelementptr inbounds i32, ptr addrspace(1) [[PTR]], i64 [[GIDLoad]], !dbg

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !13, metadata !DIExpression()), !dbg !15
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !16, metadata !DIExpression()), !dbg !15
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !17, metadata !DIExpression()), !dbg !15
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !18, metadata !DIExpression()), !dbg !19
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !20
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #6, !dbg !21
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call, !dbg !20
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !22
  %1 = load i32, ptr %retval, align 4, !dbg !23
  ret i32 %1, !dbg !23
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local i32 @bar(ptr addrspace(1) noalias noundef %dst) #0 !dbg !24 !kernel_arg_base_type !36 !arg_type_null_val !61 {
entry:
; CHECK-LABEL: @bar
; CHECK: call i64 @_Z13get_global_idj(i32 noundef 0)

  %retval = alloca i32, align 4
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !25, metadata !DIExpression()), !dbg !26
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !27
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #6, !dbg !28
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call, !dbg !27
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !29
  %1 = load i32, ptr %retval, align 4, !dbg !30
  ret i32 %1, !dbg !30
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #3 !dbg !31 !kernel_arg_addr_space !34 !kernel_arg_access_qual !35 !kernel_arg_type !36 !kernel_arg_base_type !36 !kernel_arg_type_qual !37 !kernel_arg_name !38 !kernel_arg_host_accessible !39 !kernel_arg_pipe_depth !40 !kernel_arg_pipe_io !41 !kernel_arg_buffer_location !41 !no_barrier_path !42 !kernel_has_sub_groups !39 !kernel_execution_length !43 !kernel_has_global_sync !39 !recommended_vector_length !34 !arg_type_null_val !61 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !44, metadata !DIExpression()), !dbg !45
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !46, metadata !DIExpression()), !dbg !45
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !47, metadata !DIExpression()), !dbg !45
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !48, metadata !DIExpression()), !dbg !49
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !50
  %call = call i32 @foo(ptr addrspace(1) noundef %0) #7, !dbg !51
  ret void, !dbg !52
}

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test_barrier(ptr addrspace(1) noalias noundef align 4 %dst) #4 !dbg !53 !kernel_arg_addr_space !34 !kernel_arg_access_qual !35 !kernel_arg_type !36 !kernel_arg_base_type !36 !kernel_arg_type_qual !37 !kernel_arg_name !38 !kernel_arg_host_accessible !39 !kernel_arg_pipe_depth !40 !kernel_arg_pipe_io !41 !kernel_arg_buffer_location !41 !no_barrier_path !39 !kernel_has_sub_groups !39 !kernel_execution_length !54 !kernel_has_global_sync !39 !recommended_vector_length !34 !arg_type_null_val !61 {
entry:
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !55, metadata !DIExpression()), !dbg !56
  call void @_Z7barrierj(i32 noundef 1) #8, !dbg !57
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !58
  %call = call i32 @bar(ptr addrspace(1) noundef %0) #7, !dbg !59
  ret void, !dbg !60
}

; Function Attrs: convergent
declare void @_Z7barrierj(i32 noundef) #5

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #4 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #5 = { convergent "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #6 = { convergent nounwind readnone willreturn }
attributes #7 = { convergent }
attributes #8 = { convergent "kernel-call-once" "kernel-convergent-call" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

; CHECK: [[DBGGidFoo]] = !DILocation(line: 2, column: 7, scope:

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{ptr @test, ptr @test_barrier}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!7 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !8)
!8 = !{!9, !10}
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !DIDerivedType(tag: DW_TAG_restrict_type, baseType: !11)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64)
!12 = !{}
!13 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, line: 1, type: !14, flags: DIFlagArtificial)
!14 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!15 = !DILocation(line: 1, scope: !6)
!16 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, line: 1, type: !14, flags: DIFlagArtificial)
!17 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, line: 1, type: !14, flags: DIFlagArtificial)
!18 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 1, type: !10)
!19 = !DILocation(line: 1, column: 57, scope: !6)
!20 = !DILocation(line: 2, column: 3, scope: !6)
!21 = !DILocation(line: 2, column: 7, scope: !6)
!22 = !DILocation(line: 2, column: 25, scope: !6)
!23 = !DILocation(line: 3, column: 1, scope: !6)
!24 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 5, type: !7, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!25 = !DILocalVariable(name: "dst", arg: 1, scope: !24, file: !1, line: 5, type: !10)
!26 = !DILocation(line: 5, column: 57, scope: !24)
!27 = !DILocation(line: 6, column: 3, scope: !24)
!28 = !DILocation(line: 6, column: 7, scope: !24)
!29 = !DILocation(line: 6, column: 25, scope: !24)
!30 = !DILocation(line: 7, column: 1, scope: !24)
!31 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 9, type: !32, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!32 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !33)
!33 = !{null, !10}
!34 = !{i32 1}
!35 = !{!"none"}
!36 = !{!"int*"}
!37 = !{!"restrict"}
!38 = !{!"dst"}
!39 = !{i1 false}
!40 = !{i32 0}
!41 = !{!""}
!42 = !{i1 true}
!43 = !{i32 6}
!44 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !31, line: 1, type: !14, flags: DIFlagArtificial)
!45 = !DILocation(line: 9, scope: !31)
!46 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !31, line: 1, type: !14, flags: DIFlagArtificial)
!47 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !31, line: 1, type: !14, flags: DIFlagArtificial)
!48 = !DILocalVariable(name: "dst", arg: 1, scope: !31, file: !1, line: 9, type: !10)
!49 = !DILocation(line: 9, column: 40, scope: !31)
!50 = !DILocation(line: 10, column: 7, scope: !31)
!51 = !DILocation(line: 10, column: 3, scope: !31)
!52 = !DILocation(line: 11, column: 1, scope: !31)
!53 = distinct !DISubprogram(name: "test_barrier", scope: !1, file: !1, line: 13, type: !32, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!54 = !{i32 7}
!55 = !DILocalVariable(name: "dst", arg: 1, scope: !53, file: !1, line: 13, type: !10)
!56 = !DILocation(line: 13, column: 48, scope: !53)
!57 = !DILocation(line: 14, column: 3, scope: !53)
!58 = !DILocation(line: 15, column: 7, scope: !53)
!59 = !DILocation(line: 15, column: 3, scope: !53)
!60 = !DILocation(line: 16, column: 1, scope: !53)
!61 = !{ptr addrspace(1) null}

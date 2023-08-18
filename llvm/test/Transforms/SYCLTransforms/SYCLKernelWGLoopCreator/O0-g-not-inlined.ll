; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

; This test checks that WG loops are created in O0 and -g mode. Not-inlined
; function read local id from TLS global.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @__LocalIds = internal thread_local global [3 x i64] undef, align 16

; CHECK-LABEL: @foo
; CHECK-NEXT: entry:
; CHECK-NEXT:  %lid1.addr = alloca i64, align 8
; CHECK-NEXT:  %lid2.addr = alloca i64, align 8
; CHECK-NEXT:  %gid0.addr = alloca i64, align 8
; CHECK-NEXT:  %gid1.addr = alloca i64, align 8
; CHECK-NEXT:  %gid2.addr = alloca i64, align 8
; CHECK-NEXT:  %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT:  %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT:  %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT:  %dst.addr = alloca ptr addrspace(1), align 8
; CHECK-NEXT:  %lid0.addr = alloca i64, align 8
; CHECK-NEXT:  %lid0 = load i64, ptr @__LocalIds, align 8
; CHECK-NEXT:  store i64 %lid0, ptr %lid0.addr, align 8
; CHECK-NEXT:  %lid1 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1), align 8
; CHECK-NEXT:  store i64 %lid1, ptr %lid1.addr, align 8
; CHECK-NEXT:  %lid2 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2), align 8
; CHECK-NEXT:  store i64 %lid2, ptr %lid2.addr, align 8
; CHECK-NEXT:  %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT:  %gid0 = add i64 %lid0, %base.gid0
; CHECK-NEXT:  store i64 %gid0, ptr %gid0.addr, align 8
; CHECK-NEXT:  %base.gid1 = call i64 @get_base_global_id.(i32 1)
; CHECK-NEXT:  %gid1 = add i64 %lid1, %base.gid1
; CHECK-NEXT:  store i64 %gid1, ptr %gid1.addr, align 8
; CHECK-NEXT:  %base.gid2 = call i64 @get_base_global_id.(i32 2)
; CHECK-NEXT:  %gid2 = add i64 %lid2, %base.gid2
; CHECK-NEXT:  store i64 %gid2, ptr %gid2.addr, align 8
; CHECK-NEXT:  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !{{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT:  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !{{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT:  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !{{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT:  %gid0.ld = load i64, ptr %gid0.addr, align 8
; CHECK-NEXT:  store volatile i64 %gid0.ld, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT:  %gid1.ld = load i64, ptr %gid1.addr, align 8
; CHECK-NEXT:  store volatile i64 %gid1.ld, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT:  %gid2.ld = load i64, ptr %gid2.addr, align 8
; CHECK-NEXT:  store volatile i64 %gid2.ld, ptr %__ocl_dbg_gid2, align 8
; CHECK-NEXT:  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
; CHECK-NEXT:  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !{{.*}}, metadata !DIExpression()), !dbg
; CHECK-NEXT:  [[LOAD:%[0-9]+]] = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg
; CHECK-NEXT:  %gid0.ld1 = load i64, ptr %gid0.addr, align 8, !dbg [[DBGGid:![0-9]+]]
; CHECK-NEXT:  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) [[LOAD]], i64 %gid0.ld1, !dbg

; CHECK: [[DBGGid]] = !DILocation(line: 3, column: 7, scope: !6)

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @foo(ptr addrspace(1) noalias noundef %dst) #0 !dbg !6 !kernel_arg_base_type !27 !arg_type_null_val !43 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !12, metadata !DIExpression()), !dbg !14
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !15, metadata !DIExpression()), !dbg !14
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !16, metadata !DIExpression()), !dbg !14
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !17, metadata !DIExpression()), !dbg !18
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !19
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #4, !dbg !20
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call, !dbg !19
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !21
  ret void, !dbg !22
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #3 !dbg !23 !kernel_arg_addr_space !25 !kernel_arg_access_qual !26 !kernel_arg_type !27 !kernel_arg_base_type !27 !kernel_arg_type_qual !28 !kernel_arg_name !29 !kernel_arg_host_accessible !30 !kernel_arg_pipe_depth !31 !kernel_arg_pipe_io !28 !kernel_arg_buffer_location !28 !no_barrier_path !32 !kernel_has_sub_groups !30 !kernel_execution_length !33 !kernel_has_global_sync !30 !recommended_vector_length !25 !arg_type_null_val !43 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !34, metadata !DIExpression()), !dbg !35
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !36, metadata !DIExpression()), !dbg !35
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !37, metadata !DIExpression()), !dbg !35
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !38, metadata !DIExpression()), !dbg !39
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !40
  call void @foo(ptr addrspace(1) noundef %0) #5, !dbg !41
  ret void, !dbg !42
}

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #4 = { convergent nounwind readnone willreturn }
attributes #5 = { convergent }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.compiler.options = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!5 = !{ptr @test}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 2, type: !7, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !11)
!7 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{}
!12 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, line: 1, type: !13, flags: DIFlagArtificial)
!13 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!14 = !DILocation(line: 2, scope: !6)
!15 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, line: 1, type: !13, flags: DIFlagArtificial)
!16 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, line: 1, type: !13, flags: DIFlagArtificial)
!17 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 2, type: !9)
!18 = !DILocation(line: 2, column: 22, scope: !6)
!19 = !DILocation(line: 3, column: 3, scope: !6)
!20 = !DILocation(line: 3, column: 7, scope: !6)
!21 = !DILocation(line: 3, column: 25, scope: !6)
!22 = !DILocation(line: 4, column: 1, scope: !6)
!23 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 6, type: !24, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !11)
!24 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!25 = !{i32 1}
!26 = !{!"none"}
!27 = !{!"int*"}
!28 = !{!""}
!29 = !{!"dst"}
!30 = !{i1 false}
!31 = !{i32 0}
!32 = !{i1 true}
!33 = !{i32 6}
!34 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !23, line: 1, type: !13, flags: DIFlagArtificial)
!35 = !DILocation(line: 6, scope: !23)
!36 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !23, line: 1, type: !13, flags: DIFlagArtificial)
!37 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !23, line: 1, type: !13, flags: DIFlagArtificial)
!38 = !DILocalVariable(name: "dst", arg: 1, scope: !23, file: !1, line: 6, type: !9)
!39 = !DILocation(line: 6, column: 30, scope: !23)
!40 = !DILocation(line: 7, column: 7, scope: !23)
!41 = !DILocation(line: 7, column: 3, scope: !23)
!42 = !DILocation(line: 8, column: 1, scope: !23)
!43 = !{ptr addrspace(1) null}

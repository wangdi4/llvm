; RUN: opt -implicit-gid-handle-barrier=false -passes=sycl-kernel-implicit-gid -S %s | FileCheck %s

; This test checks that implicit gids are added to non-inlined function.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @foo(ptr addrspace(1) noalias noundef %dst) #0 !dbg !6 !kernel_arg_base_type !34 !arg_type_null_val !35 {
entry:
; CHECK-LABEL: @foo
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata [[fooDbgMD0:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata [[fooDbgMD1:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata [[fooDbgMD2:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !12, metadata !DIExpression()), !dbg !13
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !14
  %call = call i64 @_Z13get_global_idj(i32 noundef 0) #4, !dbg !15
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %0, i64 %call, !dbg !14
  store i32 0, ptr addrspace(1) %arrayidx, align 4, !dbg !16
  ret void, !dbg !17
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #2

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(ptr addrspace(1) noalias noundef align 4 %dst) #3 !dbg !18 !kernel_arg_addr_space !20 !kernel_arg_access_qual !21 !kernel_arg_type !22 !kernel_arg_base_type !22 !kernel_arg_type_qual !23 !kernel_arg_name !24 !kernel_arg_host_accessible !25 !kernel_arg_pipe_depth !26 !kernel_arg_pipe_io !23 !kernel_arg_buffer_location !23 !no_barrier_path !27 !kernel_has_sub_groups !25 !kernel_execution_length !28 !kernel_has_global_sync !25 !recommended_vector_length !20 !arg_type_null_val !35 {
entry:
; CHECK-LABEL: @test
; CHECK: %__ocl_dbg_gid0 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata [[DbgMD0:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid1 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata [[DbgMD1:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %__ocl_dbg_gid2 = alloca i64, align 8
; CHECK-NEXT: call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata [[DbgMD2:![0-9]+]], metadata !DIExpression()), !dbg
; CHECK-NEXT: %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT: store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
; CHECK-NEXT: %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
; CHECK-NEXT: store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
; CHECK-NEXT: %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
; CHECK-NEXT: store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8

  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %dst.addr, metadata !29, metadata !DIExpression()), !dbg !30
  %0 = load ptr addrspace(1), ptr %dst.addr, align 8, !dbg !31
  call void @foo(ptr addrspace(1) noundef %0) #5, !dbg !32
  ret void, !dbg !33
}

; CHECK: [[fooDbgMD0]] = !DILocalVariable(name: "__ocl_dbg_gid0",
; CHECK: [[fooDbgMD1]] = !DILocalVariable(name: "__ocl_dbg_gid1",
; CHECK: [[fooDbgMD2]] = !DILocalVariable(name: "__ocl_dbg_gid2",

; CHECK: [[DbgMD0]] = !DILocalVariable(name: "__ocl_dbg_gid0",
; CHECK: [[DbgMD1]] = !DILocalVariable(name: "__ocl_dbg_gid1",
; CHECK: [[DbgMD2]] = !DILocalVariable(name: "__ocl_dbg_gid2",

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
!12 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 2, type: !9)
!13 = !DILocation(line: 2, column: 22, scope: !6)
!14 = !DILocation(line: 3, column: 3, scope: !6)
!15 = !DILocation(line: 3, column: 7, scope: !6)
!16 = !DILocation(line: 3, column: 25, scope: !6)
!17 = !DILocation(line: 4, column: 1, scope: !6)
!18 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 6, type: !19, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !11)
!19 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!20 = !{i32 1}
!21 = !{!"none"}
!22 = !{!"int*"}
!23 = !{!""}
!24 = !{!"dst"}
!25 = !{i1 false}
!26 = !{i32 0}
!27 = !{i1 true}
!28 = !{i32 6}
!29 = !DILocalVariable(name: "dst", arg: 1, scope: !18, file: !1, line: 6, type: !9)
!30 = !DILocation(line: 6, column: 30, scope: !18)
!31 = !DILocation(line: 7, column: 7, scope: !18)
!32 = !DILocation(line: 7, column: 3, scope: !18)
!33 = !DILocation(line: 8, column: 1, scope: !18)
!34 = !{!"int*"}
!35 = !{ptr addrspace(1) null}

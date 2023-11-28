; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s

; This test checks that early_exit_call in bar doesn't have debug info.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(ptr addrspace(1) noalias noundef align 4 %dst) local_unnamed_addr #0 !dbg !6 !no_barrier_path !17 !kernel_has_sub_groups !18 !max_wg_dimensions !19 {
entry:
  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !20, metadata !DIExpression()), !dbg !22
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !23, metadata !DIExpression()), !dbg !22
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !24, metadata !DIExpression()), !dbg !22
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  ret void, !dbg !25
}

; Function Attrs: convergent mustprogress nofree nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) local_unnamed_addr #1

; Function Attrs: convergent norecurse nounwind
define dso_local void @bar(ptr addrspace(1) noalias noundef align 4 %dst) local_unnamed_addr #0 !dbg !26 !no_barrier_path !17 !kernel_has_sub_groups !18 !max_wg_dimensions !19 {
entry:
; CHECK-LABEL: @bar(
; CHECK: %early_exit_call = call [7 x i64] @WG.boundaries.bar(ptr addrspace(1) %dst)
; CHECK-NOT: !dbg
; CHECK-NEXT: %uniform.early.exit = extractvalue [7 x i64] %early_exit_call, 0

  %__ocl_dbg_gid0 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !30, metadata !DIExpression()), !dbg !31
  %__ocl_dbg_gid1 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !32, metadata !DIExpression()), !dbg !31
  %__ocl_dbg_gid2 = alloca i64, align 8
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !33, metadata !DIExpression()), !dbg !31
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store volatile i64 %GlobalID_0, ptr %__ocl_dbg_gid0, align 8
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store volatile i64 %GlobalID_1, ptr %__ocl_dbg_gid1, align 8
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store volatile i64 %GlobalID_2, ptr %__ocl_dbg_gid2, align 8
  ret void, !dbg !34
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

define [7 x i64] @WG.boundaries.bar(ptr addrspace(1) %0) {
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

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent mustprogress nofree nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 2, i32 0}
!5 = !{ptr @foo, ptr @bar}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !11)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{null, !9}
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12, !13}
!12 = !DILocalVariable(name: "dst", arg: 1, scope: !6, file: !1, line: 1, type: !9)
!13 = !DILocalVariable(name: "gid", scope: !6, file: !1, line: 2, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "size_t", file: !15, line: 118, baseType: !16)
!15 = !DIFile(filename: "opencl-c-base.h", directory: "")
!16 = !DIBasicType(name: "unsigned long", size: 64, encoding: DW_ATE_unsigned)
!17 = !{i1 true}
!18 = !{i1 false}
!19 = !{i32 1}
!20 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !6, line: 1, type: !21, flags: DIFlagArtificial)
!21 = !DIBasicType(name: "ind type", size: 64, encoding: DW_ATE_unsigned)
!22 = !DILocation(line: 1, scope: !6)
!23 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !6, line: 1, type: !21, flags: DIFlagArtificial)
!24 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !6, line: 1, type: !21, flags: DIFlagArtificial)
!25 = !DILocation(line: 4, column: 1, scope: !6)
!26 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 6, type: !7, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !27)
!27 = !{!28, !29}
!28 = !DILocalVariable(name: "dst", arg: 1, scope: !26, file: !1, line: 6, type: !9)
!29 = !DILocalVariable(name: "gid", scope: !26, file: !1, line: 7, type: !14)
!30 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !26, line: 1, type: !21, flags: DIFlagArtificial)
!31 = !DILocation(line: 6, scope: !26)
!32 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !26, line: 1, type: !21, flags: DIFlagArtificial)
!33 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !26, line: 1, type: !21, flags: DIFlagArtificial)
!34 = !DILocation(line: 9, column: 1, scope: !26)

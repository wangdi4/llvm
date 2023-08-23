; RUN: opt -passes=sycl-kernel-implicit-gid -S %s | FileCheck %s

; Check that implicit gid is not inserted since emissionKind is LineTablesOnly.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: %__ocl_dbg_gid

define dso_local void @test() local_unnamed_addr #0 !dbg !6 !kernel_arg_base_type !9 !arg_type_null_val !10 !no_barrier_path !11 !kernel_has_sub_groups !12 {
entry:
  ret void, !dbg !13
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(none) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!sycl.kernels = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "/")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 3, i32 0}
!5 = !{ptr @test}
!6 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 393, type: !7, scopeLine: 400, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!7 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !8)
!8 = !{}
!9 = !{!"float*"}
!10 = !{ptr addrspace(1) null}
!11 = !{i1 true}
!12 = !{i1 false}
!13 = !DILocation(line: 415, column: 1, scope: !6)

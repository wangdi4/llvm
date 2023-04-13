; Checks that !gdb metadata is not preserved after cleaning up function.

; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: @foo
; CHECK-NOT: !dbg

define dso_local void @foo() #0 !dbg !14 {
entry:
  ret void, !dbg !17
}

define dso_local void @test() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 {
entry:
  ret void
}

attributes #0 = { convergent norecurse nounwind }

!llvm.dbg.cu = !{!0}
!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!3, !3}
!opencl.spir.version = !{!3, !3}
!opencl.used.extensions = !{!2, !2}
!opencl.used.optional.core.features = !{!2, !2}
!opencl.compiler.options = !{!4, !4}
!llvm.ident = !{!5, !5}
!opencl.stat.type = !{!6, !6}
!opencl.stat.exec_time = !{!7, !7}
!opencl.stat.run_time_version = !{!8, !8}
!opencl.stat.workload_name = !{!9, !9}
!opencl.stat.module_name = !{!10, !10}
!llvm.module.flags = !{!11, !12}
!sycl.kernels = !{!13}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "two-func.cl", directory: "")
!2 = !{}
!3 = !{i32 2, i32 0}
!4 = !{!"-cl-std=CL2.0", !"-g"}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!6 = !{!""}
!7 = !{!"2021-06-20 14:56:16"}
!8 = !{!"2021.12.6.0"}
!9 = !{!""}
!10 = !{!""}
!11 = !{i32 7, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{void ()* @test}
!14 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !15, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!15 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !16)
!16 = !{null}
!17 = !DILocation(line: 5, column: 1, scope: !14)

; DEBUGIFY: ModuleDebugify: Skipping module with debug info

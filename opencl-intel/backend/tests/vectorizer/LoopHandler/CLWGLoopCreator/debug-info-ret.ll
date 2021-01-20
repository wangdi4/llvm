; This test checks that ret instruction's DebugLoc is preserved by
; CLWGLoopCreator pass.
;
; The IR is dumped at the beginning of CLWGLoopCreator::runOnModule()
; from source with build option "-g":
; kernel void test() {
; }
;
; RUN: %oclopt -cl-loop-creator -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test() local_unnamed_addr #0 !dbg !14 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !no_barrier_path !18 !kernel_execution_length !19 !kernel_has_barrier !20 !kernel_has_global_sync !20 !max_wg_dimensions !21 {
entry:
; CHECK: ret void, !dbg
  ret void, !dbg !22
}

define [7 x i64] @WG.boundaries.test() {
entry:
  %0 = call i64 @_Z14get_local_sizej(i32 0)
  %1 = call i64 @get_base_global_id.(i32 0)
  %2 = call i64 @_Z14get_local_sizej(i32 1)
  %3 = call i64 @get_base_global_id.(i32 1)
  %4 = call i64 @_Z14get_local_sizej(i32 2)
  %5 = call i64 @get_base_global_id.(i32 2)
  %6 = insertvalue [7 x i64] undef, i64 %0, 2
  %7 = insertvalue [7 x i64] %6, i64 %1, 1
  %8 = insertvalue [7 x i64] %7, i64 %2, 4
  %9 = insertvalue [7 x i64] %8, i64 %3, 3
  %10 = insertvalue [7 x i64] %9, i64 %4, 6
  %11 = insertvalue [7 x i64] %10, i64 %5, 5
  %12 = insertvalue [7 x i64] %11, i64 1, 0
  ret [7 x i64] %12
}

declare i64 @_Z14get_local_sizej(i32)

declare i64 @get_base_global_id.(i32)

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }

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
!opencl.stat.type = !{!8}
!opencl.stat.exec_time = !{!9}
!opencl.stat.run_time_version = !{!10}
!opencl.stat.workload_name = !{!11}
!opencl.stat.module_name = !{!12}
!opencl.kernels = !{!13}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "empty.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-g"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{!""}
!9 = !{!"2020-06-07 13:49:01"}
!10 = !{!"2020.10.6.0"}
!11 = !{!""}
!12 = !{!""}
!13 = !{void ()* @test}
!14 = distinct !DISubprogram(name: "test", scope: !15, file: !15, line: 1, type: !16, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!15 = !DIFile(filename: "empty.cl", directory: "")
!16 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !17)
!17 = !{null}
!18 = !{i1 true}
!19 = !{i32 1}
!20 = !{i1 false}
!21 = !{i32 0}
!22 = !DILocation(line: 2, column: 1, scope: !14)

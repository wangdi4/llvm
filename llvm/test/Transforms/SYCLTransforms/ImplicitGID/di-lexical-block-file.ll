; RUN: opt -passes=sycl-kernel-implicit-gid -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-implicit-gid -S %s | FileCheck %s

; This test checks that DILexicalBlockFile scope can be identified by
; ImplicitGlobalIdPass.
;
; The IR is dumped at the beginning of ImplicitGlobalIdPass::runOnModule()
; from source with build option "-g":
; kernel void test() {
; }
; Then the IR is modified by appending fake DILexicalBlockFile metadata.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define void @test() local_unnamed_addr #0 !dbg !9 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !kernel_execution_length !13 !no_barrier_path !14 !kernel_has_global_sync !14 {
entry:
; CHECK-LABEL: entry:
; CHECK-NEXT: call void @dummybarrier.()
; CHECK-NEXT: %__ocl_dbg_gid0 = alloca i64
  call void @dummybarrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z7barrierj(i32 1)
  ret void, !dbg !15
}

declare void @dummybarrier.()

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #1

attributes #0 = { convergent norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { convergent }

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
!sycl.kernels = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "empty.cl", directory: ".")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 2, i32 0}
!6 = !{!"-cl-std=CL2.0", !"-g", !"-cl-fast-relaxed-math"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{ptr @test}
!9 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!10 = !DIFile(filename: "empty.cl", directory: ".")
!11 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !12)
!12 = !{null}
!13 = !{i32 1}
!14 = !{i1 false}
!15 = !DILocation(line: 2, column: 2, scope: !16)
!16 = !DILexicalBlockFile(scope: !9, file: !10, discriminator: 0)

; DEBUGIFY-NOT: WARNING

; RUN: opt -S -passes=sycl-kernel-barrier %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) #0 !dbg !9 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !kernel_execution_length !12 !no_barrier_path !13 !kernel_has_global_sync !13 {
entry:
  call void @dummy_barrier.()
  call void @llvm.dbg.value(metadata i32 %a, metadata !14, metadata !DIExpression()), !dbg !16
  %g = alloca i32, align 4
  call void @llvm.dbg.value(metadata ptr %g, metadata !17, metadata !DIExpression(DW_OP_deref)), !dbg !18
  store i32 1, ptr %g, align 4, !dbg !18
  br label %barrier

barrier:
  call void @_Z18work_group_barrierj(i32 1)
  ret void, !dbg !16
}

; Debug information for argument %a should be hoisted into the entry basic
; block.

; CHECK:      define void @test(i32 %a)
; CHECK-SAME:   !dbg [[TEST:![0-9]+]]
; CHECK-SAME:   {
; CHECK:      entry:
; CHECK:        call void @llvm.dbg.value(metadata i32 %a, metadata [[A:![0-9]+]], metadata !DIExpression()
; CHECK:        br label %FirstBB
;
; CHECK:      FirstBB:
; CHECK:        br label %SyncBB1
;
; CHECK:      SyncBB1:
; CHECK-NOT:    call void @llvm.dbg.value(metadata i32 %a,
; CHECK:        call void @llvm.dbg.value(metadata ptr undef, metadata !{{.*}}, metadata !DIExpression(DW_OP_deref)
; CHECK:        br label %barrier
; CHECK:      }

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1
declare void @llvm.dbg.value(metadata, metadata, metadata) #1
declare void @dummy_barrier.()
declare void @_Z18work_group_barrierj(i32) #2
declare i64 @_Z13get_global_idj(i32) #3

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent }
attributes #3 = { nounwind readnone }

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

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "SyncBBUsers.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, i32 2}
!6 = !{!"-g", !"-cl-opt-disable"}
!7 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!8 = !{ptr @test}
!9 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !10, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !11)
!11 = !{null}
!12 = !{i32 4}
!13 = !{i1 false}
!14 = !DILocalVariable(name: "a", scope: !9, file: !1, line: 3, type: !15)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocation(line: 4, column: 1, scope: !9)
!17 = !DILocalVariable(name: "g", scope: !9, file: !1, line: 3, type: !15)
!18 = !DILocation(line: 3, column: 7, scope: !9)

; CHECK: [[TEST]] = distinct !DISubprogram(name: "test"
; CHECK: [[A]] = !DILocalVariable(name: "a"
; CHECK:                        scope: [[TEST]]
; CHECK:                        type: [[INT:![0-9]+]]
; CHECK: [[INT]] = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)


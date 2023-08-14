; This test checks that alloca's user is correctly handled if it is in SyncBB.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule() from source:
;
; kernel void test() {
;   int g = 1;
; }
;
; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define void @test() #0 !dbg !9 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 !kernel_execution_length !13 !no_barrier_path !14 !kernel_has_global_sync !14 {
entry:
; CHECK-LABEL: entry:
; CHECK: %g.addr = alloca ptr
; CHECK-NOT: %g = alloca i32

  call void @dummy_barrier.()
  %__ocl_dbg_gid0 = alloca i64
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid0, metadata !15, metadata !DIExpression()), !dbg !18
  %GlobalID_01 = call i64 @_Z13get_global_idj(i32 0)
  store i64 %GlobalID_01, ptr %__ocl_dbg_gid0
  %__ocl_dbg_gid1 = alloca i64
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid1, metadata !19, metadata !DIExpression()), !dbg !18
  %GlobalID_12 = call i64 @_Z13get_global_idj(i32 1)
  store i64 %GlobalID_12, ptr %__ocl_dbg_gid1
  %__ocl_dbg_gid2 = alloca i64
  call void @llvm.dbg.declare(metadata ptr %__ocl_dbg_gid2, metadata !20, metadata !DIExpression()), !dbg !18
  %GlobalID_23 = call i64 @_Z13get_global_idj(i32 2)
  store i64 %GlobalID_23, ptr %__ocl_dbg_gid2
  %g = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %g, metadata !21, metadata !DIExpression()), !dbg !18
  store i32 1, ptr %g, align 4, !dbg !18

; CHECK-LABEL: SyncBB1:
; CHECK:      [[Index:%SBIndex]] = load i64, ptr %pCurrSBIndex
; CHECK-NEXT: [[Offset:%SB_LocalId_Offset]] = add nuw i64 [[Index]], {{[0-9]+}}
; CHECK-NEXT: [[GEP:%pSB_LocalId[0-9]*]] = getelementptr inbounds i8, ptr %pSB, i64 [[Offset]]
; CHECK-NEXT: store ptr [[GEP]], ptr %g.addr
; CHECK-NEXT: [[G:%[0-9]+]] = load ptr, ptr %g.addr
; CHECK:      call void @llvm.dbg.declare(metadata ptr %g.addr, metadata !{{[0-9]+}}, metadata !DIExpression(DW_OP_deref)), !dbg
; CHECK-NEXT: store i32 1, ptr [[G]], align 4, !dbg

  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %GlobalID_2 = call i64 @_Z13get_global_idj(i32 2)
  store i64 %GlobalID_2, ptr %__ocl_dbg_gid2
  %GlobalID_1 = call i64 @_Z13get_global_idj(i32 1)
  store i64 %GlobalID_1, ptr %__ocl_dbg_gid1
  %GlobalID_0 = call i64 @_Z13get_global_idj(i32 0)
  store i64 %GlobalID_0, ptr %__ocl_dbg_gid0
  ret void, !dbg !23
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #2

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #3

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
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
!9 = distinct !DISubprogram(name: "test", scope: !10, file: !10, line: 1, type: !11, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!10 = !DIFile(filename: "SyncBBUsers.cl", directory: "")
!11 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !12)
!12 = !{null}
!13 = !{i32 4}
!14 = !{i1 false}
!15 = !DILocalVariable(name: "__ocl_dbg_gid0", scope: !9, file: !16, line: 1, type: !17, flags: DIFlagArtificial)
!16 = !DIFile(filename: "", directory: "")
!17 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!18 = !DILocation(line: 3, column: 7, scope: !9)
!19 = !DILocalVariable(name: "__ocl_dbg_gid1", scope: !9, file: !16, line: 1, type: !17, flags: DIFlagArtificial)
!20 = !DILocalVariable(name: "__ocl_dbg_gid2", scope: !9, file: !16, line: 1, type: !17, flags: DIFlagArtificial)
!21 = !DILocalVariable(name: "g", scope: !9, file: !10, line: 3, type: !22)
!22 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!23 = !DILocation(line: 4, column: 1, scope: !9)

; DEBUGIFY-NOT: WARNING

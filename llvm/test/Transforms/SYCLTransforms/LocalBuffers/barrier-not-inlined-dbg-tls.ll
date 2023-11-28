; RUN: opt -passes=sycl-kernel-local-buffers -S %s | FileCheck %s

; Check for tls mode. Local variable used in scalar kernel and vector kernel has
; the same offset in both kernels. Local buffer size is the same for both kernels.
; Note that barrier loops are simplified.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32, !dbg !0
@__pLocalMemBase = linkonce_odr thread_local global ptr addrspace(3) undef, align 8

; CHECK: @test.i = external addrspace(3) global i32,
; CHECK: @__pLocalMemBase = linkonce_odr thread_local global ptr addrspace(3) undef, align 8

define internal fastcc void @foo() {
; CHECK: define internal fastcc void @foo()
; CHECK-NOT: !local_buffer_size
; CHECK-SAME: {
; CHECK:   %LocalMemBase = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
; CHECK:   [[GEP0:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %LocalMemBase, i32 0
; CHECK:   store i32 1, ptr addrspace(3) [[GEP0]], align 4, !dbg
;
entry:
  store i32 1, ptr addrspace(3) @test.i, align 4, !dbg !14
  ret void
}

define dso_local void @test(ptr addrspace(1) noundef align 4 %dst) !dbg !2 !kernel_arg_addr_space !17 !vectorized_kernel !18 !vectorized_width !17 {
; CHECK: define dso_local void @test(ptr addrspace(1) noundef align 4 %dst)
; CHECK-SAME: !vectorized_kernel [[VECTORIZED_KERNEL:![0-9]+]]
; CHECK-SAME: !vectorized_width [[VECTORIZED_WIDTH1:![0-9]+]]
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE:![0-9]+]]
; CHECK:   %LocalMemBase = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
; CHECK:   [[GEP1:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %LocalMemBase, i32 0
; CHECK:   call void @llvm.dbg.value(metadata ptr addrspace(3) [[GEP1]], metadata {{.*}}, metadata !DIExpression(DW_OP_deref)), !dbg
; CHECK:   store i32 %conv, ptr addrspace(3) [[GEP1]], align 4, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   [[LOAD1:%[0-9]+]] = load i32, ptr addrspace(3) [[GEP1]], align 4, !dbg
; CHECK:   store i32 [[LOAD1]], ptr addrspace(1) %dst, align 4, !dbg
;
entry:
  br label %SyncBB2

SyncBB2:                                          ; preds = %SyncBB2, %entry
  %conv = trunc i64 0 to i32
  store i32 %conv, ptr addrspace(3) @test.i, align 4, !dbg !19
  br label %SyncBB2

Dispatch8:                                        ; preds = %Dispatch8
  tail call fastcc void @foo() #0, !dbg !20
  %0 = load i32, ptr addrspace(3) @test.i, align 4, !dbg !21
  store i32 %0, ptr addrspace(1) %dst, align 4, !dbg !22
  br label %Dispatch8
}

define dso_local void @_ZGVeN16u_test(ptr addrspace(1) noundef align 4 %dst) !vectorized_width !23 !scalar_kernel !13 {
; CHECK: define dso_local void @_ZGVeN16u_test(ptr addrspace(1) noundef align 4 %dst)
; CHECK-SAME: !vectorized_width [[VECTORIZED_WIDTH16:![0-9]+]]
; CHECK-SAME: !scalar_kernel [[SCALAR_KERNEL:![0-9]+]]
; CHECK-SAME: !local_buffer_size [[LOCAL_SIZE]]
; CHECK:   %LocalMemBase = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
; CHECK:   [[GEP2:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %LocalMemBase, i32 0
; CHECK:   store i32 %.extract.15..lcssa, ptr addrspace(3) [[GEP2]], align 4, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   tail call fastcc void @foo() {{.*}}, !dbg
; CHECK:   [[LOAD2:%[0-9]+]] = load i32, ptr addrspace(3) [[GEP2]], align 4, !dbg
; CHECK:   store i32 [[LOAD2]], ptr addrspace(1) %dst, align 4, !dbg
;
entry:
  br label %LoopEnd_0

LoopEnd_0:                                        ; preds = %entry
  %.extract.15..lcssa = phi i32 [ 0, %entry ]
  store i32 %.extract.15..lcssa, ptr addrspace(3) @test.i, align 4, !dbg !24
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  tail call fastcc void @foo() #0, !dbg !26
  %0 = load i32, ptr addrspace(3) @test.i, align 4, !dbg !27
  store i32 %0, ptr addrspace(1) %dst, align 4, !dbg !28
  ret void
}

attributes #0 = { convergent }

!llvm.dbg.cu = !{!6}
!llvm.module.flags = !{!11, !12}
!sycl.kernels = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "i", scope: !2, file: !3, line: 6, type: !10, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "test", scope: !3, file: !3, line: 5, type: !4, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !6, retainedNodes: !9)
!3 = !DIFile(filename: "local.cl2.barrier.foo", directory: "")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !7, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !8, splitDebugInlining: false, nameTableKind: None)
!7 = !DIFile(filename: "test.cl", directory: "")
!8 = !{!0}
!9 = !{}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{i32 7, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{ptr @test}
!14 = !DILocation(line: 2, column: 6, scope: !15)
!15 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 1, type: !16, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !6, retainedNodes: !9)
!16 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !5)
!17 = !{i32 1}
!18 = !{ptr @_ZGVeN16u_test}
!19 = !DILocation(line: 7, column: 5, scope: !2)
!20 = !DILocation(line: 9, column: 3, scope: !2)
!21 = !DILocation(line: 10, column: 12, scope: !2)
!22 = !DILocation(line: 10, column: 10, scope: !2)
!23 = !{i32 16}
!24 = !DILocation(line: 7, column: 5, scope: !25)
!25 = distinct !DISubprogram(name: "test", scope: !3, file: !3, line: 5, type: !4, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !6, retainedNodes: !9)
!26 = !DILocation(line: 9, column: 3, scope: !25)
!27 = !DILocation(line: 10, column: 12, scope: !25)
!28 = !DILocation(line: 10, column: 10, scope: !25)

; CHECK: [[SCALAR_KERNEL]] = !{ptr @test}
; CHECK: [[VECTORIZED_WIDTH1]] = !{i32 1}
; CHECK: [[VECTORIZED_KERNEL]] = !{ptr @_ZGVeN16u_test}
; CHECK: [[LOCAL_SIZE]] = !{i32 4}
; CHECK: [[VECTORIZED_WIDTH16]] = !{i32 16}

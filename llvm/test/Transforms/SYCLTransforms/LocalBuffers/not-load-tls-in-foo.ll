; RUN: opt -sycl-kernel-enable-tls-globals -passes=sycl-kernel-local-buffers -S %s | FileCheck %s

; Check that there is no load from pLocalMemBase in function foo which doesn't
; use local variable.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.i = external addrspace(3) global i32, !dbg !0
@LocalIds = linkonce_odr thread_local global [3 x i64] undef, align 16
@pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8
@pWorkDim = linkonce_odr thread_local global { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}*, [3 x i64], [2 x [3 x i64]], [3 x i64] }* undef, align 8
@pWGId = linkonce_odr thread_local global i64* undef, align 8
@BaseGlbId = linkonce_odr thread_local global [4 x i64] undef, align 16
@pSpecialBuf = linkonce_odr thread_local global i8* undef, align 8
@RuntimeHandle = linkonce_odr thread_local global {}* undef, align 8

; Function Attrs: convergent
define dso_local void @foo() #0 !dbg !13 {
; CHECK-LABEL: define dso_local void @foo()
; CHECK-NOT: @pLocalMemBase
;
entry:
  ret void, !dbg !15
}

define dso_local void @test() !dbg !2 {
; CHECK-LABEL: define dso_local void @test()
; CHECK: %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8, !dbg
;
entry:
  store i32 0, i32 addrspace(3)* @test.i, align 4, !dbg !16
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { convergent }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!6}
!llvm.module.flags = !{!10, !11}
!sycl.kernels = !{!12}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "i", scope: !2, file: !3, line: 5, type: !9, isLocal: true, isDefinition: true)
!2 = distinct !DISubprogram(name: "test", scope: !3, file: !3, line: 4, type: !4, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !8)
!3 = !DIFile(filename: "test.cl", directory: "")
!4 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !5)
!5 = !{null}
!6 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !7, splitDebugInlining: false, nameTableKind: None)
!7 = !{!0}
!8 = !{}
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{i32 7, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{void ()* @test}
!13 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 1, type: !14, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !8)
!14 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !5)
!15 = !DILocation(line: 2, column: 1, scope: !13)
!16 = !DILocation(line: 6, column: 5, scope: !2)

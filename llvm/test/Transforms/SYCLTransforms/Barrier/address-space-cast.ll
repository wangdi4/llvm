; This test checks that debug instrinsic of addrspacecast of an alloca is used
; to create DbgDeclare for new alloca.

; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test() #0 !dbg !7 {
entry:

; CHECK: %a.addr = alloca ptr, align 8
; CHECK: call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !{{[0-9]+}}, metadata !DIExpression(DW_OP_deref)), !dbg

  call void @dummy_barrier.()
  %a = alloca i32, align 4
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  call void @llvm.dbg.declare(metadata ptr addrspace(4) %a.ascast, metadata !12, metadata !DIExpression()), !dbg !13
  br label %"Barrier BB", !dbg !14

"Barrier BB":                                     ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1), !dbg !14
  ret void, !dbg !14
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #2

attributes #0 = { convergent noinline norecurse nounwind optnone }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { convergent }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{!"-cl-std=CL2.0", !"-cl-opt-disable", !"-g"}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!7 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !9)
!9 = !{null, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64, dwarfAddressSpace: 1)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "dst", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!13 = !DILocation(line: 1, column: 30, scope: !7)
!14 = !DILocation(line: 3, column: 1, scope: !7)

; DEBUGIFY-NOT: WARNING

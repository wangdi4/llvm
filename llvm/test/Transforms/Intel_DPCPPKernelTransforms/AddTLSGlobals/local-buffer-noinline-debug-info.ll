; This test checks that debug info of saving/restoring pLocalMemBase should be
; the same as the call instruction.

; RUN: opt -dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-tls-globals %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local void @foo() #0 !dbg !6 {
entry:

; CHECK-LABEL: foo
; CHECK: [[LI:%[0-9]+]] = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8, !dbg [[DIL:![0-9]+]]
; CHECK-NEXT: %pLocalMem_bar = getelementptr i8, i8 addrspace(3)* [[LI]], i32 0, !dbg [[DIL]]
; CHECK-NEXT: store i8 addrspace(3)* %pLocalMem_bar, i8 addrspace(3)** @pLocalMemBase, align 8, !dbg [[DIL]]
; CHECK-NEXT: call void @bar(), !dbg [[DIL]]
; CHECK-NEXT: store i8 addrspace(3)* [[LI]], i8 addrspace(3)** @pLocalMemBase, align 8, !dbg [[DIL]]

  call void @bar(), !dbg !10
  ret void, !dbg !11
}

define dso_local void @bar() #0 !dbg !12 {
entry:
  ret void, !dbg !13
}

attributes #0 = { convergent noinline norecurse nounwind }

!llvm.dbg.cu = !{!0, !3}
!llvm.module.flags = !{!4, !5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cl", directory: "")
!2 = !{}
!3 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!4 = !{i32 7, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 2, type: !8, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!7 = !DIFile(filename: "test.cl", directory: "")
!8 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !9)
!9 = !{null}
!10 = !DILocation(line: 3, column: 3, scope: !6)
!11 = !DILocation(line: 4, column: 1, scope: !6)
!12 = distinct !DISubprogram(name: "bar", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !3, retainedNodes: !2)
!13 = !DILocation(line: 1, column: 13, scope: !12)

; DEBUGIFY-NOT: WARNING

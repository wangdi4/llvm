; Inline report
; RUN: opt -passes='cgscc(inline)' -inline-report=0x7 < %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -passes='cgscc(inline)' -inline-report=0x207 < %s -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-INTRIN
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0x286 < %s -S | opt -passes='cgscc(inline)' -inline-report=0x286 -S | opt -passes='inlinereportemitter' -inline-report=0x286 -disable-output 2>&1 | FileCheck %s --check-prefix=CHECK-INTRIN

; This test checks that setting DontSkipIntrin bit in the -inline-report option
; forces including the llvm.dbg.value call site info into inline report.

; CHECK-NOT: llvm.dbg.value{{.*}}Callee is intrinsic
; CHECK-INTRIN: llvm.dbg.value{{.*}}Callee is intrinsic

@N = common dso_local local_unnamed_addr global i32 0, align 4, !dbg !0

define dso_local i32 @foo(i32 %i) local_unnamed_addr !dbg !12 {
entry:
  call void @llvm.dbg.value(metadata i32 %i, metadata !16, metadata !DIExpression()), !dbg !17
  %add = add nsw i32 %i, 1, !dbg !18
  ret i32 %add, !dbg !19
}

define dso_local i32 @main() local_unnamed_addr !dbg !20 {
entry:
  %0 = load i32, i32* @N, align 4, !dbg !23, !tbaa !24
  %call = call i32 @foo(i32 %0), !dbg !28
  ret i32 %call, !dbg !29
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata)

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!7, !8, !9}
!llvm.dbg.intel.emit_class_debug_always = !{!10}
!llvm.ident = !{!11}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "N", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based icx (ICX) dev.8.x.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "test7.c", directory: "/export/iusers/ochupina/inl_report")
!4 = !{}
!5 = !{!0}
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{!"true"}
!11 = !{!"icx (ICX) dev.8.x.0"}
!12 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 3, type: !13, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !15)
!13 = !DISubroutineType(types: !14)
!14 = !{!6, !6}
!15 = !{!16}
!16 = !DILocalVariable(name: "i", arg: 1, scope: !12, file: !3, line: 3, type: !6)
!17 = !DILocation(line: 0, scope: !12)
!18 = !DILocation(line: 4, column: 11, scope: !12)
!19 = !DILocation(line: 4, column: 3, scope: !12)
!20 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 7, type: !21, scopeLine: 7, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !4)
!21 = !DISubroutineType(types: !22)
!22 = !{!6}
!23 = !DILocation(line: 8, column: 14, scope: !20)
!24 = !{!25, !25, i64 0}
!25 = !{!"int", !26, i64 0}
!26 = !{!"omnipotent char", !27, i64 0}
!27 = !{!"Simple C/C++ TBAA"}
!28 = !DILocation(line: 8, column: 10, scope: !20)
!29 = !DILocation(line: 8, column: 3, scope: !20)


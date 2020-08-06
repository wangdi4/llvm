; Check a function without prologue doesn't cause crash.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s --filetype=obj -o %t1

; To regenerate the test file traceback-noprologue.ll
; clang -traceback -S -emit-llvm traceback-noprologue.c

@i = dso_local global i32 0, align 4

define dso_local void @foo() !dbg !7 {
entry:
  %0 = load i32, i32* @i, align 4, !dbg !9
  %inc = add nsw i32 %0, 1, !dbg !9
  store i32 %inc, i32* @i, align 4, !dbg !9
  ret void, !dbg !10
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-noprologue.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 3, type: !8, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 4, column: 3, scope: !7)
!10 = !DILocation(line: 5, column: 1, scope: !7)

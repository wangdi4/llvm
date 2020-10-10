; Check traceback can work along with -function-sections.

; Check the program does not crash when emitting obj file.
; RUN: llc --filetype=obj -function-sections -O0 -mtriple x86_64-linux-gnu %s -o %t1

; Check a separate module is generated for each subsection.
; RUN: llc -function-sections -O0 -mtriple x86_64-linux-gnu %s -o %t2
; RUN: FileCheck < %t2 %s

; CHECK-COUNT-1:  .section    .trace,"a",@progbits
; CHECK-COUNT-3:  .byte    10                      # TB_TAG_Module

; To regenerate the test file traceback-function-sections.ll
; clang -traceback -S -emit-llvm traceback-function-sections.c

define dso_local i32 @f() !dbg !7 {
entry:
  ret i32 0, !dbg !9
}

define dso_local i32 @g() !dbg !10 {
entry:
  ret i32 0, !dbg !11
}

define dso_local i32 @main() !dbg !12 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 @f(), !dbg !13
  %call1 = call i32 @g(), !dbg !14
  %add = add nsw i32 %call, %call1, !dbg !15
  ret i32 %add, !dbg !16
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-function-sections.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 1, column: 11, scope: !7)
!10 = distinct !DISubprogram(name: "g", scope: !1, file: !1, line: 2, type: !8, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!11 = !DILocation(line: 2, column: 11, scope: !10)
!12 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !8, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!13 = !DILocation(line: 3, column: 21, scope: !12)
!14 = !DILocation(line: 3, column: 27, scope: !12)
!15 = !DILocation(line: 3, column: 25, scope: !12)
!16 = !DILocation(line: 3, column: 14, scope: !12)

; Check the contents of section .trace with optimal encodings.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s --filetype=obj -o %t1.o
; RUN: llvm-objdump --full-contents --section=.trace  %t1.o | FileCheck --check-prefixes=CONTENT %s
; CONTENT-LABEL: Contents of section .trace:
; CONTENT-NEXT: 0000 0a020000 43000000 00000000 00000000  ....C...........
; CONTENT-NEXT: 0010 01000000 0b000000 00001100 74726163  ............trac
; CONTENT-NEXT: 0020 65626163 6b2d6261 7369632e 63000000  eback-basic.c...
; CONTENT-NEXT: 0030 0c000400 00000000 00000000 6d61696e  ............main
; CONTENT-NEXT: 0040 89c000                               ...

; To regenerate the test file traceback-basic.ll
; clang -traceback -S -emit-llvm traceback-basic.c

define dso_local i32 @main() !dbg !7 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  ret i32 0, !dbg !9
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-basic.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 1, column: 14, scope: !7)

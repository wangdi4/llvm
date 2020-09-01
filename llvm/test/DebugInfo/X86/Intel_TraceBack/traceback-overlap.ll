; Check the condition when the definitions of functions from different files are overlapped.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s --check-prefixes=HEADER
; HEADER:         .long    3                                # TB_AT_NumOfFiles

; HEADER:         .short    19                              # TB_AT_NameLength
; HEADER-NEXT:    .ascii    "traceback-header1.h"           # TB_AT_FileName
; HEADER-NEXT:    .short    19                              # TB_AT_NameLength
; HEADER-NEXT:    .ascii    "traceback-header2.h"           # TB_AT_FileName
; HEADER-NEXT:    .short    19                              # TB_AT_NameLength
; HEADER-NEXT:    .ascii    "traceback-headers.c"           # TB_AT_FileName

; HEADER:         .long    1                                # TB_AT_FileIdx
; HEADER:         .long    0                                # TB_AT_FileIdx
; HEADER:         .long    2                                # TB_AT_FileIdx

; To regenerate the test file traceback-headers.ll
; clang -traceback -target x86_64-linux-gnu -emit-llvm -S traceback-headers.c
; Then change the order of the definitions of functions mannually.

define dso_local i32 @header1_f1() !dbg !7 {
entry:
  ret i32 1, !dbg !10
}

define dso_local i32 @header2_f1() !dbg !13 {
entry:
  ret i32 3, !dbg !15
}

define dso_local i32 @header1_f2() !dbg !11 {
entry:
  ret i32 2, !dbg !12
}

define dso_local i32 @main() !dbg !16 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %call = call i32 @header1_f1(), !dbg !18
  store i32 %call, i32* %x, align 4, !dbg !19
  %call1 = call i32 @header2_f1(), !dbg !20
  %0 = load i32, i32* %x, align 4, !dbg !21
  %add = add nsw i32 %0, %call1, !dbg !21
  store i32 %add, i32* %x, align 4, !dbg !21
  %call2 = call i32 @header1_f2(), !dbg !22
  %1 = load i32, i32* %x, align 4, !dbg !23
  %sub = sub nsw i32 %1, %call2, !dbg !23
  store i32 %sub, i32* %x, align 4, !dbg !23
  %2 = load i32, i32* %x, align 4, !dbg !24
  ret i32 %2, !dbg !25
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-headers.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "header1_f1", scope: !8, file: !8, line: 1, type: !9, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DIFile(filename: "traceback-header1.h", directory: "/temp")
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 1, column: 20, scope: !7)
!11 = distinct !DISubprogram(name: "header1_f2", scope: !8, file: !8, line: 3, type: !9, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!12 = !DILocation(line: 3, column: 20, scope: !11)
!13 = distinct !DISubprogram(name: "header2_f1", scope: !14, file: !14, line: 1, type: !9, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!14 = !DIFile(filename: "traceback-header2.h", directory: "/temp")
!15 = !DILocation(line: 1, column: 20, scope: !13)
!16 = distinct !DISubprogram(name: "main", scope: !17, file: !17, line: 4, type: !9, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!17 = !DIFile(filename: "traceback-headers.c", directory: "/temp")
!18 = !DILocation(line: 5, column: 11, scope: !16)
!19 = !DILocation(line: 5, column: 7, scope: !16)
!20 = !DILocation(line: 6, column: 8, scope: !16)
!21 = !DILocation(line: 6, column: 5, scope: !16)
!22 = !DILocation(line: 7, column: 8, scope: !16)
!23 = !DILocation(line: 7, column: 5, scope: !16)
!24 = !DILocation(line: 8, column: 10, scope: !16)
!25 = !DILocation(line: 8, column: 3, scope: !16)

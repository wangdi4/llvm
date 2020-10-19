; Check we are able to emit the DWARF section and .trace section when flags
; "TraceBack","Dwarf Version" exist at same time.

; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s --check-prefixes=DWARF
; DWARF: .section   .debug_{{.*}}

; Check we add an initial line record for main only (not for foo) when -g is enabled.

; RUN: FileCheck < %t1 %s --check-prefixes=RECORD
; RECORD-LABEL:   .ascii    "foo"                          # TB_AT_RoutineName
; RECORD:         .byte    4                               # TB_TAG_LN1
; RECORD-NEXT:    .byte    4                               # TB_AT_LN1
; RECORD-NEXT:    .byte    9                               # TB_TAG_PC4
; RECORD-NEXT:    .long    (.L{{.*}}-.L{{.*}})-1           # TB_AT_PC4

; RECORD-LABEL:    .ascii    "main"                        # TB_AT_RoutineName
; RECORD:         .byte    4                               # TB_TAG_LN1
; RECORD-NEXT:    .byte    2                               # TB_AT_LN1
; RECORD-NEXT:    .byte    9                               # TB_TAG_PC4
; RECORD-NEXT:    .long    (.L{{.*}}-main)-1               # TB_AT_PC4

; To regenerate the test file traceback-debug.ll
; clang -traceback -g -S -emit-llvm traceback-debug.c -o traceback-debug.ll

@i = dso_local global i32 0, align 4, !dbg !0

define dso_local void @foo() !dbg !13 {
entry:
  %0 = load i32, i32* @i, align 4, !dbg !16
  %inc = add nsw i32 %0, 1, !dbg !16
  store i32 %inc, i32* @i, align 4, !dbg !16
  ret void, !dbg !17
}

define dso_local i32 @main() !dbg !18 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load i32, i32* @i, align 4, !dbg !21
  %inc = add nsw i32 %0, 1, !dbg !21
  store i32 %inc, i32* @i, align 4, !dbg !21
  ret i32 0, !dbg !22
}

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!8, !9, !10, !11}
!llvm.ident = !{!12}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "i", scope: !2, file: !6, line: 1, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "traceback-debug.c", directory: "/temp")
!4 = !{}
!5 = !{!0}
!6 = !DIFile(filename: "traceback-debug.c", directory: "/temp")
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{i32 7, !"Dwarf Version", i32 4}
!9 = !{i32 2, !"TraceBack", i32 1}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{!"clang version 11.0.0"}
!13 = distinct !DISubprogram(name: "foo", scope: !6, file: !6, line: 3, type: !14, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!14 = !DISubroutineType(types: !15)
!15 = !{null}
!16 = !DILocation(line: 4, column: 3, scope: !13)
!17 = !DILocation(line: 5, column: 1, scope: !13)
!18 = distinct !DISubprogram(name: "main", scope: !6, file: !6, line: 7, type: !19, scopeLine: 7, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!19 = !DISubroutineType(types: !20)
!20 = !{!7}
!21 = !DILocation(line: 8, column: 3, scope: !18)
!22 = !DILocation(line: 9, column: 3, scope: !18)

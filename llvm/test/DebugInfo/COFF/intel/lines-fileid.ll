; RUN: llc < %s \
; RUN:   | FileCheck %s --check-prefix=ASM
; RUN: llc < %s -filetype=obj | llvm-readobj --codeview - \
; RUN:   | FileCheck %s --check-prefix=OBJ
;
; Validate the source location for routine bar() has a non-zero file
; identifier. A bug caused a zero file id in the asm case and a crash in
; the object case.
;
; ASM-LABEL: foo:
; ASM: 	     .cv_func_id 0
; ASM: 	     .cv_file	1 "/path/to/test.cpp"
; ASM: 	     .cv_loc	0 1 30 0
; ASM: 	     retq
; ASM-LABEL: bar:
; ASM: 	     .cv_func_id 1
; ASM: 	     .cv_loc	1 1 25 0
; ASM: 	     #MEMBARRIER
; ASM: 	     retq
;
; OBJ:  FunctionLineTable [
; OBJ:    LinkageName: foo
; OBJ:    FilenameSegment [
; OBJ:      Filename: /path/to/test.cpp (0x0)
; OBJ:      +0x0 [
; OBJ:        LineNumberStart: 30
; OBJ:        LineNumberEndDelta: 0
; OBJ:        IsStatement: No
; OBJ:      ]
; OBJ:    ]
; OBJ:  ]
; OBJ:  FunctionLineTable [
; OBJ:    LinkageName: bar
; OBJ:    FilenameSegment [
; OBJ:      Filename: /path/to/test.cpp (0x0)
; OBJ:      +0x0 [
; OBJ:        LineNumberStart: 25
; OBJ:        LineNumberEndDelta: 0
; OBJ:        IsStatement: No
; OBJ:      ]
; OBJ:    ]
; OBJ:  ]
;
; ModuleID = 'test.ll'
source_filename = "test.cpp"
target triple = "x86_64-pc-windows-msvc"

; Function Attrs: nounwind uwtable
define void @foo() #0 !dbg !4 {
  ret void, !dbg !7
}

; Function Attrs: nounwind uwtable
define void @bar() #0 !dbg !9 {
  fence acq_rel, !dbg !10
  ret void
}

attributes #0 = { nounwind uwtable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test.cpp", directory: "/path/to")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 2, !"CodeView", i32 1}
!4 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 15, type: !5, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{}
!7 = !DILocation(line: 30, scope: !8)
!8 = distinct !DILexicalBlock(scope: !4, file: !1, line: 21)
!9 = distinct !DISubprogram(name: "bar", scope: !1, file: !1, line: 21, type: !5, scopeLine: 21, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !0)
!10 = !DILocation(line: 25, scope: !11)
!11 = distinct !DILexicalBlock(scope: !9, file: !1, line: 25)

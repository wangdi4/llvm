; RUN: opt -S -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.MyStruct -o %t %s
; RUN:   grep "DILocalVariable(name: \"x\"" %t | count 1
; RUN:   grep "DILocalVariable(name: \"y\"" %t | count 1
; RUN:   grep "DILocalVariable(name: \"z\"" %t | count 1

; This test is to verify that when the DTrans optimization base class clones
; a function containing debug variables in different lexical blocks that it
; does not result in duplicate DILocalVariables containing the same name.

%struct.MyStruct = type { i32, i32, i32 }

declare void @llvm.dbg.addr(metadata, metadata, metadata)
declare void @llvm.dbg.value(metadata, metadata, metadata)

define void @test01caller(i32 %x, i32 %y, i32 %z, %struct.MyStruct* %s) !dbg !7 {
entry:
  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %x.addr, metadata !13, metadata !DIExpression()), !dbg !22
  br label %block1

block1:                                           ; preds = %entry
  ; Test with variable in a different lexical block
  %y.addr = alloca i32, align 4
  store i32 %x, i32* %y.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %y.addr, metadata !18, metadata !DIExpression()), !dbg !24
  br label %block2

block2:                                           ; preds = %block1
  ; Test with variable in a different lexical block
  %z.addr = alloca i32, align 4
  store i32 %x, i32* %z.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %z.addr, metadata !20, metadata !DIExpression()), !dbg !25
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test.c", directory: "C:\5Ctemp")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 2}
!6 = !{!"clang version 6.0.0 "}
!7 = distinct !DISubprogram(name: "test01caller", scope: !1, file: !1, line: 100, type: !8, isLocal: false, isDefinition: true, scopeLine: 100, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10, !10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "MyStruct", scope: !0, file: !1, size: 24, elements: !12, runtimeLang: DW_LANG_C89, identifier: "MyStruct")
!12 = !{!10, !10, !10}
!13 = !DILocalVariable(name: "x", arg: 1, scope: !14, file: !1, line: 3, type: !10)
!14 = distinct !DISubprogram(name: "test01callee", scope: !1, file: !1, line: 3, type: !15, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !17)
!15 = !DISubroutineType(types: !16)
!16 = !{null, null}
!17 = !{!13, !18, !20}
!18 = !DILocalVariable(name: "y", arg: 2, scope: !19, file: !1, line: 3, type: !10)
!19 = distinct !DILexicalBlock(scope: !14, file: !1, line: 4)
!20 = !DILocalVariable(name: "z", arg: 3, scope: !21, file: !1, line: 3, type: !10)
!21 = distinct !DILexicalBlock(scope: !14, file: !1, line: 5)
!22 = !DILocation(line: 3, column: 12, scope: !14, inlinedAt: !23)
!23 = distinct !DILocation(line: 102, column: 9, scope: !7)
!24 = !DILocation(line: 4, column: 3, scope: !19, inlinedAt: !23)
!25 = !DILocation(line: 5, column: 5, scope: !21, inlinedAt: !23)

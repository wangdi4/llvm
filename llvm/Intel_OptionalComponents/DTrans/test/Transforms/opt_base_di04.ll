; RUN: opt -S -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.MyStruct -o %t %s
; RUN:   grep "DILocalVariable(name: \"x\"" %t | count 1
; RUN:   grep "DILocalVariable(name: \"y\"" %t | count 1
; RUN:   grep "DILocalVariable(name: \"z\"" %t | count 1

; This test is to verify that when the DTrans optimization base class remaps
; a function containing debug variables in different lexical blocks that it
; does not result in duplicate DILocalVariables containing the same name.


declare void @llvm.dbg.addr(metadata, metadata, metadata) #0
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

%struct.MyStruct = type { i32, i32, i32 }

define void @test01(i32 %x, i32 %y, i32 %z) !dbg !7 {
entry:
  %tmp = alloca %struct.MyStruct

  %x.addr = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %x.addr, metadata !12, metadata !DIExpression()), !dbg !17
  br label %block1

block1:                                           ; preds = %entry
  %y.addr = alloca i32, align 4
  store i32 %x, i32* %y.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %y.addr, metadata !13, metadata !DIExpression()), !dbg !18
  br label %block2

block2:                                           ; preds = %block1
  %z.addr = alloca i32, align 4
  store i32 %x, i32* %z.addr, align 4
  call void @llvm.dbg.addr(metadata i32* %z.addr, metadata !15, metadata !DIExpression()), !dbg !19
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
!7 = distinct !DISubprogram(name: "test01", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !11)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10, !10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12, !13, !15}
!12 = !DILocalVariable(name: "x", arg: 1, scope: !7, file: !1, line: 3, type: !10)
!13 = !DILocalVariable(name: "y", arg: 2, scope: !14, file: !1, line: 3, type: !10)
!14 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4)
!15 = !DILocalVariable(name: "z", arg: 3, scope: !16, file: !1, line: 3, type: !10)
!16 = distinct !DILexicalBlock(scope: !7, file: !1, line: 5)
!17 = !DILocation(line: 3, column: 12, scope: !7)
!18 = !DILocation(line: 4, column: 3, scope: !14)
!19 = !DILocation(line: 5, column: 5, scope: !16)

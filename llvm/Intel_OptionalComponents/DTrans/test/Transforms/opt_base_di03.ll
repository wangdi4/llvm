; This test is to verify that the DTrans base class cloning/remapping of
; functions does not cause the debug information to fail the module verifier.
; In this test the function that originally contained the local variable
; has been optimized away, but has left inlined instances within two functions.

; Old pass manager
; RUN: sed -e s/^.DO_CLONE1:// %s | sed s/^.DO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.NO_CLONE1:// %s | sed s/^.DO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.DO_CLONE1:// %s | sed s/^.NO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.NO_CLONE1:// %s | sed s/^.NO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; New pass manager
; RUN: sed -e s/^.DO_CLONE1:// %s | sed s/^.DO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.NO_CLONE1:// %s | sed s/^.DO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.DO_CLONE1:// %s | sed s/^.NO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

; RUN: sed -e s/^.NO_CLONE1:// %s | sed s/^.NO_CLONE2:// | \
; RUN:     opt -disable-output -whole-program-assume -passes=dtrans-optbasetest -dtrans-optbasetest-typelist=struct.node

%struct.network = type { i64, %struct.node*, %struct.arc* }
%struct.node = type { i64 }
%struct.arc = type { i64 }

@net = internal global %struct.network zeroinitializer, align 8, !dbg !0

declare void @llvm.dbg.value(metadata, metadata, metadata)
declare void @llvm.dbg.declare(metadata, metadata, metadata)

;DO_CLONE1: define internal i64 @suspend_impl(%struct.network* %t0) !dbg !35 {
;NO_CLONE1: define internal i64 @suspend_impl() !dbg !35 {
  ; Debug info for local variable
  call void @llvm.dbg.value(metadata %struct.network* @net, metadata !39, metadata !DIExpression()), !dbg !40

  ; Debug info for inlined variable.
  call void @llvm.dbg.value(metadata %struct.network* @net, metadata !33, metadata !DIExpression()), !dbg !41
  ret i64 undef
}

;DO_CLONE2: define internal i64 @price_out(%struct.network*) !dbg !45 {
;NO_CLONE2: define internal i64 @price_out() !dbg !45 {
  ; Debug info for inlined variable.
  call void @llvm.dbg.value(metadata %struct.network* @net, metadata !33, metadata !DIExpression()), !dbg !50
  ret i64 undef
}

; These flags are necessary in order for debug to be processed.
!llvm.module.flags = !{!19, !20, !21, !22}

; Description of global variable 'net'.
!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "net", scope: !2, file: !3, line: 30, type: !9, isLocal: false, isDefinition: true)

; Source file information.
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 7.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4, globals: !8)
!3 = !DIFile(filename: "mcf.c", directory: "build_base_hsw.0000")
!4 = !{!5}

!5 = !DIDerivedType(tag: DW_TAG_typedef, name: "int64_t", file: !6, line: 197, baseType: !7)
!6 = !DIFile(filename: "/usr/include/sys/types.h", directory: "build_base_hsw.0000")
!7 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)

; Global variable list
!8 = !{!0}

; Description of network structure.
!9 = !DIDerivedType(tag: DW_TAG_typedef, name: "network_t", file: !10, line: 206, baseType: !11)
!10 = !DIFile(filename: "defines.h", directory: "build_base_hsw.0000")
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "network", file: !10, line: 181, size: 8, elements: !12)
!12 = !{!13, !14, !18}
!13 = !DIDerivedType(tag: DW_TAG_member, name: "n", scope: !11, file: !10, line: 185, baseType: !5, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_member, name: "nodes", scope: !11, file: !10, line: 200, baseType: !15, size: 64, offset: 8)

; Description of node structure.
!15 = !DIDerivedType(tag: DW_TAG_typedef, name: "node_p", file: !10, line: 135, baseType: !16)
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 8)
!17 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "node", file: !10, line: 149, size: 832, elements: !4)
!18 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "arc", file: !10, line: 168, size: 576, elements: !4)

; Referenced from module flags to cause opt to process debug info.
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"wchar_size", i32 4}
!22 = !{i32 1, !"ThinLTO", i32 0}

; Debug information for routine refreshPositions.
!23 = distinct !DISubprogram(name: "refreshPositions", scope: !24, file: !24, line: 77, type: !25, isLocal: false, isDefinition: true, scopeLine: 84, flags: DIFlagPrototyped, isOptimized: true, unit: !31, retainedNodes: !32)
!24 = !DIFile(filename: "implicit.c", directory: "build_base_hsw.0000")
!25 = !DISubroutineType(types: !26)
!26 = !{null, !27}
!27 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !28, size: 64)
!28 = !DIDerivedType(tag: DW_TAG_typedef, name: "network_t", file: !10, line: 206, baseType: !29)
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "network", file: !10, line: 181, size: 8, elements: !30)
!30 = !{!13}
!31 = distinct !DICompileUnit(language: DW_LANG_C89, file: !24, producer: "clang version 7.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, retainedTypes: !4)
!32 = !{!33}

; This variable gets inlined, causing references from multiple routines
; to reference it.
!33 = !DILocalVariable(name: "net", arg: 1, scope: !23, file: !24, line: 77, type: !27)
!34 = !DILocation(line: 77, column: 35, scope: !23)

; Debug information for routine suspend_impl
!35 = distinct !DISubprogram(name: "suspend_impl", scope: !24, file: !24, line: 787, type: !36, isLocal: false, isDefinition: true, scopeLine: 794, flags: DIFlagPrototyped, isOptimized: true, unit: !31, retainedNodes: !38)
!36 = !DISubroutineType(types: !37)
;DO_CLONE1:!37 = !{null, !27}
;NO_CLONE1:!37 = !{null}
!38 = !{!39}
!39 = !DILocalVariable(name: "net", arg: 1, scope: !35, file: !24, line: 77, type: !27)
!40 = !DILocation(line: 787, column: 31, scope: !35)


; Information about the source line location inside of suspend_impl
; that is from an inlined call to refreshPositions
!41 = !DILocation(line: 77, column: 35, scope: !23, inlinedAt: !42)
!42 = distinct !DILocation(line: 866, column: 9, scope: !43)
!43 = distinct !DILexicalBlock(scope: !44, file: !24, line: 852, column: 5)
!44 = distinct !DILexicalBlock(scope: !35, file: !24, line: 851, column: 9)

!45 = distinct !DISubprogram(name: "price_out_impl", scope: !24, file: !24, line: 444, type: !48, isLocal: false, isDefinition: true, scopeLine: 449, flags: DIFlagPrototyped, isOptimized: true, unit: !31, retainedNodes: !46)
!46 = !{!47}
!47 = !DILocalVariable(name: "net", arg: 1, scope: !45, file: !24, line: 444, type: !27)
!48 = !DISubroutineType(types: !49)
;DO_CLONE2:!49 = !{null, !27}
;NO_CLONE2:!49 = !{null}

!50 = !DILocation(line: 77, column: 35, scope: !23, inlinedAt: !51)
!51 = distinct !DILocation(line: 757, column: 9, scope: !45)

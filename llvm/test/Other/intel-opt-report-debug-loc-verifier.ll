; RUN: opt -S -verify < %s | FileCheck %s

; Check that embedded DILocations are allowed in "intel.optreport.debug_location".

; CHECK-LABEL: @foo

define void @foo() {
  ret void, !llvm.loop.optreport !7
}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "t.c", directory: "/path/to/test/Transforms/ADCE")
!2 = !{}
!4 = distinct !DISubprogram(name: "variable_in_unused_subscope", scope: !1, file: !1, line: 3, type: !5, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
!5 = !DISubroutineType(types: !6)
!6 = !{null}
!7 = !{!"intel.optreport.debug_location", !8}
!8 = !DILocation(line: 14, column: 9, scope: !4, inlinedAt: !9)
!9 = distinct !DILocation(line: 19, column: 5, scope: !4)

; RUN: opt -S -skip-partial-inlining-cost-analysis -passes=partial-inliner < %s | FileCheck %s
;
; During partial inlining, ensure nested global types are not duplicated.
;
; CHECK:      !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME:   name: "inner",
; CHECK-SAME:   scope: [[OUTER:![0-9]+]]
; CHECK-NOT:  !DICompositeType(tag: DW_TAG_structure_type, name: "inner"
; CHECK:      [[OUTER]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME:   name: "outer"
; CHECK-NOT:  !DICompositeType(tag: DW_TAG_structure_type, name: "inner"

declare void @llvm.dbg.value(metadata, metadata, metadata) #0

define internal i32 @callee(i1 %param1, i32* align 4 %param2) !dbg !4 {
entry:
  br i1 %param1, label %if.then, label %return

if.then:                                          ; preds = %entry
  call void @llvm.dbg.value(metadata i32* %param2, metadata !17, metadata !DIExpression()), !dbg !18
  store i32 42, i32* %param2, align 4
  br label %return

return:                                           ; preds = %if.then, %entry
  ret i32 0
}

define internal i32 @caller(i1 %param1, i32* align 2 %param2) !dbg !19 {
entry:
  %val = call i32 @callee(i1 %param1, i32* %param2), !dbg !20
  ret i32 %val
}

attributes #0 = { nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3}

!0 = distinct !DICompileUnit(language: DW_LANG_C, file: !1, producer: "handwritten", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "<stdin>", directory: "/")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = distinct !DISubprogram(name: "callee", scope: !1, file: !1, line: 1, type: !5, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!5 = !DISubroutineType(types: !6)
!6 = !{!7, !8, !9}
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !DIBasicType(name: "int", size: 8, encoding: DW_ATE_signed)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "inner", scope: !11, file: !1, line: 91, size: 32, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !15, identifier: "_inner")
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "outer", scope: !12, file: !1, line: 84, size: 32, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !13, identifier: "_outer")
!12 = !DINamespace(name: "namespace_1", scope: null)
!13 = !{!14}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "member_1", scope: !11, file: !1, line: 93, baseType: !9, size: 32)
!15 = !{!16}
!16 = !DIDerivedType(tag: DW_TAG_member, name: "member_2", scope: !10, file: !1, line: 93, baseType: !7, size: 32)
!17 = !DILocalVariable(name: "var", arg: 2, scope: !4, type: !9)
!18 = !DILocation(line: 5, column: 1, scope: !4)
!19 = distinct !DISubprogram(name: "caller", scope: !1, file: !1, line: 2, type: !5, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!20 = !DILocation(line: 6, column: 1, scope: !19)

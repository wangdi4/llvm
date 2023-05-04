; RUN: llvm-as %s -o %t.o
; RUN: llvm-lto -O3 -exported-symbol=main %t.o

; This is a regression test for a problem described in CMPLRLLVM-348.
;
; The problem occurs when the Promote 'by reference' arguments to scalars
; pass runs, but is unable to delete an old function declaration due to
; it still be referenced within the SCC graph. This results in invalid
; debug metadata being referenced in the new function body produced by
; the pass. Because of the need for several optimization passes to
; run, while maintaining the SCC graph, this test needed to be run
; with 'llvm-lto', rather than with 'opt'. Success or failure is
; determined by the successful completion of the command, and not based
; on any specific IR or asm being generated.
;
; In this case, the following sequence of events leads to a problem:
; 1) The Indirect Call Conversion (indirectcallconv) pass will identify
;    potential targets for an indirect call, and produce direct calls for
;    the indirect call in 'main'
; 2) Function inlining will inline the direct calls from step 1.
; 3) The instruction combiner (instcombine) pass will remove the 'select'
;    statement from 'main' which references the function addresses. (This step
;    is critical in enabling the argument promotion (argpromotion) pass to
;    process the function because there will no longer be any indirect
;    references to the function, but the functions will still exist in the
;    SCC graph).
; 4) The argument promotion pass processes the 'do_add' and 'do_sub' functions,
;    converting the pointer arguments to by-value arguments.
;
; Note: the order of the definitions is important because the instcombine
; step must be performed on 'main' before the argument promotion pass is
; run on 'do_add'


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { i32, i32 }

define dso_local i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr !dbg !13 {
entry:
  %s.i = alloca %struct.S, align 4
  %0 = bitcast ptr %s.i to ptr, !dbg !22
  %x.i = getelementptr inbounds %struct.S, ptr %s.i, i64 0, i32 0, !dbg !36
  store i32 2, ptr %x.i, align 4, !dbg !37, !tbaa !38
  %y.i = getelementptr inbounds %struct.S, ptr %s.i, i64 0, i32 1, !dbg !43
  store i32 3, ptr %y.i, align 4, !dbg !44, !tbaa !45
  %tobool.i.i = icmp eq i32 %argc, 0, !dbg !46

  ; Instruction will be replaced by instcombine to not reference address of
  ; functions, but the function will still be in the SCC, although there will
  ;  be no more calls to it.
  %cond.i.i = select i1 %tobool.i.i, ptr @do_sub, ptr @do_add, !dbg !46

  ; Call will be converted to direct calls based on possible targets, and then
  ;  inlined.
  %call.i.i = call i32 %cond.i.i(ptr nonnull %s.i), !dbg !60, !callees !61
  ret i32 %call.i.i, !dbg !62
}

; This function can have the pointer parameter converted to a by-value
; parameter.
define internal i32 @do_add(ptr nocapture readonly %s) unnamed_addr !dbg !63 {
entry:

  ; The debug information is pointing to an argument that is going to be replaced
  call void @llvm.dbg.value(metadata ptr %s, metadata !65, metadata !DIExpression()), !dbg !69

  %x = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 0, !dbg !70
  %0 = load i32, ptr %x, align 4, !dbg !70, !tbaa !38
  call void @llvm.dbg.value(metadata i32 %0, metadata !66, metadata !DIExpression()), !dbg !71
  %y = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1, !dbg !72
  %1 = load i32, ptr %y, align 4, !dbg !72, !tbaa !45
  call void @llvm.dbg.value(metadata i32 %1, metadata !67, metadata !DIExpression()), !dbg !73
  %add = add nsw i32 %1, %0, !dbg !74
  call void @llvm.dbg.value(metadata i32 %add, metadata !68, metadata !DIExpression()), !dbg !75
  ret i32 %add, !dbg !76
}

; This function can have the pointer parameter converted to a by-value
; parameter.
define internal i32 @do_sub(ptr nocapture readonly %s) unnamed_addr !dbg !77 {
entry:

  ; The debug information is pointing to an argument that is going to be replaced
  call void @llvm.dbg.value(metadata ptr %s, metadata !79, metadata !DIExpression()), !dbg !83

  %x = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 0, !dbg !84
  %0 = load i32, ptr %x, align 4, !dbg !84, !tbaa !38
  call void @llvm.dbg.value(metadata i32 %0, metadata !80, metadata !DIExpression()), !dbg !85
  %y = getelementptr inbounds %struct.S, ptr %s, i64 0, i32 1, !dbg !86
  %1 = load i32, ptr %y, align 4, !dbg !86, !tbaa !45
  call void @llvm.dbg.value(metadata i32 %1, metadata !81, metadata !DIExpression()), !dbg !87
  %sub = sub nsw i32 %0, %1, !dbg !88
  call void @llvm.dbg.value(metadata i32 %sub, metadata !82, metadata !DIExpression()), !dbg !89
  ret i32 %sub, !dbg !90
}

declare void @llvm.dbg.value(metadata, metadata, metadata)
declare void @llvm.lifetime.start.p0i8(i64, ptr nocapture)
declare void @llvm.lifetime.end.p0i8(i64, ptr nocapture)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.dbg.intel.emit_class_debug_always = !{!11}
!llvm.ident = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 69eba4a4fc81c7c549a3e90a9446d8c40490a931) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm aa5c3967c3fde797f0664e749e0ff2bced76bc50)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3, nameTableKind: None)
!1 = !DIFile(filename: "byval1.c", directory: "/nfs/site/home/cmchruls/simple")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5, expr: !DIExpression())
!5 = distinct !DIGlobalVariable(name: "glob", scope: !0, file: !1, line: 8, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 1, !"ThinLTO", i32 0}
!11 = !{!"true"}
!12 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 69eba4a4fc81c7c549a3e90a9446d8c40490a931) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm aa5c3967c3fde797f0664e749e0ff2bced76bc50)"}
!13 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 38, type: !14, isLocal: false, isDefinition: true, scopeLine: 39, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !19)
!14 = !DISubroutineType(types: !15)
!15 = !{!6, !6, !16}
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!19 = !{!20, !21}
!20 = !DILocalVariable(name: "argc", arg: 1, scope: !13, file: !1, line: 38, type: !6)
!21 = !DILocalVariable(name: "argv", arg: 2, scope: !13, file: !1, line: 38, type: !16)
!22 = !DILocation(line: 31, column: 3, scope: !23, inlinedAt: !35)
!23 = distinct !DISubprogram(name: "callercaller", scope: !1, file: !1, line: 30, type: !24, isLocal: false, isDefinition: true, scopeLine: 30, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !26)
!24 = !DISubroutineType(types: !25)
!25 = !{!6, !6}
!26 = !{!27, !28, !34}
!27 = !DILocalVariable(name: "z", arg: 1, scope: !23, file: !1, line: 30, type: !6)
!28 = !DILocalVariable(name: "s", scope: !23, file: !1, line: 31, type: !29)
!29 = !DIDerivedType(tag: DW_TAG_typedef, name: "S", file: !1, line: 6, baseType: !30)
!30 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S", file: !1, line: 3, size: 64, elements: !31)
!31 = !{!32, !33}
!32 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !30, file: !1, line: 4, baseType: !6, size: 32)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !30, file: !1, line: 5, baseType: !6, size: 32, offset: 32)
!34 = !DILocalVariable(name: "x", scope: !23, file: !1, line: 34, type: !6)
!35 = distinct !DILocation(line: 40, column: 9, scope: !13)
!36 = !DILocation(line: 32, column: 5, scope: !23, inlinedAt: !35)
!37 = !DILocation(line: 32, column: 7, scope: !23, inlinedAt: !35)
!38 = !{!39, !40, i64 0}
!39 = !{!"struct@S", !40, i64 0, !40, i64 4}
!40 = !{!"int", !41, i64 0}
!41 = !{!"omnipotent char", !42, i64 0}
!42 = !{!"Simple C/C++ TBAA"}
!43 = !DILocation(line: 33, column: 5, scope: !23, inlinedAt: !35)
!44 = !DILocation(line: 33, column: 7, scope: !23, inlinedAt: !35)
!45 = !{!39, !40, i64 4}
!46 = !DILocation(line: 25, column: 19, scope: !47, inlinedAt: !59)
!47 = distinct !DISubprogram(name: "caller", scope: !1, file: !1, line: 24, type: !48, isLocal: true, isDefinition: true, scopeLine: 24, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !51)
!48 = !DISubroutineType(types: !49)
!49 = !{!6, !50, !6}
!50 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!51 = !{!52, !53, !54, !58}
!52 = !DILocalVariable(name: "s", arg: 1, scope: !47, file: !1, line: 24, type: !50)
!53 = !DILocalVariable(name: "z", arg: 2, scope: !47, file: !1, line: 24, type: !6)
!54 = !DILocalVariable(name: "f", scope: !47, file: !1, line: 25, type: !55)
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !56, size: 64)
!56 = !DISubroutineType(types: !57)
!57 = !{!6, !50}
!58 = !DILocalVariable(name: "c", scope: !47, file: !1, line: 26, type: !6)
!59 = distinct !DILocation(line: 34, column: 11, scope: !23, inlinedAt: !35)
!60 = !DILocation(line: 26, column: 11, scope: !47, inlinedAt: !59)
!61 = !{ptr @do_sub, ptr @do_add}
!62 = !DILocation(line: 40, column: 2, scope: !13)
!63 = distinct !DISubprogram(name: "do_add", scope: !1, file: !1, line: 10, type: !56, isLocal: true, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !64)
!64 = !{!65, !66, !67, !68}
!65 = !DILocalVariable(name: "s", arg: 1, scope: !63, file: !1, line: 10, type: !50)
!66 = !DILocalVariable(name: "a", scope: !63, file: !1, line: 11, type: !6)
!67 = !DILocalVariable(name: "b", scope: !63, file: !1, line: 12, type: !6)
!68 = !DILocalVariable(name: "c", scope: !63, file: !1, line: 13, type: !6)
!69 = !DILocation(line: 10, column: 28, scope: !63)
!70 = !DILocation(line: 11, column: 14, scope: !63)
!71 = !DILocation(line: 11, column: 7, scope: !63)
!72 = !DILocation(line: 12, column: 14, scope: !63)
!73 = !DILocation(line: 12, column: 7, scope: !63)
!74 = !DILocation(line: 13, column: 13, scope: !63)
!75 = !DILocation(line: 13, column: 7, scope: !63)
!76 = !DILocation(line: 14, column: 3, scope: !63)
!77 = distinct !DISubprogram(name: "do_sub", scope: !1, file: !1, line: 17, type: !56, isLocal: true, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !78)
!78 = !{!79, !80, !81, !82}
!79 = !DILocalVariable(name: "s", arg: 1, scope: !77, file: !1, line: 17, type: !50)
!80 = !DILocalVariable(name: "a", scope: !77, file: !1, line: 18, type: !6)
!81 = !DILocalVariable(name: "b", scope: !77, file: !1, line: 19, type: !6)
!82 = !DILocalVariable(name: "c", scope: !77, file: !1, line: 20, type: !6)
!83 = !DILocation(line: 17, column: 27, scope: !77)
!84 = !DILocation(line: 18, column: 14, scope: !77)
!85 = !DILocation(line: 18, column: 7, scope: !77)
!86 = !DILocation(line: 19, column: 14, scope: !77)
!87 = !DILocation(line: 19, column: 7, scope: !77)
!88 = !DILocation(line: 20, column: 13, scope: !77)
!89 = !DILocation(line: 20, column: 7, scope: !77)
!90 = !DILocation(line: 21, column: 3, scope: !77)

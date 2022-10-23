; Inline report
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=1 < %s -disable-output 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0x80 < %s -S | opt -passes='cgscc(inline)' -inline-report=0x80 -S | opt -passes='inlinereportemitter' -inline-report=0x80 -disable-output 2>&1 | FileCheck %s

; This test checks that metadata-based inline report correctly handles
; llvm.dbg.declare intrinsic during inlining process, when it is deleted
; from the inlined basic block and moved right after the corresponding
; inlined alloca instruction.

; CHECK: Begin Inlining Report

define dso_local i32 @main(i32 %argc) !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata i32 %argc, metadata !13, metadata !DIExpression()), !dbg !14
  %call = call i32 @y(i32 %argc), !dbg !15
  %cmp = icmp sgt i32 %call, 7, !dbg !17
  br i1 %cmp, label %if.then, label %if.end, !dbg !18

if.then:                                          ; preds = %entry
  %call1 = call i32 @y(i32 200), !dbg !19
  br label %if.end, !dbg !20

if.end:                                           ; preds = %if.then, %entry
  %retval.0 = phi i32 [ %call1, %if.then ], [ 0, %entry ]
  ret i32 %retval.0, !dbg !21
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata)

; Function Attrs: nounwind uwtable
define internal i32 @y(i32 %i) !dbg !22 {
entry:
  %i.addr = alloca i32, align 4
  store i32 %i, ptr %i.addr, align 4, !tbaa !25
  call void @llvm.dbg.declare(metadata ptr %i.addr, metadata !24, metadata !DIExpression()), !dbg !29
  %0 = load i32, ptr %i.addr, align 4, !dbg !30, !tbaa !25
  %add = add nsw i32 %0, 1, !dbg !31
  ret i32 %add, !dbg !32
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) dev.8.x.0", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test5.c", directory: "/export/iusers/ochupina/inl_report")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"icx (ICX) dev.8.x.0"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !12)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !{!13}
!13 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 3, type: !11)
!14 = !DILocation(line: 0, scope: !8)
!15 = !DILocation(line: 4, column: 7, scope: !16)
!16 = distinct !DILexicalBlock(scope: !8, file: !1, line: 4, column: 7)
!17 = !DILocation(line: 4, column: 15, scope: !16)
!18 = !DILocation(line: 4, column: 7, scope: !8)
!19 = !DILocation(line: 5, column: 11, scope: !16)
!20 = !DILocation(line: 5, column: 4, scope: !16)
!21 = !DILocation(line: 6, column: 1, scope: !8)
!22 = distinct !DISubprogram(name: "y", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !23)
!23 = !{!24}
!24 = !DILocalVariable(name: "i", arg: 1, scope: !22, file: !1, line: 1, type: !11)
!25 = !{!26, !26, i64 0}
!26 = !{!"int", !27, i64 0}
!27 = !{!"omnipotent char", !28, i64 0}
!28 = !{!"Simple C/C++ TBAA"}
!29 = !DILocation(line: 1, column: 18, scope: !22)
!30 = !DILocation(line: 1, column: 30, scope: !22)
!31 = !DILocation(line: 1, column: 32, scope: !22)
!32 = !DILocation(line: 1, column: 23, scope: !22)


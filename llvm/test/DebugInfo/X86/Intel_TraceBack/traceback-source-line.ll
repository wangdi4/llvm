; Check line record is only emitted when source line changes and is not emitted
; for the optimized code that doesn't have a distinct source location.
; RUN: llc -O0 -mtriple x86_64-linux-gnu %s -o %t1
; RUN: FileCheck < %t1 %s

; CHECK:         .byte    4       # TB_TAG_LN1
; CHECK-NEXT:    .byte    4       # TB_AT_LN1
; CHECK-NEXT:    .byte    9       # TB_TAG_PC4
; CHECK-NEXT:    .long    {{.*}}  # TB_AT_PC4
; CHECK-NEXT:    .byte    4       # TB_TAG_LN1
; CHECK-NEXT:    .byte    1       # TB_AT_LN1
; CHECK-NEXT:    .byte    9       # TB_TAG_PC4
; CHECK-NEXT:    .long    {{.*}}  # TB_AT_PC4
; CHECK-NEXT:    .byte    4       # TB_TAG_LN1
; CHECK-NEXT:    .byte    1       # TB_AT_LN1
; CHECK-NEXT:    .byte    9       # TB_TAG_PC4
; CHECK-NEXT:    .long    {{.*}}  # TB_AT_PC4
; CHECK-NEXT:    .byte    4       # TB_TAG_LN1
; CHECK-NEXT:    .byte    2       # TB_AT_LN1
; CHECK-NEXT:    .byte    9       # TB_TAG_PC4
; CHECK-NEXT:    .long    {{.*}}  # TB_AT_PC4
; CHECK-NEXT:    .byte    4       # TB_TAG_LN1
; CHECK-NEXT:    .byte    1       # TB_AT_LN1
; CHECK-NEXT:    .byte    9       # TB_TAG_PC4
; CHECK-NEXT:    .long    {{.*}}  # TB_AT_PC4

; To regenerate the test file traceback-source-line.ll
; clang -O2 -traceback -S -emit-llvm traceback-source-line.c

define dso_local i32 @binop(i32 %a, i32 %b) local_unnamed_addr !dbg !7 {
entry:
  %tobool.not = icmp eq i32 %a, 0, !dbg !9
  br i1 %tobool.not, label %if.else, label %if.then, !dbg !9

if.then:                                          ; preds = %entry
  %call = tail call i32 @foo(), !dbg !10
  br label %if.end, !dbg !11

if.else:                                          ; preds = %entry
  %call1 = tail call i32 @bar(), !dbg !12
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %call.pn = phi i32 [ %call, %if.then ], [ %call1, %if.else ]
  %b.addr.0 = sub nsw i32 %b, %call.pn, !dbg !13
  ret i32 %b.addr.0, !dbg !14
}

declare dso_local i32 @foo() local_unnamed_addr

declare dso_local i32 @bar() local_unnamed_addr

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 11.0.0", isOptimized: true, runtimeVersion: 0, emissionKind: DebugDirectivesOnly, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "traceback-source-line.c", directory: "/temp")
!2 = !{}
!3 = !{i32 2, !"TraceBack", i32 1}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 11.0.0"}
!7 = distinct !DISubprogram(name: "binop", scope: !1, file: !1, line: 4, type: !8, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !2)
!9 = !DILocation(line: 5, column: 7, scope: !7)
!10 = !DILocation(line: 6, column: 10, scope: !7)
!11 = !DILocation(line: 6, column: 5, scope: !7)
!12 = !DILocation(line: 8, column: 10, scope: !7)
!13 = !DILocation(line: 0, scope: !7)
!14 = !DILocation(line: 9, column: 3, scope: !7)

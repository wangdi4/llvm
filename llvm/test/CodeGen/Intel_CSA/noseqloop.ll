; RUN: llc -pass-remarks-missed=csa-.* < %s | FileCheck %s
; RUN: llc -pass-remarks-missed=csa-.* < %s 2>&1 | FileCheck -check-prefix=REMARKS %s
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; CHECK-LABEL: test_while_stride
; CHECK-DAG: replace1 %[[PRED:[a-zA-Z0-9_.]+]], %[[COND:[a-zA-Z0-9_.]+]], 1, 0, 2, 2
; CHECK-DAG: .curr %[[PRED]]; .value 1
; CHECK-DAG: repeat32 %[[OUTPUT1:[a-zA-Z0-9_.]+]], %[[PRED]]
; CHECK-DAG: stride64 %[[OUTPUT2:[a-zA-Z0-9_.]+]], %[[PRED]]
define dso_local i32 @test_while_stride(i32* %arr, i32 %check) local_unnamed_addr !dbg !8 {
entry:
  br label %while.cond, !dbg !10

while.cond:                                       ; preds = %while.cond, %entry
  %arr.addr.0 = phi i32* [ %arr, %entry ], [ %incdec.ptr, %while.cond ]
  %incdec.ptr = getelementptr inbounds i32, i32* %arr.addr.0, i64 1, !dbg !11
  %0 = load volatile i32, i32* %arr.addr.0, align 4, !dbg !12, !tbaa !13
  %cmp = icmp eq i32 %0, %check, !dbg !17
  br i1 %cmp, label %while.end, label %while.cond, !dbg !10, !llvm.loop !18

while.end:                                        ; preds = %while.cond
  %sub.ptr.lhs.cast = ptrtoint i32* %incdec.ptr to i64, !dbg !20
  %sub.ptr.rhs.cast = ptrtoint i32* %arr to i64, !dbg !20
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, %sub.ptr.rhs.cast, !dbg !20
  %1 = lshr exact i64 %sub.ptr.sub, 2, !dbg !20
  %conv = trunc i64 %1 to i32, !dbg !21
  ret i32 %conv, !dbg !22
}

; REMARKS: test.c:4:3:  loop will not be driven by a sequence operator

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = distinct !DISubprogram(name: "test_while_stride", scope: !1, file: !1, line: 2, type: !9, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 4, column: 3, scope: !8)
!11 = !DILocation(line: 4, column: 14, scope: !8)
!12 = !DILocation(line: 4, column: 10, scope: !8)
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !DILocation(line: 4, column: 17, scope: !8)
!18 = distinct !{!18, !10, !19}
!19 = !DILocation(line: 4, column: 26, scope: !8)
!20 = !DILocation(line: 5, column: 14, scope: !8)
!21 = !DILocation(line: 5, column: 10, scope: !8)
!22 = !DILocation(line: 5, column: 3, scope: !8)

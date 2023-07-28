; RUN: opt -passes="loop(loop-reduce)" -S < %s | FileCheck %s

; 11269: LSR was holding deleted SCEVs and values, while trying to update
; debug information.

; CHECK-NOT: %spec.select.1
; CHECK: call void @llvm.dbg.value(metadata ptr

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.MLoopCol.28.228.314.343.458.602.631.689.718.950.979.1008.1034.1112.1373 = type { i8, i8, i8, i8 }

define dso_local void @badcase() local_unnamed_addr #0 !dbg !215 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %lcol18.0120 = phi ptr [ undef, %entry ], [ %spec.select, %for.body ]
  %p_lcol.0119 = phi ptr [ undef, %entry ], [ %spec.select1, %for.body ]
  %spec.select = select i1 undef, ptr %lcol18.0120, ptr null
  %spec.select1 = select i1 undef, ptr %p_lcol.0119, ptr null
  call void @llvm.dbg.value(metadata ptr %spec.select1, metadata !507, metadata !DIExpression()), !dbg !515
  br label %for.body
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!214}
!214 = !{i32 2, !"Debug Info Version", i32 3}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "x", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "x.c", directory: "/x")
!215 = distinct !DISubprogram(name: "x", scope: !1, file: !1, line: 460, type: !216, scopeLine: 461, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!216 = !DISubroutineType(types: !217)
!217 = !{null}
!488 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "x", file: !1, line: 129, size: 32)
!507 = !DILocalVariable(name: "x", scope: !514, file: !1, line: 497)
;, type: !488)
!514 = distinct !DILexicalBlock(scope: !215, file: !1, line: 469, column: 6)
!515 = !DILocation(line: 0, scope: !514)

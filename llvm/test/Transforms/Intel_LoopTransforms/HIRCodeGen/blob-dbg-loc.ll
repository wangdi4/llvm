; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -force-hir-cg -S 2>&1 < %s | FileCheck %s

; Verify that debug location of %add and %mul instructions which get folded into
; the blob ((%t1 + %ld) * %t2) is recovered during CG. Previously, it was
; assigned the debug location of the store which is the use site (rval of store).

; The other issue is that parser should have used the debug location of %mul for
; this blob as that is the value we formed the SCEV for. This needs to be fixed
; on the parser side.

; CHECK: + DO i1 = 0, 63, 1   <DO_LOOP>
; CHECK: |   %ld = (%A)[i1];
; CHECK: |   (%B)[i1] = ((%t1 + %ld) * %t2);
; CHECK: + END LOOP

; CHECK: loop.{{.*}}:
; CHECK: = add i64 %t1, %t{{.*}}, !dbg [[ADDDBGLOC:![0-9]+]]
; CHECK: = mul i64 %t2, %{{.*}}, !dbg [[MULDBGLOC:![0-9]+]]

; CHECK: [[ADDDBGLOC]] = !DILocation(line: 7, column: 1, scope: !{{.*}})
; CHECK: [[MULDBGLOC]] = !DILocation(line: 9, column: 21, scope: !{{.*}})


define void @foo(ptr %A, i64 %t1, i64 %t2, ptr %B) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx1 = getelementptr inbounds i64, ptr %A, i64 %indvars.iv
  %ld = load i64, ptr %arrayidx1, !dbg !46
  %add = add i64 %t1, %ld, !dbg !30
  %mul = mul i64 %add, %t2, !dbg !41
  %arrayidx2 = getelementptr inbounds i64, ptr %B, i64 %indvars.iv
  store i64 %mul, ptr %arrayidx2, align 4, !dbg !46
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 64
  br i1 %exitcond, label %for.cond.cleanup, label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void
}

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!25, !26, !27, !28}
!llvm.ident = !{!29}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, globals: !16, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "t.c", directory: "/localdisk2/pchawla/ics/tests/CMPLRLLVM-28793/reduced")
!2 = !{}
!3 = !{!4, !11}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIDerivedType(tag: DW_TAG_typedef, name: "__m64", file: !6, line: 13, baseType: !7, align: 64)
!6 = !DIFile(filename: "xmain-web/deploy/linux_prod/lib/clang/13.0.0/include/mmintrin.h", directory: "/localdisk2/pchawla/ics")
!7 = !DICompositeType(tag: DW_TAG_array_type, baseType: !8, size: 64, flags: DIFlagVector, elements: !9)
!8 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!9 = !{!10}
!10 = !DISubrange(count: 1)
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "__v2si", file: !6, line: 16, baseType: !12)
!12 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, size: 64, flags: DIFlagVector, elements: !14)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !{!15}
!15 = !DISubrange(count: 2)
!16 = !{!17, !19, !21, !23}
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(name: "a", scope: !0, file: !1, line: 2, type: !13, isLocal: false, isDefinition: true)
!19 = !DIGlobalVariableExpression(var: !20, expr: !DIExpression())
!20 = distinct !DIGlobalVariable(name: "b", scope: !0, file: !1, line: 2, type: !13, isLocal: false, isDefinition: true)
!21 = !DIGlobalVariableExpression(var: !22, expr: !DIExpression())
!22 = distinct !DIGlobalVariable(name: "c", scope: !0, file: !1, line: 2, type: !13, isLocal: false, isDefinition: true)
!23 = !DIGlobalVariableExpression(var: !24, expr: !DIExpression())
!24 = distinct !DIGlobalVariable(name: "d", scope: !0, file: !1, line: 2, type: !13, isLocal: false, isDefinition: true)
!25 = !{i32 7, !"Dwarf Version", i32 4}
!26 = !{i32 2, !"Debug Info Version", i32 3}
!27 = !{i32 1, !"wchar_size", i32 4}
!28 = !{i32 7, !"uwtable", i32 1}
!29 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!30 = !DILocation(line: 7, column: 1, scope: !31)
!31 = distinct !DILexicalBlock(scope: !32, file: !1, line: 7, column: 1)
!32 = distinct !DISubprogram(name: "e", scope: !1, file: !1, line: 3, type: !33, scopeLine: 3, flags: DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !35)
!33 = !DISubroutineType(types: !34)
!34 = !{!13}
!35 = !{!36, !37, !39}
!36 = !DILocalVariable(name: "f", scope: !32, file: !1, line: 4, type: !5)
!37 = !DILocalVariable(name: "g", scope: !32, file: !1, line: 5, type: !38)
!38 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!39 = !DILocalVariable(name: "i", scope: !32, file: !1, line: 6, type: !40)
!40 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, size: 32, elements: !9)
!41 = !DILocation(line: 9, column: 21, scope: !42)
!42 = distinct !DILexicalBlock(scope: !43, file: !1, line: 9, column: 14)
!43 = distinct !DILexicalBlock(scope: !44, file: !1, line: 9, column: 14)
!44 = distinct !DILexicalBlock(scope: !45, file: !1, line: 9, column: 1)
!45 = distinct !DILexicalBlock(scope: !31, file: !1, line: 7, column: 1)
!46 = !DILocation(line: 10, column: 21, scope: !42)

; Source code:
;void foo(int *a) {
;  int i;
;  for (i=0;i<100;i++) {
;    if (i == 0) {
;      a[i] = 0;
;    }
;    a[i] += i;
;  }
;}

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-var-predicate -disable-output -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-var-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -disable-output -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT

; We test the optreport for three things:
;
; (1) "<Predicate Optimized v1>" is not printed when there is less than two output
;     loops.
; (2) If the input loop is optimized away, and we have a different output loop,
;     any remarks from the input loop (e.g., from previous passes) are moved to
;     the output loop. Remark #25579 is a dummy remark included in the input
;     llvm-ir loop's metadata for test purposes only.
; (3) If the loop is peeled, the peeling-specific remark (#25258) is printed
;     instead of the generic remark for hir-opt-var-predicate.
;
;OPTREPORT:     LOOP BEGIN at foo.c (3, 3)
;OPTREPORT-NOT: <Predicate Optimized v1>
;OPTREPORT:         remark #25579: Loop was reversed
;OPTREPORT:         remark #25258: Loop peeled using condition at line 4
;OPTREPORT:     LOOP END


;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %a) local_unnamed_addr #0 !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata i32* %a, metadata !14, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 0, metadata !15, metadata !DIExpression()), !dbg !17
  br label %for.body, !dbg !18

for.body:                                         ; preds = %if.end, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !15, metadata !DIExpression()), !dbg !17
  %cmp1 = icmp eq i64 %indvars.iv, 0, !dbg !20
  br i1 %cmp1, label %if.then, label %if.end, !dbg !24

if.then:                                          ; preds = %for.body
  store i32 0, i32* %a, align 4, !dbg !25, !tbaa !27
  br label %if.end, !dbg !31

if.end:                                           ; preds = %if.then, %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv, !dbg !32
  %0 = load i32, i32* %arrayidx3, align 4, !dbg !33, !tbaa !27
  %1 = trunc i64 %indvars.iv to i32, !dbg !33
  %add = add nsw i32 %0, %1, !dbg !33
  store i32 %add, i32* %arrayidx3, align 4, !dbg !33, !tbaa !27
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !34
  %exitcond = icmp eq i64 %indvars.iv.next, 100, !dbg !35
  br i1 %exitcond, label %for.end, label %for.body, !dbg !18, !llvm.loop !36

for.end:                                          ; preds = %if.end
  ret void, !dbg !38
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c350c42c99febedb6ec27e7ffc00c745441132b0)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "foo.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm c350c42c99febedb6ec27e7ffc00c745441132b0)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15}
!14 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 2, type: !12)
!16 = !DILocation(line: 1, column: 15, scope: !8)
!17 = !DILocation(line: 2, column: 7, scope: !8)
!18 = !DILocation(line: 3, column: 3, scope: !19)
!19 = distinct !DILexicalBlock(scope: !8, file: !1, line: 3, column: 3)
!20 = !DILocation(line: 4, column: 11, scope: !21)
!21 = distinct !DILexicalBlock(scope: !22, file: !1, line: 4, column: 9)
!22 = distinct !DILexicalBlock(scope: !23, file: !1, line: 3, column: 23)
!23 = distinct !DILexicalBlock(scope: !19, file: !1, line: 3, column: 3)
!24 = !DILocation(line: 4, column: 9, scope: !22)
!25 = !DILocation(line: 5, column: 12, scope: !26)
!26 = distinct !DILexicalBlock(scope: !21, file: !1, line: 4, column: 17)
!27 = !{!28, !28, i64 0}
!28 = !{!"int", !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 6, column: 5, scope: !26)
!32 = !DILocation(line: 7, column: 5, scope: !22)
!33 = !DILocation(line: 7, column: 10, scope: !22)
!34 = !DILocation(line: 3, column: 19, scope: !23)
!35 = !DILocation(line: 3, column: 13, scope: !23)
!36 = distinct !{!36, !18, !37, !39}
!37 = !DILocation(line: 8, column: 3, scope: !19)
!38 = !DILocation(line: 9, column: 1, scope: !8)
!39 = distinct !{!"intel.optreport.rootnode", !40}
!40 = distinct !{!"intel.optreport", !41}
!41 = !{!"intel.optreport.remarks", !42}
!42 = !{!"intel.optreport.remark", i32 25579, !"Loop was reversed"}

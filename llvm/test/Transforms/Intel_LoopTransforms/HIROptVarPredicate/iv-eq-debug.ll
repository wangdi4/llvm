; Source code:
;
; void foo(int *p, int *q, long n, long d) {
;   long j;
;   for (j=0;j<n;++j) {
;     if (j == d) {
;       p[j] = j;
;     } else {
;       q[j] = j;
;     }
;   }
; }

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-var-predicate -disable-output -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-var-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -disable-output -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;
;OPTREPORT: Global optimization report for : foo
;
;OPTREPORT: LOOP BEGIN at t1.c (3, 3)
;OPTREPORT: <Predicate Optimized v1>
;OPTREPORT:     remark #25580: Induction variable range split using condition at line 4
;OPTREPORT: LOOP END
;
;OPTREPORT: LOOP BEGIN at t1.c (3, 3)
;OPTREPORT: <Predicate Optimized v2>
;OPTREPORT: LOOP END


;Module Before HIR; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i32* nocapture %p, i32* nocapture %q, i64 %n, i64 %d) local_unnamed_addr #0 !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata i32* %p, metadata !15, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.value(metadata i32* %q, metadata !16, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i64 %n, metadata !17, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.value(metadata i64 %d, metadata !18, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.value(metadata i64 0, metadata !19, metadata !DIExpression()), !dbg !24
  %cmp10 = icmp sgt i64 %n, 0, !dbg !25
  br i1 %cmp10, label %for.body.lr.ph, label %for.end, !dbg !28

for.body.lr.ph:                                   ; preds = %entry
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %d
  br label %for.body, !dbg !28

for.body:                                         ; preds = %for.inc, %for.body.lr.ph
  %j.011 = phi i64 [ 0, %for.body.lr.ph ], [ %inc, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %j.011, metadata !19, metadata !DIExpression()), !dbg !24
  %cmp1 = icmp eq i64 %j.011, %d, !dbg !29
  %conv = trunc i64 %j.011 to i32
  br i1 %cmp1, label %if.then, label %if.else, !dbg !32

if.then:                                          ; preds = %for.body
  store i32 %conv, i32* %arrayidx, align 4, !dbg !33, !tbaa !35
  br label %for.inc, !dbg !39

if.else:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %q, i64 %j.011, !dbg !40
  store i32 %conv, i32* %arrayidx3, align 4, !dbg !42, !tbaa !35
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %inc = add nuw nsw i64 %j.011, 1, !dbg !43
  call void @llvm.dbg.value(metadata i64 %inc, metadata !19, metadata !DIExpression()), !dbg !24
  %exitcond = icmp eq i64 %inc, %n, !dbg !25
  br i1 %exitcond, label %for.end.loopexit, label %for.body, !dbg !28, !llvm.loop !44

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end, !dbg !46

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void, !dbg !46
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm da575b3ba1cb39c0292fa629466b9c8affc9330d)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "t1.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm da575b3ba1cb39c0292fa629466b9c8affc9330d)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !14)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11, !11, !13, !13}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!14 = !{!15, !16, !17, !18, !19}
!15 = !DILocalVariable(name: "p", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!16 = !DILocalVariable(name: "q", arg: 2, scope: !8, file: !1, line: 1, type: !11)
!17 = !DILocalVariable(name: "n", arg: 3, scope: !8, file: !1, line: 1, type: !13)
!18 = !DILocalVariable(name: "d", arg: 4, scope: !8, file: !1, line: 1, type: !13)
!19 = !DILocalVariable(name: "j", scope: !8, file: !1, line: 2, type: !13)
!20 = !DILocation(line: 1, column: 15, scope: !8)
!21 = !DILocation(line: 1, column: 23, scope: !8)
!22 = !DILocation(line: 1, column: 31, scope: !8)
!23 = !DILocation(line: 1, column: 39, scope: !8)
!24 = !DILocation(line: 2, column: 8, scope: !8)
!25 = !DILocation(line: 3, column: 13, scope: !26)
!26 = distinct !DILexicalBlock(scope: !27, file: !1, line: 3, column: 3)
!27 = distinct !DILexicalBlock(scope: !8, file: !1, line: 3, column: 3)
!28 = !DILocation(line: 3, column: 3, scope: !27)
!29 = !DILocation(line: 4, column: 11, scope: !30)
!30 = distinct !DILexicalBlock(scope: !31, file: !1, line: 4, column: 9)
!31 = distinct !DILexicalBlock(scope: !26, file: !1, line: 3, column: 21)
!32 = !DILocation(line: 4, column: 9, scope: !31)
!33 = !DILocation(line: 5, column: 12, scope: !34)
!34 = distinct !DILexicalBlock(scope: !30, file: !1, line: 4, column: 17)
!35 = !{!36, !36, i64 0}
!36 = !{!"int", !37, i64 0}
!37 = !{!"omnipotent char", !38, i64 0}
!38 = !{!"Simple C/C++ TBAA"}
!39 = !DILocation(line: 6, column: 5, scope: !34)
!40 = !DILocation(line: 7, column: 7, scope: !41)
!41 = distinct !DILexicalBlock(scope: !30, file: !1, line: 6, column: 12)
!42 = !DILocation(line: 7, column: 12, scope: !41)
!43 = !DILocation(line: 3, column: 16, scope: !26)
!44 = distinct !{!44, !28, !45}
!45 = !DILocation(line: 9, column: 3, scope: !27)
!46 = !DILocation(line: 10, column: 1, scope: !8)

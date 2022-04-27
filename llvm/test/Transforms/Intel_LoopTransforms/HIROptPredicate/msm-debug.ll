; Source code:
;int A[1000];
;int B[1000];
;int foo(int y){
;  int i1, i2;
;  for(i1 = 0; i1 < 1000; i1++){
;    for(i2 = 0; i2 < 1000; i2++){
;      if(i1 < 50){
;        B[i1] = i2;
;      }
;      if(y > 0){
;        A[i2] += y;
;      }
;    }
;  }
;}

;*** IR Dump After HIR OptPredicate ***
;Function: foo
;
;<0>       BEGIN REGION { modified }
;<18>            if (%y > 0)
;<18>            {
;<41>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<10>               |   if (i1 <u 50)
;<10>               |   {
;<42>               |      + DO i2 = 0, 999, 1   <DO_LOOP>
;<15>               |      |   (@B)[0][i1] = i2;
;<23>               |      |   %1 = (@A)[0][i2];
;<25>               |      |   (@A)[0][i2] = %y + %1;
;<42>               |      + END LOOP
;<10>               |   }
;<10>               |   else
;<10>               |   {
;<43>               |      + DO i2 = 0, 999, 1   <DO_LOOP>
;<46>               |      |   %1 = (@A)[0][i2];
;<47>               |      |   (@A)[0][i2] = %y + %1;
;<43>               |      + END LOOP
;<10>               |   }
;<41>               + END LOOP
;<18>            }
;<18>            else
;<18>            {
;<48>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<49>               |   if (i1 <u 50)
;<49>               |   {
;<50>               |      + DO i2 = 0, 999, 1   <DO_LOOP>
;<51>               |      |   (@B)[0][i1] = i2;
;<50>               |      + END LOOP
;<49>               |   }
;<48>               + END LOOP
;<18>            }
;<0>       END REGION

;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;
; Line number repetition ("at lines 10 and 10" in the remark) is not a bug.
; It results from multiple transformations applied to the same line number.
;
;OPTREPORT: Global optimization report for : foo
;
;OPTREPORT: LOOP BEGIN at foo.c (5, 3)
;OPTREPORT: <Predicate Optimized v4>
;
;OPTREPORT:     LOOP BEGIN at foo.c (6, 5)
;OPTREPORT:     LOOP END
;
;OPTREPORT:     LOOP BEGIN at foo.c (6, 5)
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END
;
;OPTREPORT: LOOP BEGIN at foo.c (5, 3)
;OPTREPORT: <Predicate Optimized v3>
;OPTREPORT:     remark #25423: Invariant If condition at lines 10 and 10 hoisted out of this loop
;
;OPTREPORT:     LOOP BEGIN at foo.c (6, 5)
;OPTREPORT:     <Predicate Optimized v2>
;OPTREPORT:     LOOP END
;
;OPTREPORT:     LOOP BEGIN at foo.c (6, 5)
;OPTREPORT:     <Predicate Optimized v1>
;OPTREPORT:         remark #25423: Invariant If condition at line 7 hoisted out of this loop
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END

;Module Before HIR; ModuleID = 'foo.c'
source_filename = "foo.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16, !dbg !0
@A = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16, !dbg !6

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %y) local_unnamed_addr #0 !dbg !17 {
entry:
  call void @llvm.dbg.value(metadata i32 %y, metadata !21, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !25
  %cmp5 = icmp sgt i32 %y, 0
  br label %for.body, !dbg !26

for.body:                                         ; preds = %for.inc10, %entry
  %indvars.iv23 = phi i64 [ 0, %entry ], [ %indvars.iv.next24, %for.inc10 ]
  call void @llvm.dbg.value(metadata i32 0, metadata !23, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.value(metadata i64 %indvars.iv23, metadata !22, metadata !DIExpression()), !dbg !25
  %cmp4 = icmp ult i64 %indvars.iv23, 50
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @B, i64 0, i64 %indvars.iv23
  br label %for.body3, !dbg !29

for.body3:                                        ; preds = %for.inc, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !23, metadata !DIExpression()), !dbg !28
  br i1 %cmp4, label %if.then, label %if.end, !dbg !33

if.then:                                          ; preds = %for.body3
  %0 = trunc i64 %indvars.iv to i32, !dbg !36
  store i32 %0, i32* %arrayidx, align 4, !dbg !36, !tbaa !39
  br label %if.end, !dbg !44

if.end:                                           ; preds = %if.then, %for.body3
  br i1 %cmp5, label %if.then6, label %for.inc, !dbg !45

if.then6:                                         ; preds = %if.end
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @A, i64 0, i64 %indvars.iv, !dbg !46
  %1 = load i32, i32* %arrayidx8, align 4, !dbg !49, !tbaa !39
  %add = add nsw i32 %1, %y, !dbg !49
  store i32 %add, i32* %arrayidx8, align 4, !dbg !49, !tbaa !39
  br label %for.inc, !dbg !50

for.inc:                                          ; preds = %if.end, %if.then6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !51
  %exitcond = icmp eq i64 %indvars.iv.next, 1000, !dbg !52
  br i1 %exitcond, label %for.inc10, label %for.body3, !dbg !29, !llvm.loop !53

for.inc10:                                        ; preds = %for.inc
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1, !dbg !55
  %exitcond25 = icmp eq i64 %indvars.iv.next24, 1000, !dbg !56
  br i1 %exitcond25, label %for.end12, label %for.body, !dbg !26, !llvm.loop !57

for.end12:                                        ; preds = %for.inc10
  ret i32 undef, !dbg !59
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!12, !13, !14}
!llvm.dbg.intel.emit_class_debug_always = !{!15}
!llvm.ident = !{!16}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "B", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 2e83120ebed4cadb8b9eefd77d3b6853adc26455)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "foo.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!4 = !{}
!5 = !{!6, !0}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "A", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 32000, elements: !10)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{!11}
!11 = !DISubrange(count: 1000)
!12 = !{i32 2, !"Dwarf Version", i32 4}
!13 = !{i32 2, !"Debug Info Version", i32 3}
!14 = !{i32 1, !"wchar_size", i32 4}
!15 = !{!"true"}
!16 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 2e83120ebed4cadb8b9eefd77d3b6853adc26455)"}
!17 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 3, type: !18, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: true, unit: !2, retainedNodes: !20)
!18 = !DISubroutineType(types: !19)
!19 = !{!9, !9}
!20 = !{!21, !22, !23}
!21 = !DILocalVariable(name: "y", arg: 1, scope: !17, file: !3, line: 3, type: !9)
!22 = !DILocalVariable(name: "i1", scope: !17, file: !3, line: 4, type: !9)
!23 = !DILocalVariable(name: "i2", scope: !17, file: !3, line: 4, type: !9)
!24 = !DILocation(line: 3, column: 13, scope: !17)
!25 = !DILocation(line: 4, column: 7, scope: !17)
!26 = !DILocation(line: 5, column: 3, scope: !27)
!27 = distinct !DILexicalBlock(scope: !17, file: !3, line: 5, column: 3)
!28 = !DILocation(line: 4, column: 11, scope: !17)
!29 = !DILocation(line: 6, column: 5, scope: !30)
!30 = distinct !DILexicalBlock(scope: !31, file: !3, line: 6, column: 5)
!31 = distinct !DILexicalBlock(scope: !32, file: !3, line: 5, column: 31)
!32 = distinct !DILexicalBlock(scope: !27, file: !3, line: 5, column: 3)
!33 = !DILocation(line: 7, column: 10, scope: !34)
!34 = distinct !DILexicalBlock(scope: !35, file: !3, line: 6, column: 33)
!35 = distinct !DILexicalBlock(scope: !30, file: !3, line: 6, column: 5)
!36 = !DILocation(line: 8, column: 15, scope: !37)
!37 = distinct !DILexicalBlock(scope: !38, file: !3, line: 7, column: 18)
!38 = distinct !DILexicalBlock(scope: !34, file: !3, line: 7, column: 10)
!39 = !{!40, !41, i64 0}
!40 = !{!"array@_ZTSA1000_i", !41, i64 0}
!41 = !{!"int", !42, i64 0}
!42 = !{!"omnipotent char", !43, i64 0}
!43 = !{!"Simple C/C++ TBAA"}
!44 = !DILocation(line: 9, column: 7, scope: !37)
!45 = !DILocation(line: 10, column: 10, scope: !34)
!46 = !DILocation(line: 11, column: 9, scope: !47)
!47 = distinct !DILexicalBlock(scope: !48, file: !3, line: 10, column: 16)
!48 = distinct !DILexicalBlock(scope: !34, file: !3, line: 10, column: 10)
!49 = !DILocation(line: 11, column: 15, scope: !47)
!50 = !DILocation(line: 12, column: 7, scope: !47)
!51 = !DILocation(line: 6, column: 30, scope: !35)
!52 = !DILocation(line: 6, column: 20, scope: !35)
!53 = distinct !{!53, !29, !54}
!54 = !DILocation(line: 13, column: 5, scope: !30)
!55 = !DILocation(line: 5, column: 28, scope: !32)
!56 = !DILocation(line: 5, column: 18, scope: !32)
!57 = distinct !{!57, !26, !58}
!58 = !DILocation(line: 14, column: 3, scope: !27)
!59 = !DILocation(line: 15, column: 1, scope: !17)

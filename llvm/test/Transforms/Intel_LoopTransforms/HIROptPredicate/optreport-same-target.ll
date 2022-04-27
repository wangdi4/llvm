; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low -disable-output 2>&1 | FileCheck %s -check-prefix=OPTREPORT


; Source code:
;
;void foo(int *p, int *q, int n) {
;  for (int i=0;i<n;++i) {
;    for (int j=0;j<n;++j) {
;      p[i] = j;
;      if (i == 16) {
;        q[j]++;
;      }
;      q[j] = p[j];
;      if (i == 4) {
;        q[j] = p[j] + 1;
;      }
;    }
;  }
;}

;*** IR Dump Before HIR OptPredicate ***
;Function: foo
;
;<0>       BEGIN REGION { }
;<54>            + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<55>            |   + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<12>            |   |   (%p)[i1] = i2;
;<13>            |   |   if (i1 == 16)
;<13>            |   |   {
;<31>            |   |      %1 = (%q)[i2];
;<33>            |   |      (%q)[i2] = %1 + 1;
;<35>            |   |      %2 = (%p)[i2];
;<36>            |   |      (%q)[i2] = %2;
;<13>            |   |   }
;<13>            |   |   else
;<13>            |   |   {
;<18>            |   |      %3 = (%p)[i2];
;<20>            |   |      (%q)[i2] = %3;
;<21>            |   |      if (i1 == 4)
;<21>            |   |      {
;<25>            |   |         %4 = (%p)[i2];
;<27>            |   |         (%q)[i2] = %4 + 1;
;<21>            |   |      }
;<13>            |   |   }
;<55>            |   + END LOOP
;<54>            + END LOOP
;<0>       END REGION
;
;*** IR Dump After HIR OptPredicate ***
;Function: foo
;
;<0>       BEGIN REGION { modified }
;<54>            + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<13>            |   if (i1 == 16)
;<13>            |   {
;<55>            |      + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<12>            |      |   (%p)[i1] = i2;
;<31>            |      |   %1 = (%q)[i2];
;<33>            |      |   (%q)[i2] = %1 + 1;
;<35>            |      |   %2 = (%p)[i2];
;<36>            |      |   (%q)[i2] = %2;
;<55>            |      + END LOOP
;<13>            |   }
;<13>            |   else
;<13>            |   {
;<21>            |      if (i1 == 4)
;<21>            |      {
;<56>            |         + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<57>            |         |   (%p)[i1] = i2;
;<18>            |         |   %3 = (%p)[i2];
;<20>            |         |   (%q)[i2] = %3;
;<25>            |         |   %4 = (%p)[i2];
;<27>            |         |   (%q)[i2] = %4 + 1;
;<56>            |         + END LOOP
;<21>            |      }
;<21>            |      else
;<21>            |      {
;<59>            |         + DO i2 = 0, sext.i32.i64(%n) + -1, 1   <DO_LOOP>
;<60>            |         |   (%p)[i1] = i2;
;<61>            |         |   %3 = (%p)[i2];
;<62>            |         |   (%q)[i2] = %3;
;<59>            |         + END LOOP
;<21>            |      }
;<13>            |   }
;<54>            + END LOOP
;<0>       END REGION
;
;Global optimization report for : foo
;
;OPTREPORT: LOOP BEGIN at bar.c (2, 3)
;OPTREPORT:     LOOP BEGIN at bar.c (3, 5)
;OPTREPORT:     <Predicate Optimized v3>
;OPTREPORT:     LOOP END
;OPTREPORT:     LOOP BEGIN at bar.c (3, 5)
;OPTREPORT:     <Predicate Optimized v2>
;OPTREPORT:         remark #25423: Invariant If condition at line 9 hoisted out of this loop
;OPTREPORT:     LOOP END
;OPTREPORT:     LOOP BEGIN at bar.c (3, 5)
;OPTREPORT:     <Predicate Optimized v1>
;OPTREPORT:         remark #25423: Invariant If condition at line 5 hoisted out of this loop
;OPTREPORT:     LOOP END
;OPTREPORT: LOOP END

;Module Before HIR; ModuleID = 'bar.c'
source_filename = "bar.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %p, i32* nocapture %q, i32 %n) local_unnamed_addr #0 !dbg !8 {
entry:
  call void @llvm.dbg.value(metadata i32* %p, metadata !14, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.value(metadata i32* %q, metadata !15, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i32 %n, metadata !16, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 0, metadata !17, metadata !DIExpression()), !dbg !26
  %cmp46 = icmp sgt i32 %n, 0, !dbg !27
  br i1 %cmp46, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup, !dbg !28

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %wide.trip.count = sext i32 %n to i64
  br label %for.body4.lr.ph, !dbg !28

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv48 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next49, %for.cond.cleanup3 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv48, metadata !17, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 0, metadata !19, metadata !DIExpression()), !dbg !29
  %arrayidx = getelementptr inbounds i32, i32* %p, i64 %indvars.iv48
  %cmp5 = icmp eq i64 %indvars.iv48, 16
  %cmp12 = icmp eq i64 %indvars.iv48, 4
  br label %for.body4, !dbg !30

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup, !dbg !31

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void, !dbg !31

for.cond.cleanup3:                                ; preds = %for.inc
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1, !dbg !32
  call void @llvm.dbg.value(metadata i32 undef, metadata !17, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !26
  %exitcond51 = icmp eq i64 %indvars.iv.next49, %wide.trip.count, !dbg !27
  br i1 %exitcond51, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph, !dbg !28, !llvm.loop !33

for.body4:                                        ; preds = %for.inc, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !19, metadata !DIExpression()), !dbg !29
  %0 = trunc i64 %indvars.iv to i32, !dbg !35
  store i32 %0, i32* %arrayidx, align 4, !dbg !35, !tbaa !38
  br i1 %cmp5, label %if.then, label %if.end, !dbg !42

if.then:                                          ; preds = %for.body4
  %arrayidx7 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv, !dbg !43
  %1 = load i32, i32* %arrayidx7, align 4, !dbg !46, !tbaa !38
  %inc = add nsw i32 %1, 1, !dbg !46
  store i32 %inc, i32* %arrayidx7, align 4, !dbg !46, !tbaa !38
  %arrayidx941 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv, !dbg !47
  %2 = load i32, i32* %arrayidx941, align 4, !dbg !47, !tbaa !38
  store i32 %2, i32* %arrayidx7, align 4, !dbg !48, !tbaa !38
  br label %for.inc, !dbg !49

if.end:                                           ; preds = %for.body4
  %arrayidx9 = getelementptr inbounds i32, i32* %p, i64 %indvars.iv, !dbg !47
  %3 = load i32, i32* %arrayidx9, align 4, !dbg !47, !tbaa !38
  %arrayidx11 = getelementptr inbounds i32, i32* %q, i64 %indvars.iv, !dbg !50
  store i32 %3, i32* %arrayidx11, align 4, !dbg !48, !tbaa !38
  br i1 %cmp12, label %if.then13, label %for.inc, !dbg !49

if.then13:                                        ; preds = %if.end
  %4 = load i32, i32* %arrayidx9, align 4, !dbg !51, !tbaa !38
  %add = add nsw i32 %4, 1, !dbg !54
  store i32 %add, i32* %arrayidx11, align 4, !dbg !55, !tbaa !38
  br label %for.inc, !dbg !56

for.inc:                                          ; preds = %if.then, %if.end, %if.then13
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !57
  call void @llvm.dbg.value(metadata i32 undef, metadata !19, metadata !DIExpression(DW_OP_plus_uconst, 1, DW_OP_stack_value)), !dbg !29
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !58
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4, !dbg !30, !llvm.loop !59
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm de29246acda81f2b9ed8f0c1b0d8968e04871ba3)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "bar.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c95d67b22c5ea6ea67afdc54154ea9648f91208c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm de29246acda81f2b9ed8f0c1b0d8968e04871ba3)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11, !11, !12}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15, !16, !17, !19}
!14 = !DILocalVariable(name: "p", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "q", arg: 2, scope: !8, file: !1, line: 1, type: !11)
!16 = !DILocalVariable(name: "n", arg: 3, scope: !8, file: !1, line: 1, type: !12)
!17 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 2, type: !12)
!18 = distinct !DILexicalBlock(scope: !8, file: !1, line: 2, column: 3)
!19 = !DILocalVariable(name: "j", scope: !20, file: !1, line: 3, type: !12)
!20 = distinct !DILexicalBlock(scope: !21, file: !1, line: 3, column: 5)
!21 = distinct !DILexicalBlock(scope: !22, file: !1, line: 2, column: 25)
!22 = distinct !DILexicalBlock(scope: !18, file: !1, line: 2, column: 3)
!23 = !DILocation(line: 1, column: 15, scope: !8)
!24 = !DILocation(line: 1, column: 23, scope: !8)
!25 = !DILocation(line: 1, column: 30, scope: !8)
!26 = !DILocation(line: 2, column: 12, scope: !18)
!27 = !DILocation(line: 2, column: 17, scope: !22)
!28 = !DILocation(line: 2, column: 3, scope: !18)
!29 = !DILocation(line: 3, column: 14, scope: !20)
!30 = !DILocation(line: 3, column: 5, scope: !20)
!31 = !DILocation(line: 14, column: 1, scope: !8)
!32 = !DILocation(line: 2, column: 20, scope: !22)
!33 = distinct !{!33, !28, !34}
!34 = !DILocation(line: 13, column: 3, scope: !18)
!35 = !DILocation(line: 4, column: 12, scope: !36)
!36 = distinct !DILexicalBlock(scope: !37, file: !1, line: 3, column: 27)
!37 = distinct !DILexicalBlock(scope: !20, file: !1, line: 3, column: 5)
!38 = !{!39, !39, i64 0}
!39 = !{!"int", !40, i64 0}
!40 = !{!"omnipotent char", !41, i64 0}
!41 = !{!"Simple C/C++ TBAA"}
!42 = !DILocation(line: 5, column: 11, scope: !36)
!43 = !DILocation(line: 6, column: 9, scope: !44)
!44 = distinct !DILexicalBlock(scope: !45, file: !1, line: 5, column: 20)
!45 = distinct !DILexicalBlock(scope: !36, file: !1, line: 5, column: 11)
!46 = !DILocation(line: 6, column: 13, scope: !44)
!47 = !DILocation(line: 8, column: 14, scope: !36)
!48 = !DILocation(line: 8, column: 12, scope: !36)
!49 = !DILocation(line: 9, column: 11, scope: !36)
!50 = !DILocation(line: 8, column: 7, scope: !36)
!51 = !DILocation(line: 10, column: 16, scope: !52)
!52 = distinct !DILexicalBlock(scope: !53, file: !1, line: 9, column: 19)
!53 = distinct !DILexicalBlock(scope: !36, file: !1, line: 9, column: 11)
!54 = !DILocation(line: 10, column: 21, scope: !52)
!55 = !DILocation(line: 10, column: 14, scope: !52)
!56 = !DILocation(line: 11, column: 7, scope: !52)
!57 = !DILocation(line: 3, column: 22, scope: !37)
!58 = !DILocation(line: 3, column: 19, scope: !37)
!59 = distinct !{!59, !30, !60}
!60 = !DILocation(line: 12, column: 5, scope: !20)

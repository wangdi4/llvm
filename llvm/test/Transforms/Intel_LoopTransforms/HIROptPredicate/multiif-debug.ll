; Source code

;int A[1000][1000];
;int B[1000][1000];
;int C[1000][1000];
;int foo(int n){
;  int i, j;
;  for (i=1; i < 1000; i++) {
;    for (j=1; j < 1000; j++) {
;      A[j][i] = B[i][j] + C[j][i] + B[i+1][j] + B[i][j+1];
;      if(n>10) {
;        B[i+1][j+1] = A[j][i];
;      }
;      if(n>100){
;        B[i+1][j] = i;
;      }
;    }
;    C[i][2] = i;
;  }
;}

;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-opt-predicate -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-predicate,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT

; OPTREPORT: Global optimization report for : foo
; OPTREPORT: LOOP BEGIN at t1.c (6, 3)
; OPTREPORT: <Predicate Optimized v2>
; OPTREPORT:     LOOP BEGIN at t1.c (7, 5)
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN at t1.c (6, 3)
; OPTREPORT: <Predicate Optimized v3>
; OPTREPORT:     LOOP BEGIN at t1.c (7, 5)
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END
; OPTREPORT: LOOP BEGIN at t1.c (6, 3)
; OPTREPORT: <Predicate Optimized v1>
; OPTREPORT:     remark #25423: Invariant If condition at line 9 hoisted out of this loop
; OPTREPORT:     remark #25423: Invariant If condition at line 12 hoisted out of this loop
; OPTREPORT:     LOOP BEGIN at t1.c (7, 5)
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END

;Module Before HIR; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16, !dbg !0
@C = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16, !dbg !12
@A = common local_unnamed_addr global [1000 x [1000 x i32]] zeroinitializer, align 16, !dbg !6

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %n) local_unnamed_addr #0 !dbg !19 {
entry:
  call void @llvm.dbg.value(metadata i32 %n, metadata !23, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 1, metadata !24, metadata !DIExpression()), !dbg !27
  %cmp26 = icmp sgt i32 %n, 10
  %cmp37 = icmp sgt i32 %n, 100
  br label %for.body, !dbg !28

for.body:                                         ; preds = %for.end, %entry
  %indvars.iv77 = phi i64 [ 1, %entry ], [ %indvars.iv.next78, %for.end ]
  call void @llvm.dbg.value(metadata i32 1, metadata !25, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv77, metadata !24, metadata !DIExpression()), !dbg !27
  %indvars.iv.next78 = add nuw nsw i64 %indvars.iv77, 1, !dbg !31
  %0 = trunc i64 %indvars.iv77 to i32
  br label %for.body3, !dbg !33

for.body3:                                        ; preds = %for.inc, %for.body
  %indvars.iv = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.inc ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !25, metadata !DIExpression()), !dbg !30
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @B, i64 0, i64 %indvars.iv77, i64 %indvars.iv, !dbg !36
  %1 = load i32, i32* %arrayidx5, align 4, !dbg !36, !tbaa !39
  %arrayidx9 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @C, i64 0, i64 %indvars.iv, i64 %indvars.iv77, !dbg !45
  %2 = load i32, i32* %arrayidx9, align 4, !dbg !45, !tbaa !39
  %add = add nsw i32 %2, %1, !dbg !46
  %arrayidx14 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @B, i64 0, i64 %indvars.iv.next78, i64 %indvars.iv, !dbg !47
  %3 = load i32, i32* %arrayidx14, align 4, !dbg !47, !tbaa !39
  %add15 = add nsw i32 %add, %3, !dbg !48
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !49
  %arrayidx20 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @B, i64 0, i64 %indvars.iv77, i64 %indvars.iv.next, !dbg !50
  %4 = load i32, i32* %arrayidx20, align 4, !dbg !50, !tbaa !39
  %add21 = add nsw i32 %add15, %4, !dbg !51
  %arrayidx25 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @A, i64 0, i64 %indvars.iv, i64 %indvars.iv77, !dbg !52
  store i32 %add21, i32* %arrayidx25, align 4, !dbg !53, !tbaa !39
  br i1 %cmp26, label %if.then, label %for.inc, !dbg !54

if.then:                                          ; preds = %for.body3
  %arrayidx36 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @B, i64 0, i64 %indvars.iv.next78, i64 %indvars.iv.next, !dbg !55
  store i32 %add21, i32* %arrayidx36, align 4, !dbg !58, !tbaa !39
  br i1 %cmp37, label %if.then38, label %for.inc, !dbg !59

if.then38:                                        ; preds = %if.then
  store i32 %0, i32* %arrayidx14, align 4, !dbg !60, !tbaa !39
  br label %for.inc, !dbg !63

for.inc:                                          ; preds = %for.body3, %if.then, %if.then38
  %exitcond = icmp eq i64 %indvars.iv.next, 1000, !dbg !64
  br i1 %exitcond, label %for.end, label %for.body3, !dbg !33, !llvm.loop !65

for.end:                                          ; preds = %for.inc
  %arrayidx47 = getelementptr inbounds [1000 x [1000 x i32]], [1000 x [1000 x i32]]* @C, i64 0, i64 %indvars.iv77, i64 2, !dbg !67
  %5 = trunc i64 %indvars.iv77 to i32, !dbg !68
  store i32 %5, i32* %arrayidx47, align 8, !dbg !68, !tbaa !39
  %exitcond79 = icmp eq i64 %indvars.iv.next78, 1000, !dbg !69
  br i1 %exitcond79, label %for.end50, label %for.body, !dbg !28, !llvm.loop !70

for.end50:                                        ; preds = %for.end
  ret i32 undef, !dbg !72
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16}
!llvm.dbg.intel.emit_class_debug_always = !{!17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "B", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 76a39c3be98cee923738eefc3d1e13b4966eaa79)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "t1.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIROptPredicate")
!4 = !{}
!5 = !{!6, !0, !12}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "A", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 32000000, elements: !10)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{!11, !11}
!11 = !DISubrange(count: 1000)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "C", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!14 = !{i32 2, !"Dwarf Version", i32 4}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"true"}
!18 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 76a39c3be98cee923738eefc3d1e13b4966eaa79)"}
!19 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 4, type: !20, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: true, unit: !2, retainedNodes: !22)
!20 = !DISubroutineType(types: !21)
!21 = !{!9, !9}
!22 = !{!23, !24, !25}
!23 = !DILocalVariable(name: "n", arg: 1, scope: !19, file: !3, line: 4, type: !9)
!24 = !DILocalVariable(name: "i", scope: !19, file: !3, line: 5, type: !9)
!25 = !DILocalVariable(name: "j", scope: !19, file: !3, line: 5, type: !9)
!26 = !DILocation(line: 4, column: 13, scope: !19)
!27 = !DILocation(line: 5, column: 7, scope: !19)
!28 = !DILocation(line: 6, column: 3, scope: !29)
!29 = distinct !DILexicalBlock(scope: !19, file: !3, line: 6, column: 3)
!30 = !DILocation(line: 5, column: 10, scope: !19)
!31 = !DILocation(line: 6, column: 24, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !3, line: 6, column: 3)
!33 = !DILocation(line: 7, column: 5, scope: !34)
!34 = distinct !DILexicalBlock(scope: !35, file: !3, line: 7, column: 5)
!35 = distinct !DILexicalBlock(scope: !32, file: !3, line: 6, column: 28)
!36 = !DILocation(line: 8, column: 17, scope: !37)
!37 = distinct !DILexicalBlock(scope: !38, file: !3, line: 7, column: 30)
!38 = distinct !DILexicalBlock(scope: !34, file: !3, line: 7, column: 5)
!39 = !{!40, !42, i64 0}
!40 = !{!"array@_ZTSA1000_A1000_i", !41, i64 0}
!41 = !{!"array@_ZTSA1000_i", !42, i64 0}
!42 = !{!"int", !43, i64 0}
!43 = !{!"omnipotent char", !44, i64 0}
!44 = !{!"Simple C/C++ TBAA"}
!45 = !DILocation(line: 8, column: 27, scope: !37)
!46 = !DILocation(line: 8, column: 25, scope: !37)
!47 = !DILocation(line: 8, column: 37, scope: !37)
!48 = !DILocation(line: 8, column: 35, scope: !37)
!49 = !DILocation(line: 8, column: 55, scope: !37)
!50 = !DILocation(line: 8, column: 49, scope: !37)
!51 = !DILocation(line: 8, column: 47, scope: !37)
!52 = !DILocation(line: 8, column: 7, scope: !37)
!53 = !DILocation(line: 8, column: 15, scope: !37)
!54 = !DILocation(line: 9, column: 10, scope: !37)
!55 = !DILocation(line: 10, column: 9, scope: !56)
!56 = distinct !DILexicalBlock(scope: !57, file: !3, line: 9, column: 16)
!57 = distinct !DILexicalBlock(scope: !37, file: !3, line: 9, column: 10)
!58 = !DILocation(line: 10, column: 21, scope: !56)
!59 = !DILocation(line: 12, column: 10, scope: !37)
!60 = !DILocation(line: 13, column: 19, scope: !61)
!61 = distinct !DILexicalBlock(scope: !62, file: !3, line: 12, column: 16)
!62 = distinct !DILexicalBlock(scope: !37, file: !3, line: 12, column: 10)
!63 = !DILocation(line: 14, column: 7, scope: !61)
!64 = !DILocation(line: 7, column: 17, scope: !38)
!65 = distinct !{!65, !33, !66}
!66 = !DILocation(line: 15, column: 5, scope: !34)
!67 = !DILocation(line: 16, column: 5, scope: !35)
!68 = !DILocation(line: 16, column: 13, scope: !35)
!69 = !DILocation(line: 6, column: 15, scope: !32)
!70 = distinct !{!70, !28, !71}
!71 = !DILocation(line: 17, column: 3, scope: !29)
!72 = !DILocation(line: 18, column: 1, scope: !19)

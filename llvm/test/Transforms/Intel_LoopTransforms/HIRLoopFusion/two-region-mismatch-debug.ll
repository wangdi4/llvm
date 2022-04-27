;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-fusion -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-loop-fusion,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT

; OPTREPORT: LOOP BEGIN at t1.c (6, 3)
; OPTREPORT:     LOOP BEGIN at t1.c (7, 5)
; OPTREPORT:         remark #25045: Fused Loops: 7,11,15
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t1.c (7, 5)
; OPTREPORT:     <Peeled loop for fusion>
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t1.c (11, 5)
; OPTREPORT:         remark #25046: Loop lost in Fusion
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t1.c (15, 5)
; OPTREPORT:         remark #25046: Loop lost in Fusion
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END

;*** Source Code ***
;int a[100];
;int b[100];
;int c[100];
;int foo(){
;  int i, j, k, n;
;  for(n = 0; n < 2; n++){
;    for(i = 1; i < 99; i++){
;      a[i] = i;
;    }
;
;    for(j = 1; j < 99; j++){
;      b[j] = a[j] + 1;
;    }
;
;    for(k = 1; k < 100; k++){
;      c[k] = b[k] + 1;
;    }
;  }
;  return 1;
;}

;Module Before HIR; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !0
@b = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !6
@c = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !12

; Function Attrs: nounwind uwtable
define i32 @foo() local_unnamed_addr #0 !dbg !19 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !26, metadata !DIExpression()), !dbg !27
  br label %for.body, !dbg !28

for.body:                                         ; preds = %for.inc25, %entry
  %n.045 = phi i32 [ 0, %entry ], [ %inc26, %for.inc25 ]
  call void @llvm.dbg.value(metadata i32 1, metadata !23, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i32 %n.045, metadata !26, metadata !DIExpression()), !dbg !27
  br label %for.body3, !dbg !31

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 1, %for.body ], [ %indvars.iv.next, %for.body3 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !23, metadata !DIExpression()), !dbg !30
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv, !dbg !35
  %0 = trunc i64 %indvars.iv to i32, !dbg !38
  store i32 %0, i32* %arrayidx, align 4, !dbg !38, !tbaa !39
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !44
  %exitcond = icmp eq i64 %indvars.iv.next, 99, !dbg !45
  br i1 %exitcond, label %for.end, label %for.body3, !dbg !31, !llvm.loop !46

for.end:                                          ; preds = %for.body3
  call void @llvm.dbg.value(metadata i32 1, metadata !24, metadata !DIExpression()), !dbg !48
  br label %for.body6, !dbg !49

for.body6:                                        ; preds = %for.body6, %for.end
  %indvars.iv46 = phi i64 [ 1, %for.end ], [ %indvars.iv.next47, %for.body6 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv46, metadata !24, metadata !DIExpression()), !dbg !48
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv46, !dbg !51
  %1 = load i32, i32* %arrayidx8, align 4, !dbg !51, !tbaa !39
  %add = add nsw i32 %1, 1, !dbg !54
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv46, !dbg !55
  store i32 %add, i32* %arrayidx10, align 4, !dbg !56, !tbaa !39
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1, !dbg !57
  %exitcond48 = icmp eq i64 %indvars.iv.next47, 99, !dbg !58
  br i1 %exitcond48, label %for.end13, label %for.body6, !dbg !49, !llvm.loop !59

for.end13:                                        ; preds = %for.body6
  call void @llvm.dbg.value(metadata i32 1, metadata !25, metadata !DIExpression()), !dbg !61
  br label %for.body16, !dbg !62

for.body16:                                       ; preds = %for.body16, %for.end13
  %indvars.iv49 = phi i64 [ 1, %for.end13 ], [ %indvars.iv.next50, %for.body16 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv49, metadata !25, metadata !DIExpression()), !dbg !61
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv49, !dbg !64
  %2 = load i32, i32* %arrayidx18, align 4, !dbg !64, !tbaa !39
  %add19 = add nsw i32 %2, 1, !dbg !67
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* @c, i64 0, i64 %indvars.iv49, !dbg !68
  store i32 %add19, i32* %arrayidx21, align 4, !dbg !69, !tbaa !39
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1, !dbg !70
  %exitcond51 = icmp eq i64 %indvars.iv.next50, 100, !dbg !71
  br i1 %exitcond51, label %for.inc25, label %for.body16, !dbg !62, !llvm.loop !72

for.inc25:                                        ; preds = %for.body16
  %inc26 = add nuw nsw i32 %n.045, 1, !dbg !74
  call void @llvm.dbg.value(metadata i32 %inc26, metadata !26, metadata !DIExpression()), !dbg !27
  %exitcond52 = icmp eq i32 %inc26, 2, !dbg !75
  br i1 %exitcond52, label %for.end27, label %for.body, !dbg !28, !llvm.loop !76

for.end27:                                        ; preds = %for.inc25
  ret i32 1, !dbg !78
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
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1f539b6d61725c1031a2d92d6dab2ae5ddee53d)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "t1.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIRLoopFusion")
!4 = !{}
!5 = !{!0, !6, !12}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 3200, elements: !10)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{!11}
!11 = !DISubrange(count: 100)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!14 = !{i32 2, !"Dwarf Version", i32 2}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"true"}
!18 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1f539b6d61725c1031a2d92d6dab2ae5ddee53d)"}
!19 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 4, type: !20, isLocal: false, isDefinition: true, scopeLine: 4, isOptimized: true, unit: !2, retainedNodes: !22)
!20 = !DISubroutineType(types: !21)
!21 = !{!9}
!22 = !{!23, !24, !25, !26}
!23 = !DILocalVariable(name: "i", scope: !19, file: !3, line: 5, type: !9)
!24 = !DILocalVariable(name: "j", scope: !19, file: !3, line: 5, type: !9)
!25 = !DILocalVariable(name: "k", scope: !19, file: !3, line: 5, type: !9)
!26 = !DILocalVariable(name: "n", scope: !19, file: !3, line: 5, type: !9)
!27 = !DILocation(line: 5, column: 16, scope: !19)
!28 = !DILocation(line: 6, column: 3, scope: !29)
!29 = distinct !DILexicalBlock(scope: !19, file: !3, line: 6, column: 3)
!30 = !DILocation(line: 5, column: 7, scope: !19)
!31 = !DILocation(line: 7, column: 5, scope: !32)
!32 = distinct !DILexicalBlock(scope: !33, file: !3, line: 7, column: 5)
!33 = distinct !DILexicalBlock(scope: !34, file: !3, line: 6, column: 25)
!34 = distinct !DILexicalBlock(scope: !29, file: !3, line: 6, column: 3)
!35 = !DILocation(line: 8, column: 7, scope: !36)
!36 = distinct !DILexicalBlock(scope: !37, file: !3, line: 7, column: 28)
!37 = distinct !DILexicalBlock(scope: !32, file: !3, line: 7, column: 5)
!38 = !DILocation(line: 8, column: 12, scope: !36)
!39 = !{!40, !41, i64 0}
!40 = !{!"array@_ZTSA100_i", !41, i64 0}
!41 = !{!"int", !42, i64 0}
!42 = !{!"omnipotent char", !43, i64 0}
!43 = !{!"Simple C/C++ TBAA"}
!44 = !DILocation(line: 7, column: 25, scope: !37)
!45 = !DILocation(line: 7, column: 18, scope: !37)
!46 = distinct !{!46, !31, !47}
!47 = !DILocation(line: 9, column: 5, scope: !32)
!48 = !DILocation(line: 5, column: 10, scope: !19)
!49 = !DILocation(line: 11, column: 5, scope: !50)
!50 = distinct !DILexicalBlock(scope: !33, file: !3, line: 11, column: 5)
!51 = !DILocation(line: 12, column: 14, scope: !52)
!52 = distinct !DILexicalBlock(scope: !53, file: !3, line: 11, column: 28)
!53 = distinct !DILexicalBlock(scope: !50, file: !3, line: 11, column: 5)
!54 = !DILocation(line: 12, column: 19, scope: !52)
!55 = !DILocation(line: 12, column: 7, scope: !52)
!56 = !DILocation(line: 12, column: 12, scope: !52)
!57 = !DILocation(line: 11, column: 25, scope: !53)
!58 = !DILocation(line: 11, column: 18, scope: !53)
!59 = distinct !{!59, !49, !60}
!60 = !DILocation(line: 13, column: 5, scope: !50)
!61 = !DILocation(line: 5, column: 13, scope: !19)
!62 = !DILocation(line: 15, column: 5, scope: !63)
!63 = distinct !DILexicalBlock(scope: !33, file: !3, line: 15, column: 5)
!64 = !DILocation(line: 16, column: 14, scope: !65)
!65 = distinct !DILexicalBlock(scope: !66, file: !3, line: 15, column: 29)
!66 = distinct !DILexicalBlock(scope: !63, file: !3, line: 15, column: 5)
!67 = !DILocation(line: 16, column: 19, scope: !65)
!68 = !DILocation(line: 16, column: 7, scope: !65)
!69 = !DILocation(line: 16, column: 12, scope: !65)
!70 = !DILocation(line: 15, column: 26, scope: !66)
!71 = !DILocation(line: 15, column: 18, scope: !66)
!72 = distinct !{!72, !62, !73}
!73 = !DILocation(line: 17, column: 5, scope: !63)
!74 = !DILocation(line: 6, column: 22, scope: !34)
!75 = !DILocation(line: 6, column: 16, scope: !34)
!76 = distinct !{!76, !28, !77}
!77 = !DILocation(line: 18, column: 3, scope: !29)
!78 = !DILocation(line: 19, column: 3, scope: !19)

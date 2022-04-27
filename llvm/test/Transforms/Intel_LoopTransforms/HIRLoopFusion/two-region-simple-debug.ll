;RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-loop-fusion -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT
;RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-loop-fusion,hir-cg,simplifycfg,intel-ir-optreport-emitter" -aa-pipeline="basic-aa" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s  -check-prefix=OPTREPORT

;*** Souce Code ***
;int a[100];
;int b[100];
;int c[100];
;int d[100];
;int main(){
;  int i, j, k, l, n;
;  for(n = 0; n < 2; n++){
;    for(i = 0; i < 100; i++){
;      a[i] = i;
;    }
;
;    for(j = 0; j < 100; j++){
;      b[j] = a[j] + 1;
;    }
;
;    for(k = 0; k < 100; k++){
;      c[k] = b[k] + 1;
;    }
;
;    for(l = 0; l < 100; l++){
;      d[l] = c[l] + 1;
;    }
;
;  }
;  return 1 ;
;}

; OPTREPORT: LOOP BEGIN at t.c (7, 3)
; OPTREPORT:     LOOP BEGIN at t.c (8, 5)
; OPTREPORT:         remark #25045: Fused Loops: 8,12,16,20
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t.c (12, 5)
; OPTREPORT:         remark #25046: Loop lost in Fusion
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t.c (16, 5)
; OPTREPORT:         remark #25046: Loop lost in Fusion
; OPTREPORT:     LOOP END
; OPTREPORT:     LOOP BEGIN at t.c (20, 5)
; OPTREPORT:         remark #25046: Loop lost in Fusion
; OPTREPORT:     LOOP END
; OPTREPORT: LOOP END


;Module Before HIR; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !0
@b = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !6
@c = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !12
@d = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16, !dbg !14

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 !dbg !21 {
entry:
  call void @llvm.dbg.value(metadata i32 0, metadata !29, metadata !DIExpression()), !dbg !30
  br label %for.body, !dbg !31

for.body:                                         ; preds = %for.inc36, %entry
  %n.061 = phi i32 [ 0, %entry ], [ %inc37, %for.inc36 ]
  call void @llvm.dbg.value(metadata i32 0, metadata !25, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32 %n.061, metadata !29, metadata !DIExpression()), !dbg !30
  br label %for.body3, !dbg !34

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !25, metadata !DIExpression()), !dbg !33
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv, !dbg !38
  %0 = trunc i64 %indvars.iv to i32, !dbg !41
  store i32 %0, i32* %arrayidx, align 4, !dbg !41, !tbaa !42
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !47
  %exitcond = icmp eq i64 %indvars.iv.next, 100, !dbg !48
  br i1 %exitcond, label %for.end, label %for.body3, !dbg !34, !llvm.loop !49

for.end:                                          ; preds = %for.body3
  call void @llvm.dbg.value(metadata i32 0, metadata !26, metadata !DIExpression()), !dbg !51
  br label %for.body6, !dbg !52

for.body6:                                        ; preds = %for.body6, %for.end
  %indvars.iv62 = phi i64 [ 0, %for.end ], [ %indvars.iv.next63, %for.body6 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv62, metadata !26, metadata !DIExpression()), !dbg !51
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %indvars.iv62, !dbg !54
  %1 = load i32, i32* %arrayidx8, align 4, !dbg !54, !tbaa !42
  %add = add nsw i32 %1, 1, !dbg !57
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv62, !dbg !58
  store i32 %add, i32* %arrayidx10, align 4, !dbg !59, !tbaa !42
  %indvars.iv.next63 = add nuw nsw i64 %indvars.iv62, 1, !dbg !60
  %exitcond64 = icmp eq i64 %indvars.iv.next63, 100, !dbg !61
  br i1 %exitcond64, label %for.end13, label %for.body6, !dbg !52, !llvm.loop !62

for.end13:                                        ; preds = %for.body6
  call void @llvm.dbg.value(metadata i32 0, metadata !27, metadata !DIExpression()), !dbg !64
  br label %for.body16, !dbg !65

for.body16:                                       ; preds = %for.body16, %for.end13
  %indvars.iv65 = phi i64 [ 0, %for.end13 ], [ %indvars.iv.next66, %for.body16 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv65, metadata !27, metadata !DIExpression()), !dbg !64
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %indvars.iv65, !dbg !67
  %2 = load i32, i32* %arrayidx18, align 4, !dbg !67, !tbaa !42
  %add19 = add nsw i32 %2, 1, !dbg !70
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* @c, i64 0, i64 %indvars.iv65, !dbg !71
  store i32 %add19, i32* %arrayidx21, align 4, !dbg !72, !tbaa !42
  %indvars.iv.next66 = add nuw nsw i64 %indvars.iv65, 1, !dbg !73
  %exitcond67 = icmp eq i64 %indvars.iv.next66, 100, !dbg !74
  br i1 %exitcond67, label %for.end24, label %for.body16, !dbg !65, !llvm.loop !75

for.end24:                                        ; preds = %for.body16
  call void @llvm.dbg.value(metadata i32 0, metadata !28, metadata !DIExpression()), !dbg !77
  br label %for.body27, !dbg !78

for.body27:                                       ; preds = %for.body27, %for.end24
  %indvars.iv68 = phi i64 [ 0, %for.end24 ], [ %indvars.iv.next69, %for.body27 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv68, metadata !28, metadata !DIExpression()), !dbg !77
  %arrayidx29 = getelementptr inbounds [100 x i32], [100 x i32]* @c, i64 0, i64 %indvars.iv68, !dbg !80
  %3 = load i32, i32* %arrayidx29, align 4, !dbg !80, !tbaa !42
  %add30 = add nsw i32 %3, 1, !dbg !83
  %arrayidx32 = getelementptr inbounds [100 x i32], [100 x i32]* @d, i64 0, i64 %indvars.iv68, !dbg !84
  store i32 %add30, i32* %arrayidx32, align 4, !dbg !85, !tbaa !42
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1, !dbg !86
  %exitcond70 = icmp eq i64 %indvars.iv.next69, 100, !dbg !87
  br i1 %exitcond70, label %for.inc36, label %for.body27, !dbg !78, !llvm.loop !88

for.inc36:                                        ; preds = %for.body27
  %inc37 = add nuw nsw i32 %n.061, 1, !dbg !90
  call void @llvm.dbg.value(metadata i32 %inc37, metadata !29, metadata !DIExpression()), !dbg !30
  %exitcond71 = icmp eq i32 %inc37, 2, !dbg !91
  br i1 %exitcond71, label %for.end38, label %for.body, !dbg !31, !llvm.loop !92

for.end38:                                        ; preds = %for.inc36
  ret i32 1, !dbg !94
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!16, !17, !18}
!llvm.dbg.intel.emit_class_debug_always = !{!19}
!llvm.ident = !{!20}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C89, file: !3, producer: "clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1f539b6d61725c1031a2d92d6dab2ae5ddee53d)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "t.c", directory: "/export/iusers/linayu/opt_report/llvm/test/Transforms/Intel_LoopTransforms/HIRLoopFusion")
!4 = !{}
!5 = !{!0, !6, !12, !14}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 2, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 3200, elements: !10)
!9 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!10 = !{!11}
!11 = !DISubrange(count: 100)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!14 = !DIGlobalVariableExpression(var: !15, expr: !DIExpression())
!15 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 4, type: !8, isLocal: false, isDefinition: true)
!16 = !{i32 2, !"Dwarf Version", i32 2}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 1, !"wchar_size", i32 4}
!19 = !{!"true"}
!20 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 6a026a7944d2244cc728fa0e8328c8ce3bc0d72c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1f539b6d61725c1031a2d92d6dab2ae5ddee53d)"}
!21 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 5, type: !22, isLocal: false, isDefinition: true, scopeLine: 5, isOptimized: true, unit: !2, retainedNodes: !24)
!22 = !DISubroutineType(types: !23)
!23 = !{!9}
!24 = !{!25, !26, !27, !28, !29}
!25 = !DILocalVariable(name: "i", scope: !21, file: !3, line: 6, type: !9)
!26 = !DILocalVariable(name: "j", scope: !21, file: !3, line: 6, type: !9)
!27 = !DILocalVariable(name: "k", scope: !21, file: !3, line: 6, type: !9)
!28 = !DILocalVariable(name: "l", scope: !21, file: !3, line: 6, type: !9)
!29 = !DILocalVariable(name: "n", scope: !21, file: !3, line: 6, type: !9)
!30 = !DILocation(line: 6, column: 19, scope: !21)
!31 = !DILocation(line: 7, column: 3, scope: !32)
!32 = distinct !DILexicalBlock(scope: !21, file: !3, line: 7, column: 3)
!33 = !DILocation(line: 6, column: 7, scope: !21)
!34 = !DILocation(line: 8, column: 5, scope: !35)
!35 = distinct !DILexicalBlock(scope: !36, file: !3, line: 8, column: 5)
!36 = distinct !DILexicalBlock(scope: !37, file: !3, line: 7, column: 25)
!37 = distinct !DILexicalBlock(scope: !32, file: !3, line: 7, column: 3)
!38 = !DILocation(line: 9, column: 7, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !3, line: 8, column: 29)
!40 = distinct !DILexicalBlock(scope: !35, file: !3, line: 8, column: 5)
!41 = !DILocation(line: 9, column: 12, scope: !39)
!42 = !{!43, !44, i64 0}
!43 = !{!"array@_ZTSA100_i", !44, i64 0}
!44 = !{!"int", !45, i64 0}
!45 = !{!"omnipotent char", !46, i64 0}
!46 = !{!"Simple C/C++ TBAA"}
!47 = !DILocation(line: 8, column: 26, scope: !40)
!48 = !DILocation(line: 8, column: 18, scope: !40)
!49 = distinct !{!49, !34, !50}
!50 = !DILocation(line: 10, column: 5, scope: !35)
!51 = !DILocation(line: 6, column: 10, scope: !21)
!52 = !DILocation(line: 12, column: 5, scope: !53)
!53 = distinct !DILexicalBlock(scope: !36, file: !3, line: 12, column: 5)
!54 = !DILocation(line: 13, column: 14, scope: !55)
!55 = distinct !DILexicalBlock(scope: !56, file: !3, line: 12, column: 29)
!56 = distinct !DILexicalBlock(scope: !53, file: !3, line: 12, column: 5)
!57 = !DILocation(line: 13, column: 19, scope: !55)
!58 = !DILocation(line: 13, column: 7, scope: !55)
!59 = !DILocation(line: 13, column: 12, scope: !55)
!60 = !DILocation(line: 12, column: 26, scope: !56)
!61 = !DILocation(line: 12, column: 18, scope: !56)
!62 = distinct !{!62, !52, !63}
!63 = !DILocation(line: 14, column: 5, scope: !53)
!64 = !DILocation(line: 6, column: 13, scope: !21)
!65 = !DILocation(line: 16, column: 5, scope: !66)
!66 = distinct !DILexicalBlock(scope: !36, file: !3, line: 16, column: 5)
!67 = !DILocation(line: 17, column: 14, scope: !68)
!68 = distinct !DILexicalBlock(scope: !69, file: !3, line: 16, column: 29)
!69 = distinct !DILexicalBlock(scope: !66, file: !3, line: 16, column: 5)
!70 = !DILocation(line: 17, column: 19, scope: !68)
!71 = !DILocation(line: 17, column: 7, scope: !68)
!72 = !DILocation(line: 17, column: 12, scope: !68)
!73 = !DILocation(line: 16, column: 26, scope: !69)
!74 = !DILocation(line: 16, column: 18, scope: !69)
!75 = distinct !{!75, !65, !76}
!76 = !DILocation(line: 18, column: 5, scope: !66)
!77 = !DILocation(line: 6, column: 16, scope: !21)
!78 = !DILocation(line: 20, column: 5, scope: !79)
!79 = distinct !DILexicalBlock(scope: !36, file: !3, line: 20, column: 5)
!80 = !DILocation(line: 21, column: 14, scope: !81)
!81 = distinct !DILexicalBlock(scope: !82, file: !3, line: 20, column: 29)
!82 = distinct !DILexicalBlock(scope: !79, file: !3, line: 20, column: 5)
!83 = !DILocation(line: 21, column: 19, scope: !81)
!84 = !DILocation(line: 21, column: 7, scope: !81)
!85 = !DILocation(line: 21, column: 12, scope: !81)
!86 = !DILocation(line: 20, column: 26, scope: !82)
!87 = !DILocation(line: 20, column: 18, scope: !82)
!88 = distinct !{!88, !78, !89}
!89 = !DILocation(line: 22, column: 5, scope: !79)
!90 = !DILocation(line: 7, column: 22, scope: !37)
!91 = !DILocation(line: 7, column: 16, scope: !37)
!92 = distinct !{!92, !31, !93}
!93 = !DILocation(line: 24, column: 3, scope: !32)
!94 = !DILocation(line: 25, column: 3, scope: !21)

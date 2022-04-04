; RUN: opt %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-cg -intel-ir-optreport-emitter -intel-opt-report=medium -force-hir-cg -print-before=hir-vec-dir-insert -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>,hir-vec-dir-insert,hir-cg,intel-ir-optreport-emitter" -intel-opt-report=medium -force-hir-cg -aa-pipeline=basic-aa -disable-output 2>&1 | FileCheck %s

; Check that Opt Report prints out relevant information regarding why vectorization
; does not happen. Note that p1 is a global array, and t1 is represented as a
; lval terminal ref (special case).

; TODO: Add missing debugloc for Refs (t1)

;CHECK:       BEGIN REGION { }
;CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%n) + -1, 1
;CHECK-NEXT:        |   (@p1)[0][%t1.022] = %t1.022;
;CHECK-NEXT:        |   %1 = (@p1)[0][i1];
;CHECK-NEXT:        |   %2 = (%M)[i1];
;CHECK-NEXT:        |   (%p2)[sext.i32.i64(%1) + sext.i32.i64(%t1.022)] = %2;
;CHECK-NEXT:        |   %t1.022 = i1 + 1;
;CHECK-NEXT:        + END LOOP
;CHECK-NEXT:  END REGION

; CHECK:      LOOP BEGIN at test2.c (11, 2)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test2.c (11, 2)
; CHECK-NEXT: remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT: remark #15346: vector dependence: assumed ANTI dependence between p1 (13:9) and p1 (12:10)
; CHECK-NEXT: remark #15346: vector dependence: assumed ANTI dependence between M (14:12) and p1 (12:10)
; CHECK-NEXT: remark #15346: vector dependence: assumed OUTPUT dependence between p2 (14:10) and p1 (12:10)
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between p2 (14:10) and p1 (13:9)
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between p2 (14:10) and M (14:12)
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between t1 and t1
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between t1 and t1 (12:10)
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between t1 and t1 (14:10)
; CHECK-NEXT:  LOOP END

@p1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16, !dbg !0

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local i32 @sub(i32* nocapture noundef writeonly %p2, i32 noundef %n, i32* nocapture noundef readonly %M) local_unnamed_addr #0 !dbg !14 {
entry:
  call void @llvm.dbg.value(metadata i32* %p2, metadata !19, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 %n, metadata !20, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32* %M, metadata !21, metadata !DIExpression()), !dbg !26
  %0 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @p1, i64 0, i64 0), align 16, !dbg !27, !tbaa !28
  call void @llvm.dbg.value(metadata i32 %0, metadata !22, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 0, metadata !23, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !32
  %cmp21 = icmp sgt i32 %n, 0, !dbg !33
  br i1 %cmp21, label %for.body.preheader, label %for.cond.cleanup, !dbg !35

for.body.preheader:                               ; preds = %entry
  %wide.trip.count24 = zext i32 %n to i64, !dbg !33
  br label %for.body, !dbg !35

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup, !dbg !36

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %t1.0.lcssa = phi i32 [ %0, %entry ], [ %n, %for.cond.cleanup.loopexit ]
  call void @llvm.dbg.value(metadata i32 %t1.0.lcssa, metadata !23, metadata !DIExpression()), !dbg !26
  ret i32 %t1.0.lcssa, !dbg !36

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %t1.022 = phi i32 [ %0, %for.body.preheader ], [ %3, %for.body ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !24, metadata !DIExpression()), !dbg !32
  call void @llvm.dbg.value(metadata i32 %t1.022, metadata !22, metadata !DIExpression()), !dbg !26
  %idxprom = sext i32 %t1.022 to i64, !dbg !37
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @p1, i64 0, i64 %idxprom, !dbg !37, !intel-tbaa !39
  store i32 %t1.022, i32* %arrayidx, align 4, !dbg !41, !tbaa !39
  %arrayidx2 = getelementptr inbounds [10 x i32], [10 x i32]* @p1, i64 0, i64 %indvars.iv, !dbg !42, !intel-tbaa !39
  %1 = load i32, i32* %arrayidx2, align 4, !dbg !42, !tbaa !39
  %add = add nsw i32 %1, %t1.022, !dbg !43
  call void @llvm.dbg.value(metadata i32 %add, metadata !22, metadata !DIExpression()), !dbg !26
  %arrayidx4 = getelementptr inbounds i32, i32* %M, i64 %indvars.iv, !dbg !44
  %2 = load i32, i32* %arrayidx4, align 4, !dbg !44, !tbaa !28
  %idxprom5 = sext i32 %add to i64, !dbg !45
  %arrayidx6 = getelementptr inbounds i32, i32* %p2, i64 %idxprom5, !dbg !45
  store i32 %2, i32* %arrayidx6, align 4, !dbg !46, !tbaa !28
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !47
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !24, metadata !DIExpression()), !dbg !32
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !22, metadata !DIExpression()), !dbg !26
  %3 = trunc i64 %indvars.iv.next to i32, !dbg !35
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count24, !dbg !33
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !dbg !35, !llvm.loop !48
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "p1", scope: !2, file: !3, line: 1, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -c -x CORE-AVX2 -g -Ofast -mllvm -print-module-before-loopopt test2.c -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "test2.c", directory: "/localdisk2/liuchen3/ws/xmain1")
!4 = !{!0}
!5 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 320, elements: !7)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!8}
!8 = !DISubrange(count: 10)
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"uwtable", i32 1}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!14 = distinct !DISubprogram(name: "sub", scope: !3, file: !3, line: 3, type: !15, scopeLine: 3, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !18)
!15 = !DISubroutineType(types: !16)
!16 = !{!6, !17, !6, !17}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64)
!18 = !{!19, !20, !21, !22, !23, !24}
!19 = !DILocalVariable(name: "p2", arg: 1, scope: !14, file: !3, line: 3, type: !17)
!20 = !DILocalVariable(name: "n", arg: 2, scope: !14, file: !3, line: 3, type: !6)
!21 = !DILocalVariable(name: "M", arg: 3, scope: !14, file: !3, line: 3, type: !17)
!22 = !DILocalVariable(name: "t1", scope: !14, file: !3, line: 5, type: !6)
!23 = !DILocalVariable(name: "sum", scope: !14, file: !3, line: 6, type: !6)
!24 = !DILocalVariable(name: "i", scope: !25, file: !3, line: 11, type: !6)
!25 = distinct !DILexicalBlock(scope: !14, file: !3, line: 11, column: 2)
!26 = !DILocation(line: 0, scope: !14)
!27 = !DILocation(line: 5, column: 11, scope: !14)
!28 = !{!29, !29, i64 0}
!29 = !{!"int", !30, i64 0}
!30 = !{!"omnipotent char", !31, i64 0}
!31 = !{!"Simple C/C++ TBAA"}
!32 = !DILocation(line: 0, scope: !25)
!33 = !DILocation(line: 11, column: 17, scope: !34)
!34 = distinct !DILexicalBlock(scope: !25, file: !3, line: 11, column: 2)
!35 = !DILocation(line: 11, column: 2, scope: !25)
!36 = !DILocation(line: 20, column: 2, scope: !14)
!37 = !DILocation(line: 12, column: 3, scope: !38)
!38 = distinct !DILexicalBlock(scope: !34, file: !3, line: 11, column: 27)
!39 = !{!40, !29, i64 0}
!40 = !{!"array@_ZTSA10_i", !29, i64 0}
!41 = !DILocation(line: 12, column: 10, scope: !38)
!42 = !DILocation(line: 13, column: 9, scope: !38)
!43 = !DILocation(line: 13, column: 6, scope: !38)
!44 = !DILocation(line: 14, column: 12, scope: !38)
!45 = !DILocation(line: 14, column: 3, scope: !38)
!46 = !DILocation(line: 14, column: 10, scope: !38)
!47 = !DILocation(line: 15, column: 9, scope: !38)
!48 = distinct !{!48, !35, !49, !50}
!49 = !DILocation(line: 16, column: 2, scope: !25)
!50 = !{!"llvm.loop.mustprogress"}

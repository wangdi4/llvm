; RUN: opt %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-cg -intel-ir-optreport-emitter -intel-opt-report=medium -force-hir-cg -print-before=hir-vec-dir-insert -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>,hir-vec-dir-insert,hir-cg,intel-ir-optreport-emitter" -intel-opt-report=medium -force-hir-cg -aa-pipeline=basic-aa -disable-output 2>&1 | FileCheck %s

; Check that Optreport successfully displays debug info for source variables
; that are represented as HIR BlobDDRefs. In this case, it is for t1, which is
; used in the ref (%p1)[%t1.016]

; CHECK:        BEGIN REGION { }
; CHECK-NEXT:         + DO i1 = 0, zext.i32.i64(%n) + -1, 1
; CHECK-NEXT:         |   (%p1)[%t1.016] = %t1.016;
; CHECK-NEXT:         |   %1 = (%p1)[i1];
; CHECK-NEXT:         |   %t1.016 = %1  +  %t1.016;
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:   END REGION

;CHECK:       LOOP BEGIN at test2.c (9, 2)
;CHECK:       LOOP END

; CHECK:      LOOP BEGIN at test2.c (9, 2)
; CHECK-NEXT:     remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT:     remark #15346: vector dependence: assumed ANTI dependence between p1 (11:23) and p1 (11:11)
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence between t1 (11:20) and t1
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence between t1 (11:20) and t1 (11:11)
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence between t1 (11:20) and t1
; CHECK-NEXT: LOOP END


define dso_local i32 @sub(i32* nocapture noundef %p1, i32* nocapture noundef readnone %p2, i32 noundef %n, i32* nocapture noundef readnone %M) local_unnamed_addr #0 !dbg !7 {
entry:
  call void @llvm.dbg.value(metadata i32* %p1, metadata !13, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i32* %p2, metadata !14, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i32 %n, metadata !15, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i32* %M, metadata !16, metadata !DIExpression()), !dbg !21
  %0 = load i32, i32* %p1, align 4, !dbg !22, !tbaa !23
  call void @llvm.dbg.value(metadata i32 %0, metadata !17, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.value(metadata i32 0, metadata !19, metadata !DIExpression()), !dbg !27
  %cmp15 = icmp sgt i32 %n, 0, !dbg !28
  br i1 %cmp15, label %for.body.preheader, label %for.cond.cleanup, !dbg !30

for.body.preheader:                               ; preds = %entry
  %wide.trip.count18 = zext i32 %n to i64, !dbg !28
  br label %for.body, !dbg !30

for.cond.cleanup.loopexit:                        ; preds = %for.body
  %add.lcssa = phi i32 [ %add, %for.body ], !dbg !31
  br label %for.cond.cleanup, !dbg !33

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %t1.0.lcssa = phi i32 [ %0, %entry ], [ %add.lcssa, %for.cond.cleanup.loopexit ]
  call void @llvm.dbg.value(metadata i32 %t1.0.lcssa, metadata !18, metadata !DIExpression()), !dbg !21
  ret i32 %t1.0.lcssa, !dbg !33

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %t1.016 = phi i32 [ %0, %for.body.preheader ], [ %add, %for.body ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !19, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i32 %t1.016, metadata !17, metadata !DIExpression()), !dbg !21
  %idxprom = sext i32 %t1.016 to i64, !dbg !34
  %arrayidx = getelementptr inbounds i32, i32* %p1, i64 %idxprom, !dbg !34
  store i32 %t1.016, i32* %arrayidx, align 4, !dbg !35, !tbaa !23
  %arrayidx2 = getelementptr inbounds i32, i32* %p1, i64 %indvars.iv, !dbg !36
  %1 = load i32, i32* %arrayidx2, align 4, !dbg !36, !tbaa !23
  %add = add nsw i32 %1, %t1.016, !dbg !31
  call void @llvm.dbg.value(metadata i32 %add, metadata !17, metadata !DIExpression()), !dbg !21
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !37
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !19, metadata !DIExpression()), !dbg !27
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count18, !dbg !28
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !dbg !30, !llvm.loop !38
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -c -x CORE-AVX2 -g -Ofast -mllvm -print-module-before-loopopt test2.c -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test2.c", directory: "/localdisk2/liuchen3/ws/xmain1")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"uwtable", i32 1}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!7 = distinct !DISubprogram(name: "sub", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !12)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!12 = !{!13, !14, !15, !16, !17, !18, !19}
!13 = !DILocalVariable(name: "p1", arg: 1, scope: !7, file: !1, line: 1, type: !11)
!14 = !DILocalVariable(name: "p2", arg: 2, scope: !7, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "n", arg: 3, scope: !7, file: !1, line: 1, type: !10)
!16 = !DILocalVariable(name: "M", arg: 4, scope: !7, file: !1, line: 1, type: !11)
!17 = !DILocalVariable(name: "t1", scope: !7, file: !1, line: 3, type: !10)
!18 = !DILocalVariable(name: "sum", scope: !7, file: !1, line: 4, type: !10)
!19 = !DILocalVariable(name: "i", scope: !20, file: !1, line: 9, type: !10)
!20 = distinct !DILexicalBlock(scope: !7, file: !1, line: 9, column: 2)
!21 = !DILocation(line: 0, scope: !7)
!22 = !DILocation(line: 3, column: 11, scope: !7)
!23 = !{!24, !24, i64 0}
!24 = !{!"int", !25, i64 0}
!25 = !{!"omnipotent char", !26, i64 0}
!26 = !{!"Simple C/C++ TBAA"}
!27 = !DILocation(line: 0, scope: !20)
!28 = !DILocation(line: 9, column: 17, scope: !29)
!29 = distinct !DILexicalBlock(scope: !20, file: !1, line: 9, column: 2)
!30 = !DILocation(line: 9, column: 2, scope: !20)
!31 = !DILocation(line: 11, column: 20, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !1, line: 11, column: 2)
!33 = !DILocation(line: 15, column: 2, scope: !7)
!34 = !DILocation(line: 11, column: 4, scope: !32)
!35 = !DILocation(line: 11, column: 11, scope: !32)
!36 = !DILocation(line: 11, column: 23, scope: !32)
!37 = !DILocation(line: 9, column: 23, scope: !29)
!38 = distinct !{!38, !30, !39, !40}
!39 = !DILocation(line: 11, column: 30, scope: !20)
!40 = !{!"llvm.loop.mustprogress"}

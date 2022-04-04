; RUN: opt %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-cg -intel-ir-optreport-emitter -intel-opt-report=medium -force-hir-cg -print-before=hir-vec-dir-insert -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir>,hir-vec-dir-insert,hir-cg,intel-ir-optreport-emitter" -intel-opt-report=medium -force-hir-cg -aa-pipeline=basic-aa -disable-output 2>&1 | FileCheck %s

; Check that Opt Report prints out relevant information regarding why vectorization
; does not happen. Note that p2 and t1 are global refs, M is parameter, and p1 is an alloca memref.

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, zext.i32.i64((2 * %n)) + -1, 1   <DO_LOOP>
; CHECK-NEXT:       |   %3 = (%M)[i1];
; CHECK-NEXT:       |   %4 = (%2)[%3];
; CHECK-NEXT:       |   %5 = (%p3)[i1];
; CHECK-NEXT:       |   %add = %5  +  %4;
; CHECK-NEXT:       |   (%p3)[i1] = %add;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>
; CHECK-NEXT:       |   %t1.047 = %t1.047  *  2.000000e+00;
; CHECK-NEXT:       |   %7 = (%p1)[0][i1 + -1];
; CHECK-NEXT:       |   %add17 = %7  +  %conv;
; CHECK-NEXT:       |   (%p1)[0][i1] = %add17;
; CHECK-NEXT:       |   %add22 = %sum.048  +  %t1.047;
; CHECK-NEXT:       |   %sum.048 = %add22  +  %add17;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; CHECK:      LOOP BEGIN at test.c (7, 2)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.c (7, 2)
; CHECK-NEXT: remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between p3 (9:10) and M (9:16)
; CHECK-NEXT: remark #15346: vector dependence: assumed FLOW dependence between p3 (9:10) and p2 (9:13)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.c (10, 2)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.c (10, 2)
; CHECK-NEXT:    remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT:    remark #15346: vector dependence: assumed FLOW dependence between t1 and t1
; CHECK-NEXT:    remark #15346: vector dependence: assumed FLOW dependence between p1 (14:8) and p1 (14:10)
; CHECK-NEXT: LOOP END


@p2 = dso_local local_unnamed_addr global float* null, align 8, !dbg !0
@t1 = dso_local local_unnamed_addr global float 0.000000e+00, align 4, !dbg !5

; Function Attrs: nofree nosync nounwind uwtable
define dso_local float @sub(float* nocapture noundef %p3, i32 noundef %n, i32* nocapture noundef readonly %M) local_unnamed_addr #0 !dbg !14 {
entry:
  %p1 = alloca [100 x float], align 16
  call void @llvm.dbg.value(metadata float* %p3, metadata !20, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32 %n, metadata !21, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32* %M, metadata !22, metadata !DIExpression()), !dbg !33
  %0 = bitcast [100 x float]* %p1 to i8*, !dbg !34
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %0) #3, !dbg !34
  call void @llvm.dbg.declare(metadata [100 x float]* %p1, metadata !23, metadata !DIExpression()), !dbg !35
  %1 = load float, float* %p3, align 4, !dbg !36, !tbaa !37
  call void @llvm.dbg.value(metadata float %1, metadata !27, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata float 0.000000e+00, metadata !28, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, metadata !29, metadata !DIExpression()), !dbg !41
  %mul = shl nsw i32 %n, 1, !dbg !42
  %2 = load float*, float** @p2, align 8, !dbg !44
  call void @llvm.dbg.value(metadata i32 0, metadata !29, metadata !DIExpression()), !dbg !41
  %cmp50 = icmp sgt i32 %mul, 0, !dbg !46
  br i1 %cmp50, label %for.body.preheader, label %for.cond6.preheader, !dbg !47

for.body.preheader:                               ; preds = %entry
  %wide.trip.count5557 = zext i32 %mul to i64, !dbg !46
  br label %for.body, !dbg !47

for.cond6.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond6.preheader, !dbg !48

for.cond6.preheader:                              ; preds = %for.cond6.preheader.loopexit, %entry
  %conv = sitofp i32 %n to float, !dbg !48
  call void @llvm.dbg.value(metadata i32 0, metadata !31, metadata !DIExpression()), !dbg !51
  call void @llvm.dbg.value(metadata float 0.000000e+00, metadata !28, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata float %1, metadata !27, metadata !DIExpression()), !dbg !33
  %cmp746 = icmp sgt i32 %n, 0, !dbg !52
  br i1 %cmp746, label %for.body9.preheader, label %for.cond.cleanup8, !dbg !53

for.body9.preheader:                              ; preds = %for.cond6.preheader
  %wide.trip.count58 = zext i32 %n to i64, !dbg !52
  br label %for.body9, !dbg !53

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv53 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next54, %for.body ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv53, metadata !29, metadata !DIExpression()), !dbg !41
  %arrayidx = getelementptr inbounds i32, i32* %M, i64 %indvars.iv53, !dbg !54
  %3 = load i32, i32* %arrayidx, align 4, !dbg !54, !tbaa !55
  %idxprom1 = sext i32 %3 to i64, !dbg !44
  %arrayidx2 = getelementptr inbounds float, float* %2, i64 %idxprom1, !dbg !44
  %4 = load float, float* %arrayidx2, align 4, !dbg !44, !tbaa !37
  %arrayidx4 = getelementptr inbounds float, float* %p3, i64 %indvars.iv53, !dbg !57
  %5 = load float, float* %arrayidx4, align 4, !dbg !58, !tbaa !37
  %add = fadd fast float %5, %4, !dbg !58
  store float %add, float* %arrayidx4, align 4, !dbg !58, !tbaa !37
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1, !dbg !59
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next54, metadata !29, metadata !DIExpression()), !dbg !41
  %exitcond56.not = icmp eq i64 %indvars.iv.next54, %wide.trip.count5557, !dbg !46
  br i1 %exitcond56.not, label %for.cond6.preheader.loopexit, label %for.body, !dbg !47, !llvm.loop !60

for.cond.cleanup8.loopexit:                       ; preds = %for.body9
  %add23.lcssa = phi float [ %add23, %for.body9 ], !dbg !63
  br label %for.cond.cleanup8, !dbg !64

for.cond.cleanup8:                                ; preds = %for.cond.cleanup8.loopexit, %for.cond6.preheader
  %sum.0.lcssa = phi float [ 0.000000e+00, %for.cond6.preheader ], [ %add23.lcssa, %for.cond.cleanup8.loopexit ]
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %0) #3, !dbg !64
  ret float %sum.0.lcssa, !dbg !65

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv = phi i64 [ 0, %for.body9.preheader ], [ %indvars.iv.next, %for.body9 ]
  %sum.048 = phi float [ 0.000000e+00, %for.body9.preheader ], [ %add23, %for.body9 ]
  %t1.047 = phi float [ %1, %for.body9.preheader ], [ %factor, %for.body9 ]
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !31, metadata !DIExpression()), !dbg !51
  call void @llvm.dbg.value(metadata float %sum.048, metadata !28, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata float %t1.047, metadata !27, metadata !DIExpression()), !dbg !33
  %arrayidx11 = getelementptr inbounds [100 x float], [100 x float]* %p1, i64 0, i64 %indvars.iv, !dbg !66, !intel-tbaa !67
  %factor = fmul fast float %t1.047, 2.000000e+00, !dbg !69
  call void @llvm.dbg.value(metadata float %factor, metadata !27, metadata !DIExpression()), !dbg !33
  %6 = add nsw i64 %indvars.iv, -1, !dbg !70
  %arrayidx16 = getelementptr inbounds [100 x float], [100 x float]* %p1, i64 0, i64 %6, !dbg !71, !intel-tbaa !67
  %7 = load float, float* %arrayidx16, align 4, !dbg !71, !tbaa !67
  %add17 = fadd fast float %7, %conv, !dbg !72
  store float %add17, float* %arrayidx11, align 4, !dbg !73, !tbaa !67
  %add22 = fadd fast float %sum.048, %factor, !dbg !74
  %add23 = fadd fast float %add22, %add17, !dbg !63
  call void @llvm.dbg.value(metadata float %add23, metadata !28, metadata !DIExpression()), !dbg !33
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !75
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !31, metadata !DIExpression()), !dbg !51
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count58, !dbg !52
  br i1 %exitcond.not, label %for.cond.cleanup8.loopexit, label %for.body9, !dbg !53, !llvm.loop !76
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "p2", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, flags: " --intel -c -x CORE-AVX2 -g -Ofast -mllvm -print-module-before-loopopt test.c -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "test.c", directory: "/localdisk2/liuchen3/ws/xmain1")
!4 = !{!5, !0}
!5 = !DIGlobalVariableExpression(var: !6, expr: !DIExpression())
!6 = distinct !DIGlobalVariable(name: "t1", scope: !2, file: !3, line: 1, type: !7, isLocal: false, isDefinition: true)
!7 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"uwtable", i32 1}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!14 = distinct !DISubprogram(name: "sub", scope: !3, file: !3, line: 2, type: !15, scopeLine: 2, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !19)
!15 = !DISubroutineType(types: !16)
!16 = !{!7, !8, !17, !18}
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!19 = !{!20, !21, !22, !23, !27, !28, !29, !31}
!20 = !DILocalVariable(name: "p3", arg: 1, scope: !14, file: !3, line: 2, type: !8)
!21 = !DILocalVariable(name: "n", arg: 2, scope: !14, file: !3, line: 2, type: !17)
!22 = !DILocalVariable(name: "M", arg: 3, scope: !14, file: !3, line: 2, type: !18)
!23 = !DILocalVariable(name: "p1", scope: !14, file: !3, line: 3, type: !24)
!24 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 3200, elements: !25)
!25 = !{!26}
!26 = !DISubrange(count: 100)
!27 = !DILocalVariable(name: "t1", scope: !14, file: !3, line: 4, type: !7)
!28 = !DILocalVariable(name: "sum", scope: !14, file: !3, line: 5, type: !7)
!29 = !DILocalVariable(name: "i", scope: !30, file: !3, line: 7, type: !17)
!30 = distinct !DILexicalBlock(scope: !14, file: !3, line: 7, column: 2)
!31 = !DILocalVariable(name: "i", scope: !32, file: !3, line: 10, type: !17)
!32 = distinct !DILexicalBlock(scope: !14, file: !3, line: 10, column: 2)
!33 = !DILocation(line: 0, scope: !14)
!34 = !DILocation(line: 3, column: 2, scope: !14)
!35 = !DILocation(line: 3, column: 8, scope: !14)
!36 = !DILocation(line: 4, column: 13, scope: !14)
!37 = !{!38, !38, i64 0}
!38 = !{!"float", !39, i64 0}
!39 = !{!"omnipotent char", !40, i64 0}
!40 = !{!"Simple C/C++ TBAA"}
!41 = !DILocation(line: 0, scope: !30)
!42 = !DILocation(line: 7, column: 20, scope: !43)
!43 = distinct !DILexicalBlock(scope: !30, file: !3, line: 7, column: 2)
!44 = !DILocation(line: 9, column: 13, scope: !45)
!45 = distinct !DILexicalBlock(scope: !43, file: !3, line: 9, column: 2)
!46 = !DILocation(line: 7, column: 17, scope: !43)
!47 = !DILocation(line: 7, column: 2, scope: !30)
!48 = !DILocation(line: 14, column: 20, scope: !49)
!49 = distinct !DILexicalBlock(scope: !50, file: !3, line: 12, column: 2)
!50 = distinct !DILexicalBlock(scope: !32, file: !3, line: 10, column: 2)
!51 = !DILocation(line: 0, scope: !32)
!52 = !DILocation(line: 10, column: 17, scope: !50)
!53 = !DILocation(line: 10, column: 2, scope: !32)
!54 = !DILocation(line: 9, column: 16, scope: !45)
!55 = !{!56, !56, i64 0}
!56 = !{!"int", !39, i64 0}
!57 = !DILocation(line: 9, column: 4, scope: !45)
!58 = !DILocation(line: 9, column: 10, scope: !45)
!59 = !DILocation(line: 7, column: 25, scope: !43)
!60 = distinct !{!60, !47, !61, !62}
!61 = !DILocation(line: 9, column: 23, scope: !30)
!62 = !{!"llvm.loop.mustprogress"}
!63 = !DILocation(line: 15, column: 6, scope: !49)
!64 = !DILocation(line: 18, column: 1, scope: !14)
!65 = !DILocation(line: 17, column: 2, scope: !14)
!66 = !DILocation(line: 12, column: 4, scope: !49)
!67 = !{!68, !38, i64 0}
!68 = !{!"array@_ZTSA100_f", !38, i64 0}
!69 = !DILocation(line: 12, column: 19, scope: !49)
!70 = !DILocation(line: 14, column: 14, scope: !49)
!71 = !DILocation(line: 14, column: 10, scope: !49)
!72 = !DILocation(line: 14, column: 18, scope: !49)
!73 = !DILocation(line: 14, column: 8, scope: !49)
!74 = !DILocation(line: 15, column: 12, scope: !49)
!75 = !DILocation(line: 10, column: 23, scope: !50)
!76 = distinct !{!76, !53, !77, !62}
!77 = !DILocation(line: 16, column: 1, scope: !32)

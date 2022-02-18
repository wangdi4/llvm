; RUN: opt %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-cg -intel-ir-optreport-emitter -intel-loop-optreport=medium -force-hir-cg -print-before=hir-vec-dir-insert -disable-output 2>&1 | FileCheck %s

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, zext.i32.i64((2 * %n)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 4294967294>
; CHECK-NEXT:       |   %1 = (%M)[i1];
; CHECK-NEXT:       |   %2 = (%p2)[%1];
; CHECK-NEXT:       |   %3 = (%p1)[i1];
; CHECK-NEXT:       |   %add = %3  +  %2;
; CHECK-NEXT:       |   (%p1)[i1] = %add;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; CHECK:      BEGIN REGION { }
; CHECK-NEXT:       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:       |   (%p1)[i1] = %t1.037;
; CHECK-NEXT:       |   %t1.037 = %t1.037  *  2.000000e+00;
; CHECK-NEXT:       + END LOOP
; CHECK-NEXT: END REGION

; CHECK:      LOOP BEGIN at test.cpp (6, 1)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.cpp (6, 1)
; CHECK-NEXT:     remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence between (8:9) and (8:15)
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence between (8:9) and (8:12)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.cpp (9, 1)
; CHECK-NEXT: LOOP END

; CHECK:      LOOP BEGIN at test.cpp (9, 1)
; CHECK-NEXT:     remark #15344: Loop was not vectorized: vector dependence prevents vectorization
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence
; CHECK-NEXT:     remark #15346: vector dependence: assumed FLOW dependence
; CHECK-NEXT: LOOP END

define dso_local float @_Z3subPfS_iPi(float* nocapture %p1, float* nocapture readonly %p2, i32 %n, i32* nocapture readonly %M) local_unnamed_addr #0 !dbg !6 {
entry:
  %0 = load float, float* %p1, align 4, !dbg !9, !tbaa !10
  %mul = shl nsw i32 %n, 1, !dbg !14
  %cmp39 = icmp sgt i32 %mul, 0, !dbg !15
  br i1 %cmp39, label %for.body.preheader, label %for.cond6.preheader, !dbg !16

for.body.preheader:                               ; preds = %entry
  %wide.trip.count4345 = zext i32 %mul to i64, !dbg !15
  br label %for.body, !dbg !16

for.cond6.preheader.loopexit:                     ; preds = %for.body
  br label %for.cond6.preheader, !dbg !17

for.cond6.preheader:                              ; preds = %for.cond6.preheader.loopexit, %entry
  %cmp736 = icmp sgt i32 %n, 0, !dbg !17
  br i1 %cmp736, label %for.body9.preheader, label %for.cond.cleanup8, !dbg !18

for.body9.preheader:                              ; preds = %for.cond6.preheader
  %wide.trip.count46 = zext i32 %n to i64, !dbg !17
  br label %for.body9, !dbg !18

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv41 = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next42, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %M, i64 %indvars.iv41, !dbg !19
  %1 = load i32, i32* %arrayidx, align 4, !dbg !19, !tbaa !20
  %idxprom1 = sext i32 %1 to i64, !dbg !22
  %arrayidx2 = getelementptr inbounds float, float* %p2, i64 %idxprom1, !dbg !22
  %2 = load float, float* %arrayidx2, align 4, !dbg !22, !tbaa !10
  %arrayidx4 = getelementptr inbounds float, float* %p1, i64 %indvars.iv41, !dbg !23
  %3 = load float, float* %arrayidx4, align 4, !dbg !24, !tbaa !10
  %add = fadd fast float %3, %2, !dbg !24
  store float %add, float* %arrayidx4, align 4, !dbg !24, !tbaa !10
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1, !dbg !25
  %exitcond44.not = icmp eq i64 %indvars.iv.next42, %wide.trip.count4345, !dbg !15
  br i1 %exitcond44.not, label %for.cond6.preheader.loopexit, label %for.body, !dbg !16, !llvm.loop !26

for.cond.cleanup8.loopexit:                       ; preds = %for.body9
  %factor.lcssa = phi float [ %factor, %for.body9 ], !dbg !29
  br label %for.cond.cleanup8, !dbg !30

for.cond.cleanup8:                                ; preds = %for.cond.cleanup8.loopexit, %for.cond6.preheader
  %t1.0.lcssa = phi float [ %0, %for.cond6.preheader ], [ %factor.lcssa, %for.cond.cleanup8.loopexit ]
  ret float %t1.0.lcssa, !dbg !30

for.body9:                                        ; preds = %for.body9.preheader, %for.body9
  %indvars.iv = phi i64 [ 0, %for.body9.preheader ], [ %indvars.iv.next, %for.body9 ]
  %t1.037 = phi float [ %0, %for.body9.preheader ], [ %factor, %for.body9 ]
  %arrayidx11 = getelementptr inbounds float, float* %p1, i64 %indvars.iv, !dbg !31
  store float %t1.037, float* %arrayidx11, align 4, !dbg !32, !tbaa !10
  %factor = fmul fast float %t1.037, 2.000000e+00, !dbg !29
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !33
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count46, !dbg !17
  br i1 %exitcond.not, label %for.cond.cleanup8.loopexit, label %for.body9, !dbg !18, !llvm.loop !34
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/localdisk2/ikelarev/test7")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !DISubprogram(name: "sub", scope: !1, file: !1, line: 1, type: !7, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !8)
!7 = !DISubroutineType(types: !8)
!8 = !{}
!9 = !DILocation(line: 3, column: 12, scope: !6)
!10 = !{!11, !11, i64 0}
!11 = !{!"float", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = !DILocation(line: 6, column: 19, scope: !6)
!15 = !DILocation(line: 6, column: 16, scope: !6)
!16 = !DILocation(line: 6, column: 1, scope: !6)
!17 = !DILocation(line: 9, column: 16, scope: !6)
!18 = !DILocation(line: 9, column: 1, scope: !6)
!19 = !DILocation(line: 8, column: 15, scope: !6)
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !12, i64 0}
!22 = !DILocation(line: 8, column: 12, scope: !6)
!23 = !DILocation(line: 8, column: 3, scope: !6)
!24 = !DILocation(line: 8, column: 9, scope: !6)
!25 = !DILocation(line: 6, column: 24, scope: !6)
!26 = distinct !{!26, !16, !27, !28}
!27 = !DILocation(line: 8, column: 22, scope: !6)
!28 = !{!"llvm.loop.mustprogress"}
!29 = !DILocation(line: 11, column: 18, scope: !6)
!30 = !DILocation(line: 15, column: 1, scope: !6)
!31 = !DILocation(line: 11, column: 3, scope: !6)
!32 = !DILocation(line: 11, column: 9, scope: !6)
!33 = !DILocation(line: 9, column: 22, scope: !6)
!34 = distinct !{!34, !18, !35, !28}
!35 = !DILocation(line: 11, column: 28, scope: !6)

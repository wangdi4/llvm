; Test to check that opt-report remarks from prior transforms are preserved
; appropriately in generated vector code.

; Opt-report for loop nest before VPlan
; Global optimization report for : _Z3fooPiii
;
; LOOP BEGIN at test.cpp (2, 1)
;
;     LOOP BEGIN at test.cpp (4, 5)
;     <Predicate Optimized v1>
;         remark #25423: Invariant If condition at line 5 hoisted out of this loop
;         remark #25439: Loop unrolled with remainder by 8
;     LOOP END
; LOOP END

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -intel-opt-report=low < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace
; RUN: opt -passes=hir-ssa-deconstruction,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -intel-opt-report=low < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace

; CHECK:       LOOP BEGIN at test.cpp (2, 1)
; CHECK-NEXT:      remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:      remark #15305: vectorization support: vector length 2
; CHECK-EMPTY:
; CHECK-NEXT:      LOOP BEGIN at test.cpp (4, 5)
; CHECK-NEXT:      <Predicate Optimized v1>
; CHECK-NEXT:          remark #25423: Invariant If condition at line 5 hoisted out of this loop
; CHECK-NEXT:          remark #25439: Loop unrolled with remainder by 8
; CHECK-NEXT:      LOOP END
; CHECK-NEXT:  LOOP END
; CHECK-EMPTY:
; CHECK-NEXT:  LOOP BEGIN at test.cpp (2, 1)
; CHECK-NEXT:  <Remainder loop for vectorization>
; CHECK-EMPTY:
; CHECK-NEXT:      LOOP BEGIN at test.cpp (4, 5)
; CHECK-NEXT:      LOOP END
; CHECK-NEXT:  LOOP END

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3fooPiii(ptr nocapture %arr, i32 %m, i32 %n) local_unnamed_addr #0 !dbg !8 {
entry:
  %i.linear.iv = alloca i32, align 4
  call void @llvm.dbg.value(metadata ptr %arr, metadata !14, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 %m, metadata !15, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 %n, metadata !16, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32 %m, metadata !17, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i32 %m, metadata !19, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !27
  %cmp3.not18 = icmp slt i32 %m, 1
  call void @llvm.dbg.value(metadata i32 %m, metadata !19, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !27
  call void @llvm.dbg.declare(metadata ptr undef, metadata !20, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata ptr undef, metadata !22, metadata !DIExpression(DW_OP_deref)), !dbg !27
  call void @llvm.dbg.value(metadata ptr undef, metadata !23, metadata !DIExpression(DW_OP_deref)), !dbg !28
  call void @llvm.dbg.value(metadata i32 %m, metadata !21, metadata !DIExpression(DW_OP_constu, 1, DW_OP_minus, DW_OP_stack_value)), !dbg !27
  call void @llvm.dbg.value(metadata i32 0, metadata !20, metadata !DIExpression()), !dbg !27
  br i1 %cmp3.not18, label %omp.precond.end, label %region.0, !dbg !29

omp.precond.end:                                  ; preds = %afterloop.49, %entry
  ret void, !dbg !30

region.0:                                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ], !dbg !31
  br label %DIR.OMP.SIMD.1, !dbg !31

DIR.OMP.SIMD.1:                                   ; preds = %region.0
  %1 = add i32 %m, -1
  br label %loop.49

loop.49:                                          ; preds = %ifmerge.14, %DIR.OMP.SIMD.1
  %i1.i32.0 = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %nextivloop.49, %ifmerge.14 ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv) #3, !dbg !29, !llvm.access.group !32
  %hir.cmp.21 = icmp ne i32 %i1.i32.0, 0
  br i1 %hir.cmp.21, label %then.21, label %ifmerge.14, !dbg !35

then.21:                                          ; preds = %loop.49
  %2 = zext i32 %n to i64, !dbg !29
  %hir.cmp.55.not = icmp ult i32 %n, 8, !dbg !36
  br i1 %hir.cmp.55.not, label %ifmerge.55, label %then.55, !dbg !36

then.55:                                          ; preds = %then.21
  %3 = lshr i64 %2, 3, !dbg !29
  %4 = add nsw i64 %3, -1, !dbg !36
  br label %loop.54, !dbg !36

loop.54:                                          ; preds = %loop.54, %then.55
  %i2.i64.0 = phi i64 [ 0, %then.55 ], [ %nextivloop.54, %loop.54 ]
  %5 = shl i64 %i2.i64.0, 3, !dbg !38
  %6 = getelementptr inbounds i32, ptr %arr, i64 %5, !dbg !38
  store i32 %i1.i32.0, ptr %6, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %7 = or i64 %5, 1, !dbg !38
  %8 = getelementptr inbounds i32, ptr %arr, i64 %7, !dbg !38
  store i32 %i1.i32.0, ptr %8, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %9 = or i64 %5, 2, !dbg !38
  %10 = getelementptr inbounds i32, ptr %arr, i64 %9, !dbg !38
  store i32 %i1.i32.0, ptr %10, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %11 = or i64 %5, 3, !dbg !38
  %12 = getelementptr inbounds i32, ptr %arr, i64 %11, !dbg !38
  store i32 %i1.i32.0, ptr %12, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %13 = or i64 %5, 4, !dbg !38
  %14 = getelementptr inbounds i32, ptr %arr, i64 %13, !dbg !38
  store i32 %i1.i32.0, ptr %14, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %15 = or i64 %5, 5, !dbg !38
  %16 = getelementptr inbounds i32, ptr %arr, i64 %15, !dbg !38
  store i32 %i1.i32.0, ptr %16, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %17 = or i64 %5, 6, !dbg !38
  %18 = getelementptr inbounds i32, ptr %arr, i64 %17, !dbg !38
  store i32 %i1.i32.0, ptr %18, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %19 = or i64 %5, 7, !dbg !38
  %20 = getelementptr inbounds i32, ptr %arr, i64 %19, !dbg !38
  store i32 %i1.i32.0, ptr %20, align 4, !dbg !41, !tbaa !42, !llvm.access.group !32
  %nextivloop.54 = add nuw nsw i64 %i2.i64.0, 1, !dbg !33
  %condloop.54.not = icmp eq i64 %i2.i64.0, %4, !dbg !33
  br i1 %condloop.54.not, label %ifmerge.55.loopexit, label %loop.54, !dbg !35, !llvm.loop !46

ifmerge.55.loopexit:                              ; preds = %loop.54
  br label %ifmerge.55, !dbg !36

ifmerge.55:                                       ; preds = %ifmerge.55.loopexit, %then.21
  br label %ifmerge.14

ifmerge.14:                                       ; preds = %NewDefault, %hir.L.84, %loop.49
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv) #3, !dbg !58, !llvm.access.group !32
  %nextivloop.49 = add nuw nsw i32 %i1.i32.0, 1, !dbg !59
  %condloop.49.not = icmp eq i32 %i1.i32.0, %1, !dbg !59
  br i1 %condloop.49.not, label %afterloop.49, label %loop.49, !dbg !29, !llvm.loop !60

afterloop.49:                                     ; preds = %ifmerge.14
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ], !dbg !31
  br label %omp.precond.end, !dbg !29
}

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: argmemonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #4

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #4

attributes #0 = { mustprogress nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { argmemonly mustprogress nofree nosync nounwind willreturn }
attributes #3 = { nounwind }
attributes #4 = { nofree nosync nounwind readnone willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6}
!llvm.ident = !{!7}
!nvvm.annotations = !{}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 50}
!6 = !{i32 7, !"uwtable", i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!8 = distinct !DISubprogram(name: "foo", linkageName: "_Z3fooPiii", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !13)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11, !12, !12}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !{!14, !15, !16, !17, !19, !20, !21, !22, !23}
!14 = !DILocalVariable(name: "arr", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "m", arg: 2, scope: !8, file: !1, line: 1, type: !12)
!16 = !DILocalVariable(name: "n", arg: 3, scope: !8, file: !1, line: 1, type: !12)
!17 = !DILocalVariable(name: ".capture_expr.0", scope: !18, type: !12, flags: DIFlagArtificial)
!18 = distinct !DILexicalBlock(scope: !8, file: !1, line: 2, column: 1)
!19 = !DILocalVariable(name: ".capture_expr.1", scope: !18, type: !12, flags: DIFlagArtificial)
!20 = !DILocalVariable(name: ".omp.iv", scope: !18, type: !12, flags: DIFlagArtificial)
!21 = !DILocalVariable(name: ".omp.ub", scope: !18, type: !12, flags: DIFlagArtificial)
!22 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 3, type: !12)
!23 = !DILocalVariable(name: "j", scope: !24, file: !1, line: 4, type: !12)
!24 = distinct !DILexicalBlock(scope: !25, file: !1, line: 4, column: 5)
!25 = distinct !DILexicalBlock(scope: !18, file: !1, line: 3, column: 31)
!26 = !DILocation(line: 0, scope: !8)
!27 = !DILocation(line: 0, scope: !18)
!28 = !DILocation(line: 0, scope: !24)
!29 = !DILocation(line: 2, column: 1, scope: !8)
!30 = !DILocation(line: 10, column: 1, scope: !8)
!31 = !DILocation(line: 2, column: 1, scope: !18)
!32 = distinct !{}
!33 = !DILocation(line: 4, column: 23, scope: !34)
!34 = distinct !DILexicalBlock(scope: !24, file: !1, line: 4, column: 5)
!35 = !DILocation(line: 4, column: 5, scope: !24)
!36 = !DILocation(line: 5, column: 11, scope: !37)
!37 = distinct !DILexicalBlock(scope: !34, file: !1, line: 4, column: 33)
!38 = !DILocation(line: 6, column: 9, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !1, line: 5, column: 18)
!40 = distinct !DILexicalBlock(scope: !37, file: !1, line: 5, column: 11)
!41 = !DILocation(line: 6, column: 16, scope: !39)
!42 = !{!43, !43, i64 0}
!43 = !{!"int", !44, i64 0}
!44 = !{!"omnipotent char", !45, i64 0}
!45 = !{!"Simple C++ TBAA"}
!46 = distinct !{!46, !35, !47, !48, !49, !50}
!47 = !DILocation(line: 8, column: 5, scope: !24)
!48 = !{!"llvm.loop.mustprogress"}
!49 = !{!"llvm.loop.unroll.disable"}
!50 = distinct !{!"intel.optreport", !52, !53, !56}
!52 = !{!"intel.optreport.debug_location", !35}
!53 = !{!"intel.optreport.remarks", !54, !55}
!54 = !{!"intel.optreport.remark", i32 25423, !" at line 5"}
!55 = !{!"intel.optreport.remark", i32 25439, i32 8}
!56 = !{!"intel.optreport.origin", !57}
!57 = !{!"intel.optreport.remark", i32 25476, i32 1}
!58 = !DILocation(line: 9, column: 3, scope: !25)
!59 = !DILocation(line: 3, column: 3, scope: !18)
!60 = distinct !{!60, !31, !61, !62}
!61 = !DILocation(line: 2, column: 17, scope: !18)
!62 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}

; REQUIRES: asserts
; RUN: opt -passes=vplan-vec -vplan-print-after-plain-cfg -vplan-dump-details -disable-output < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !0
@C = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !12
@A = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !6
; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 !dbg !18 {
omp.inner.for.body.lr.ph:
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i32 1023, metadata !26, metadata !DIExpression()), !dbg !28
  %i.lpriv = alloca i32, align 4, !dbg !29
  br label %DIR.OMP.SIMD.1, !dbg !29

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %i.lpriv, i32 0, i32 1) ], !dbg !29
  br label %omp.inner.for.body, !dbg !29

omp.inner.for.body:                               ; predggs = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.body ], [ 0, %DIR.OMP.SIMD.1 ]
; CHECK:          i64 [[VP_INDVARS_IV:%.*]] = phi  [ i64 [[VP_INDVARS_IV_NEXT:%.*]], [[BB2:BB[0-9]+]] ],  [ i64 0, [[BB1:BB[0-9]+]] ]
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  %arrayidx = getelementptr inbounds [1024 x float], ptr @B, i64 0, i64 %indvars.iv, !dbg !30, !intel-tbaa !32
; CHECK:          ptr [[VP_ARRAYIDX:%.*]] = getelementptr inbounds [1024 x float], ptr @B i64 0 i64 [[VP_INDVARS_IV]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:12
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  %1 = load float, ptr %arrayidx, align 4, !dbg !30, !tbaa !32
; CHECK:          float [[VP0:%.*]] = load ptr [[VP_ARRAYIDX]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:12
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      Align: 4
; CHECK-NEXT:      Ordering: 0, Volatile: 0, SSID: 1
; CHECK-NEXT:      NonDbgMDs -
; CHECK-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; CHECK-NEXT:      end of details

  %arrayidx2 = getelementptr inbounds [1024 x float], ptr @C, i64 0, i64 %indvars.iv, !dbg !37, !intel-tbaa !32
; CHECK:          ptr [[VP_ARRAYIDX2:%.*]] = getelementptr inbounds [1024 x float], ptr @C i64 0 i64 [[VP_INDVARS_IV]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:19
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  %2 = load atomic float, ptr %arrayidx2 unordered, align 4, !dbg !37, !tbaa !32
; CHECK:          float [[VP1:%.*]] = load ptr [[VP_ARRAYIDX2]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:19
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      Align: 4
; CHECK-NEXT:      Ordering: 1, Volatile: 0, SSID: 1
; CHECK-NEXT:      NonDbgMDs -
; CHECK-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; CHECK-NEXT:      end of details

  %mul3 = fmul fast float %2, %1, !dbg !38
; CHECK:          float [[VP_MUL3:%.*]] = fmul float [[VP1]] float [[VP0]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:17
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 1, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  %arrayidx5 = getelementptr inbounds [1024 x float], ptr @A, i64 0, i64 %indvars.iv, !dbg !39, !intel-tbaa !32
; CHECK:          ptr [[VP_ARRAYIDX5:%.*]] = getelementptr inbounds [1024 x float], ptr @A i64 0 i64 [[VP_INDVARS_IV]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:5
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  store float %mul3, ptr %arrayidx5, align 4, !dbg !40, !tbaa !32, !nontemporal !47
; CHECK:          store float [[VP_MUL3]] ptr [[VP_ARRAYIDX5]]
; CHECK-NEXT:      DbgLoc: lit_test.c:10:10
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      Align: 4
; CHECK-NEXT:      Ordering: 0, Volatile: 0, SSID: 1
; CHECK-NEXT:      NonDbgMDs -
; CHECK-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; CHECK-NEXT:        <0x{{.*}}> = !{i32 1}
; CHECK-NEXT:      end of details

  call void @bar(float %mul3) #1, !dbg !41
; CHECK:          call float [[VP_MUL3]] ptr @bar
; CHECK-NEXT:      DbgLoc: lit_test.c:11:5
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  %3 = ashr exact i32 42, 4
; CHECK:          i32 [[VP3:%.*]] = ashr i32 42 i32 4
; CHECK-NEXT:      DbgLoc:
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 1
; CHECK-NEXT:      end of details

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !42
; CHECK:          i64 [[VP_INDVARS_IV_NEXT]] = add i64 [[VP_INDVARS_IV]] i64 1
; CHECK-NEXT:      DbgLoc: lit_test.c:9:3
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 1, NUW: 1, Exact: 0
; CHECK-NEXT:      end of details

  %exitcond = icmp eq i64 %indvars.iv.next, 1024, !dbg !42
; CHECK:          i1 [[VP_EXITCOND:%.*]] = icmp eq i64 [[VP_INDVARS_IV_NEXT]] i64 1024
; CHECK-NEXT:      DbgLoc: lit_test.c:9:3
; CHECK-NEXT:      OperatorFlags -
; CHECK-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; CHECK-NEXT:      end of details

  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %omp.inner.for.body, !dbg !29, !llvm.loop !43

DIR.OMP.END.SIMD.4:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ], !dbg !29
  br label %DIR.OMP.END.SIMD.2, !dbg !46

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void, !dbg !46
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1
declare dso_local void @bar(float) local_unnamed_addr #2
; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" "vector-variants"="_ZGVbN4v_bar,_ZGVcN4v_bar,_ZGVdN4v_bar,_ZGVeN4v_bar,_ZGVbM4v_bar,_ZGVcM4v_bar,_ZGVdM4v_bar,_ZGVeM4v_bar" }
attributes #3 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16}
!llvm.ident = !{!17}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "B", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang based Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "lit_test.c", directory: "/export/iusers/karthik1/code/vpmetadata")
!4 = !{}
!5 = !{!6, !0, !12}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "A", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 32768, elements: !10)
!9 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!10 = !{!11}
!11 = !DISubrange(count: 1024)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "C", scope: !2, file: !3, line: 1, type: !8, isLocal: false, isDefinition: true)
!14 = !{i32 7, !"Dwarf Version", i32 4}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!18 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 6, type: !19, scopeLine: 6, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !21)
!19 = !DISubroutineType(types: !20)
!20 = !{null}
!21 = !{!22, !24, !26}
!22 = !DILocalVariable(name: "i", scope: !18, file: !3, line: 7, type: !23)
!23 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!24 = !DILocalVariable(name: ".omp.iv", scope: !25, type: !23, flags: DIFlagArtificial)
!25 = distinct !DILexicalBlock(scope: !18, file: !3, line: 8, column: 1)
!26 = !DILocalVariable(name: ".omp.ub", scope: !25, type: !23, flags: DIFlagArtificial)
!27 = !DILocation(line: 0, scope: !18)
!28 = !DILocation(line: 0, scope: !25)
!29 = !DILocation(line: 8, column: 1, scope: !18)
!30 = !DILocation(line: 10, column: 12, scope: !31)
!31 = distinct !DILexicalBlock(scope: !25, file: !3, line: 9, column: 30)
!32 = !{!33, !34, i64 0}
!33 = !{!"array@_ZTSA1024_f", !34, i64 0}
!34 = !{!"float", !35, i64 0}
!35 = !{!"omnipotent char", !36, i64 0}
!36 = !{!"Simple C/C++ TBAA"}
!37 = !DILocation(line: 10, column: 19, scope: !31)
!38 = !DILocation(line: 10, column: 17, scope: !31)
!39 = !DILocation(line: 10, column: 5, scope: !31)
!40 = !DILocation(line: 10, column: 10, scope: !31)
!41 = !DILocation(line: 11, column: 5, scope: !31)
!42 = !DILocation(line: 9, column: 3, scope: !25)
!43 = distinct !{!43, !44, !45}
!44 = !DILocation(line: 8, column: 1, scope: !25)
!45 = !DILocation(line: 8, column: 28, scope: !25)
!46 = !DILocation(line: 13, column: 1, scope: !18)
!47 = !{i32 1}


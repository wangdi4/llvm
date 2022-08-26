; Test to check correctness of HIR decomposer and vector codegen to capture and preserve
; underlying operator flags (FMF, NSW, NUW, exact).

; Incoming HIR
; <0>          BEGIN REGION { }
; <23:8>             %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; <22:8>
; <22:8>             + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3:10>             |   %0 = (@B)[0][i1];
; <5:10>             |   %1 = (@C)[0][i1];
; <6:10>             |   %mul3 = %1  *  %0;
; <8:10>             |   (@A)[0][i1] = %mul3;
; <9:11>             |   %2 = 42  >>  %t;
; <10:11>            |   %fp.cmp = %mul3 > %1;
; <13:11>            |   %red.phi = zext.i1.i32(%fp.cmp) + %2  +  %red.phi; <Safe Reduction>
; <14:11>            |   %fp.select = (%mul3 > %1) ? %mul3 : %1;
; <15:11>            |   (@A)[0][i1] = %fp.select;
; <22:8>             + END LOOP
; <22:8>
; <24:8>             @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; <0>          END REGION

; REQUIRES: asserts
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-dump-details -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VPLAN-IR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -vplan-print-after-plain-cfg -vplan-dump-details -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VPLAN-IR

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -hir-details-llvm-inst -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VEC-HIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -hir-details-llvm-inst -disable-output < %s 2>&1 | FileCheck %s --check-prefix=VEC-HIR


; VEC-HIR:      <0>          BEGIN REGION { modified }
; VEC-HIR-NEXT: <{{[0-9]+}}:8>             %red.init = call <4 x i32> @llvm.ssa.copy.v4i32(<4 x i32> undef), !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:8>             %red.init.insert = insertelement <4 x i32> undef, i32 undef, i64 0, !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:11>             %.copy = call <4 x i32> @llvm.ssa.copy.v4i32(<4 x i32> undef), !dbg !30
; VEC-HIR:      <{{[0-9]+}}:8>             + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; VEC-HIR-NEXT: <{{[0-9]+}}:10>            |     %.vec = load <4 x float>, <4 x float>* undef, align 16, !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:10>            |     %.vec1 = load <4 x float>, <4 x float>* undef, align 16, !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:10>            |     %.vec2 = fmul fast <4 x float> undef, undef, !dbg !32
; VEC-HIR-NEXT: <{{[0-9]+}}:10>            |     store <4 x float> undef, <4 x float>* undef, align 16, !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     %.vec3 = ashr exact <4 x i32> undef, undef, !dbg !30
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     %.vec4 = fcmp fast true <4 x float> undef, undef, !dbg !30
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     %.vec5 = add <4 x i32> undef, undef, !dbg !30
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     %.vec6 = add <4 x i32> undef, undef, !dbg !30
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     %.vec7 = select fast i1 undef, <4 x float> undef, <4 x float> undef, !dbg !30
; VEC-HIR-NEXT: <{{[0-9]+}}:11>            |     store <4 x float> undef, <4 x float>* undef, align 16, !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:8>             |     %.copy8 = call <4 x i32> @llvm.ssa.copy.v4i32(<4 x i32> undef), !dbg !29
; VEC-HIR-NEXT: <{{[0-9]+}}:8>             + END LOOP
; VEC-HIR:      <{{[0-9]+}}:11>            %vec.reduce = call i32 @llvm.vector.reduce.add.v4i32(<4 x i32> undef), !dbg !30
; VEC-HIR: <0>          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !0
@C = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !12
@A = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16, !dbg !6
; Function Attrs: nounwind uwtable
define dso_local i32 @foo(i32* %uni, i32 %t) local_unnamed_addr #0 !dbg !18 {
loop.ph:
  call void @llvm.dbg.value(metadata i32 0, metadata !22, metadata !DIExpression()), !dbg !27
  call void @llvm.dbg.value(metadata i32 1023, metadata !26, metadata !DIExpression()), !dbg !28
  br label %loop.body, !dbg !29

loop.body:                               ; predggs = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %red.phi = phi i32 [ %red.add, %loop.body ], [ %t, %loop.ph ]
; VPLAN-IR:          i32 [[VP4:%.*]] = phi  [ i32 [[RED_PHI0:%.*]], [[BB1:BB.*]] ],  [ i32 [[VP5:%.*]], [[BB2:BB.*]] ]
; VPLAN-IR-NEXT:      DbgLoc:
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %indvars.iv = phi i64 [ %indvars.iv.next, %loop.body ], [ 0, %loop.ph ]
; VPLAN-IR:          i64 [[VP6:%.*]] = phi  [ i64 0, [[BB1]] ],  [ i64 [[VP7:%.*]], [[BB2]] ]
; VPLAN-IR-NEXT:      DbgLoc:
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %arrayidx = getelementptr inbounds [1024 x float], [1024 x float]* @B, i64 0, i64 %indvars.iv, !dbg !30, !intel-tbaa !32
; VPLAN-IR:          float* [[VP8:%.*]] = subscript inbounds [1024 x float]* @B i64 0 i64 [[VP6]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:12
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %0 = load float, float* %arrayidx, align 4, !dbg !30, !tbaa !32
; VPLAN-IR:          float [[VP9:%.*]] = load float* [[VP8]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:12
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      Align: 4
; VPLAN-IR-NEXT:      Ordering: 0, Volatile: 0, SSID: 0
; VPLAN-IR-NEXT:      NonDbgMDs -
; VPLAN-IR-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; VPLAN-IR-NEXT:      end of details

  %arrayidx2 = getelementptr inbounds [1024 x float], [1024 x float]* @C, i64 0, i64 %indvars.iv, !dbg !37, !intel-tbaa !32
; VPLAN-IR:          float* [[VP10:%.*]] = subscript inbounds [1024 x float]* @C i64 0 i64 [[VP6]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:19
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %1 = load float, float* %arrayidx2, align 4, !dbg !37, !tbaa !32
; VPLAN-IR:          float [[VP11:%.*]] = load float* [[VP10]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:19
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      Align: 4
; VPLAN-IR-NEXT:      Ordering: 0, Volatile: 0, SSID: 0
; VPLAN-IR-NEXT:      NonDbgMDs -
; VPLAN-IR-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; VPLAN-IR-NEXT:      end of details

  %mul3 = fmul fast float %1, %0, !dbg !38
; VPLAN-IR:          float [[VP12:%.*]] = fmul float [[VP11]] float [[VP9]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:17
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 1, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %arrayidx5 = getelementptr inbounds [1024 x float], [1024 x float]* @A, i64 0, i64 %indvars.iv, !dbg !39, !intel-tbaa !32
; VPLAN-IR:          float* [[VP13:%.*]] = subscript inbounds [1024 x float]* @A i64 0 i64 [[VP6]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  store float %mul3, float* %arrayidx5, align 4, !dbg !40, !tbaa !32
; VPLAN-IR:          store float [[VP12]] float* [[VP13]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:10
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      Align: 4
; VPLAN-IR-NEXT:      Ordering: 0, Volatile: 0, SSID: 0
; VPLAN-IR-NEXT:      NonDbgMDs -
; VPLAN-IR-NEXT:        <0x{{.*}}> = !{<0x{{.*}}>, <0x{{.*}}>, i64 0}
; VPLAN-IR-NEXT:      end of details

  %2 = ashr exact i32 42, %t, !dbg !41
; VPLAN-IR:          i32 [[VP14:%.*]] = ashr i32 42 i32 %t
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 1
; VPLAN-IR-NEXT:      end of details

  %fp.cmp = fcmp fast ogt float %mul3, %1, !dbg !41
; VPLAN-IR:          i1 [[VP15:%.*]] = fcmp ogt float [[VP12]] float [[VP11]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 1, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %zext.fp.cmp = zext i1 %fp.cmp to i32, !dbg !41
; VPLAN-IR:          i32 [[VP16:%.*]] = zext i1 [[VP15]] to i32
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %tmp.add = add i32 %zext.fp.cmp, %2, !dbg !41
; VPLAN-IR:          i32 [[VP17:%.*]] = add i32 [[VP16]] i32 [[VP14]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %red.add = add nsw nuw i32 %tmp.add, %red.phi, !dbg !41
; VPLAN-IR:          i32 [[VP5]] = add i32 [[VP17]] i32 [[VP4]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 1, NUW: 1, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %fp.select = select fast i1 %fp.cmp, float %mul3, float %1, !dbg !41
; VPLAN-IR:          i1 [[VP18:%.*]] = fcmp ogt float [[VP12]] float [[VP11]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 1, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details
; VPLAN-IR-EMPTY:
; VPLAN-IR-NEXT:     float [[VP19:%.*]] = select i1 [[VP18]] float [[VP12]] float [[VP11]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 1, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  store float %fp.select, float* %arrayidx5, !dbg !41
; VPLAN-IR:          float* [[VP20:%.*]] = subscript inbounds [1024 x float]* @A i64 0 i64 [[VP6]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:10:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details
; VPLAN-IR-EMPTY:
; VPLAN-IR-NEXT:     store float [[VP19]] float* [[VP20]]
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:11:5
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      Align: 4
; VPLAN-IR-NEXT:      Ordering: 0, Volatile: 0, SSID: 0
; VPLAN-IR-NEXT:      end of details

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !42
; VPLAN-IR:          i64 [[VP7]] = add i64 [[VP6]] i64 1
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:9:3
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  %exitcond = icmp eq i64 %indvars.iv.next, 1024, !dbg !42
; VPLAN-IR:          i1 [[VP21:%.*]] = icmp slt i64 [[VP7]] i64 1024
; VPLAN-IR-NEXT:      DbgLoc: lit_test.c:9:3
; VPLAN-IR-NEXT:      OperatorFlags -
; VPLAN-IR-NEXT:        FMF: 0, NSW: 0, NUW: 0, Exact: 0
; VPLAN-IR-NEXT:      end of details

  br i1 %exitcond, label %loop.exit, label %loop.body, !dbg !29, !llvm.loop !43

loop.exit:                               ; preds = %DIR.OMP.END.SIMD.4
  ret i32 %red.add, !dbg !46
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



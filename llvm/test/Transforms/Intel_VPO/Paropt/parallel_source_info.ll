; RUN: opt -bugpoint-enable-legacy-pm -parallel-source-info=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=NOINFO
; RUN: opt -bugpoint-enable-legacy-pm -parallel-source-info=1 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FNINFO
; RUN: opt -bugpoint-enable-legacy-pm -parallel-source-info=2 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=PATHINFO
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=DEFAULT

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=0 -S %s | FileCheck %s --check-prefix=NOINFO
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=1 -S %s | FileCheck %s --check-prefix=FNINFO
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=2 -S %s | FileCheck %s --check-prefix=PATHINFO
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=DEFAULT

; Test src:
;
; int a[100];
; void foo() {
;   int i;
;   #pragma omp for
;   for(i=1; i<100; i++)
;     a[i] = i;
; }

; Check that -parallel-source-info=0 produces no source location info
; NOINFO:    private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; Check that -parallel-source-info=1 has function+line in the source location info
; FNINFO:    private unnamed_addr constant [18 x i8] c";unknown;foo;4;4;;"

; Check that -parallel-source-info=2 has path+function+line in the source location info
; PATHINFO:    private unnamed_addr constant [62 x i8] c";/full/path/of/test/directory/parallel_source_info.c;foo;4;4;;"

; Check that the default behavior is -parallel-source-info=2
; DEFAULT:  private unnamed_addr constant [62 x i8] c";/full/path/of/test/directory/parallel_source_info.c;foo;4;4;;"

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global [100 x i32] zeroinitializer, align 16, !dbg !0

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !16 {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.dbg.declare(metadata ptr %i, metadata !20, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !22, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !25, metadata !DIExpression()), !dbg !24
  store i32 0, ptr %.omp.lb, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !27, metadata !DIExpression()), !dbg !24
  store i32 98, ptr %.omp.ub, align 4, !dbg !26
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
, !dbg !28
  %1 = load i32, ptr %.omp.lb, align 4, !dbg !26
  store i32 %1, ptr %.omp.iv, align 4, !dbg !26
  br label %omp.inner.for.cond, !dbg !29

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4, !dbg !26
  %3 = load i32, ptr %.omp.ub, align 4, !dbg !26
  %cmp = icmp sle i32 %2, %3, !dbg !30
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !29

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4, !dbg !26
  %mul = mul nsw i32 %4, 1, !dbg !31
  %add = add nsw i32 1, %mul, !dbg !31
  store i32 %add, ptr %i, align 4, !dbg !31
  %5 = load i32, ptr %i, align 4, !dbg !32
  %6 = load i32, ptr %i, align 4, !dbg !33
  %idxprom = sext i32 %6 to i64, !dbg !34
  %arrayidx = getelementptr inbounds [100 x i32], ptr @a, i64 0, i64 %idxprom, !dbg !34
  store i32 %5, ptr %arrayidx, align 4, !dbg !35
  br label %omp.body.continue, !dbg !34

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !34

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4, !dbg !26
  %add1 = add nsw i32 %7, 1, !dbg !30
  store i32 %add1, ptr %.omp.iv, align 4, !dbg !30
  br label %omp.inner.for.cond, !dbg !34, !llvm.loop !36

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !34

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
, !dbg !28
  ret void, !dbg !38
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11, !12, !13, !14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !5, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C11, file: !3, producer: "clang version 17.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "parallel_source_info.c", directory: "/full/path/of/test/directory")
!4 = !{!0}
!5 = !DICompositeType(tag: DW_TAG_array_type, baseType: !6, size: 3200, elements: !7)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{!8}
!8 = !DISubrange(count: 100)
!9 = !{i32 7, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 7, !"openmp", i32 51}
!13 = !{i32 7, !"uwtable", i32 2}
!14 = !{i32 7, !"frame-pointer", i32 2}
!16 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 2, type: !17, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{null}
!19 = !{}
!20 = !DILocalVariable(name: "i", scope: !16, file: !3, line: 3, type: !6)
!21 = !DILocation(line: 3, column: 7, scope: !16)
!22 = !DILocalVariable(name: ".omp.iv", scope: !23, type: !6, flags: DIFlagArtificial)
!23 = distinct !DILexicalBlock(scope: !16, file: !3, line: 4, column: 1)
!24 = !DILocation(line: 0, scope: !23)
!25 = !DILocalVariable(name: ".omp.lb", scope: !23, type: !6, flags: DIFlagArtificial)
!26 = !DILocation(line: 5, column: 7, scope: !23)
!27 = !DILocalVariable(name: ".omp.ub", scope: !23, type: !6, flags: DIFlagArtificial)
!28 = !DILocation(line: 4, column: 1, scope: !23)
!29 = !DILocation(line: 4, column: 1, scope: !16)
!30 = !DILocation(line: 5, column: 3, scope: !23)
!31 = !DILocation(line: 5, column: 19, scope: !23)
!32 = !DILocation(line: 6, column: 12, scope: !23)
!33 = !DILocation(line: 6, column: 7, scope: !23)
!34 = !DILocation(line: 6, column: 5, scope: !23)
!35 = !DILocation(line: 6, column: 10, scope: !23)
!36 = distinct !{!36, !28, !37}
!37 = !DILocation(line: 4, column: 16, scope: !23)
!38 = !DILocation(line: 7, column: 1, scope: !16)

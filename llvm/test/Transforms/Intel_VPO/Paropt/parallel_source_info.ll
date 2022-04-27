; RUN: opt -parallel-source-info=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=CHECK-NONE
; RUN: opt -parallel-source-info=1 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=CHECK-FUNC
; RUN: opt -parallel-source-info=2 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=CHECK-PATH
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=CHECK-DEFAULT

; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=0 -S %s | FileCheck %s --check-prefix=CHECK-NONE
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=1 -S %s | FileCheck %s --check-prefix=CHECK-FUNC
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -parallel-source-info=2 -S %s | FileCheck %s --check-prefix=CHECK-PATH
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=CHECK-DEFAULT

; The test was obtained by compiling the C source below with icx -g -fiopenmp
;
; int a[100];
; void foo() {
;   int i;
;   #pragma omp for
;   for(i=1; i<100; i++)
;     a[i] = i;
; }

; Check that -parallel-source-info=0 produces no source location info
; CHECK-NONE:    private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; Check that -parallel-source-info=1 has function+line in the source location info
; CHECK-FUNC:    private unnamed_addr constant [18 x i8] c";unknown;foo;4;4;;"

; Check that -parallel-source-info=2 has path+function+line in the source location info
; CHECK-PATH:    private unnamed_addr constant [62 x i8] c";/full/path/of/test/directory/parallel_source_info.c;foo;4;4;;"

; Check that the default behavior is -parallel-source-info=1
; CHECK-DEFAULT: private unnamed_addr constant [18 x i8] c";unknown;foo;4;4;;"

; ModuleID = 'parallel_source_info.c'
source_filename = "parallel_source_info.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local global [100 x i32] zeroinitializer, align 16, !dbg !0
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !14 {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %i, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %.omp.iv, metadata !19, metadata !DIExpression()), !dbg !21
  call void @llvm.dbg.declare(metadata i32* %.omp.lb, metadata !22, metadata !DIExpression()), !dbg !21
  store i32 0, i32* %.omp.lb, align 4, !dbg !23
  call void @llvm.dbg.declare(metadata i32* %.omp.ub, metadata !24, metadata !DIExpression()), !dbg !21
  store i32 98, i32* %.omp.ub, align 4, !dbg !23
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg !25
  %1 = load i32, i32* %.omp.lb, align 4, !dbg !23
  store i32 %1, i32* %.omp.iv, align 4, !dbg !23
  br label %omp.inner.for.cond, !dbg !25

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4, !dbg !23
  %3 = load i32, i32* %.omp.ub, align 4, !dbg !23
  %cmp = icmp sle i32 %2, %3, !dbg !26
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !25

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4, !dbg !23
  %mul = mul nsw i32 %4, 1, !dbg !27
  %add = add nsw i32 1, %mul, !dbg !27
  store i32 %add, i32* %i, align 4, !dbg !27
  %5 = load i32, i32* %i, align 4, !dbg !28
  %6 = load i32, i32* %i, align 4, !dbg !29
  %idxprom = sext i32 %6 to i64, !dbg !30
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %idxprom, !dbg !30
  store i32 %5, i32* %arrayidx, align 4, !dbg !31
  br label %omp.body.continue, !dbg !30

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !32

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4, !dbg !23
  %add1 = add nsw i32 %7, 1, !dbg !26
  store i32 %add1, i32* %.omp.iv, align 4, !dbg !26
  br label %omp.inner.for.cond, !dbg !32, !llvm.loop !33

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !32

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ], !dbg !25
  ret void, !dbg !35
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: None)
!3 = !DIFile(filename: "parallel_source_info.c", directory: "/full/path/of/test/directory")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 3200, elements: !8)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DISubrange(count: 100)
!10 = !{i32 7, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{!"clang version 9.0.0"}
!14 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 2, type: !15, scopeLine: 2, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!15 = !DISubroutineType(types: !16)
!16 = !{null}
!17 = !DILocalVariable(name: "i", scope: !14, file: !3, line: 3, type: !7)
!18 = !DILocation(line: 3, column: 7, scope: !14)
!19 = !DILocalVariable(name: ".omp.iv", scope: !20, type: !7, flags: DIFlagArtificial)
!20 = distinct !DILexicalBlock(scope: !14, file: !3, line: 4, column: 3)
!21 = !DILocation(line: 0, scope: !20)
!22 = !DILocalVariable(name: ".omp.lb", scope: !20, type: !7, flags: DIFlagArtificial)
!23 = !DILocation(line: 5, column: 7, scope: !20)
!24 = !DILocalVariable(name: ".omp.ub", scope: !20, type: !7, flags: DIFlagArtificial)
!25 = !DILocation(line: 4, column: 3, scope: !14)
!26 = !DILocation(line: 5, column: 3, scope: !20)
!27 = !DILocation(line: 5, column: 19, scope: !20)
!28 = !DILocation(line: 6, column: 12, scope: !20)
!29 = !DILocation(line: 6, column: 7, scope: !20)
!30 = !DILocation(line: 6, column: 5, scope: !20)
!31 = !DILocation(line: 6, column: 10, scope: !20)
!32 = !DILocation(line: 4, column: 3, scope: !20)
!33 = distinct !{!33, !32, !34}
!34 = !DILocation(line: 4, column: 19, scope: !20)
!35 = !DILocation(line: 7, column: 1, scope: !14)

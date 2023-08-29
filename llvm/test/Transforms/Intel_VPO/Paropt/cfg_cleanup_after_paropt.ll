; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(vpo-cfg-simplify)' -S %s | FileCheck %s

; Test src:
;
; void foo(float *a) {
; #pragma omp parallel for
;   for (int i = 0; i < 100; ++i)
;     a[i] = i * 0.7f;
; }

; Check that there are no redundant blocks:
; CHECK: define internal void @foo.DIR.OMP.PARALLEL.LOOP
; CHECK-SAME: {
; CHECK-NEXT: [[ROOTBB:[A-Za-z0-9._]+]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[BB1:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[BB1]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[BB2:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[BB2]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[BB3:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[BB3]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br i1{{.*}}label %[[LOOPPH:[A-Za-z0-9._]+]], label %[[FOREND:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[LOOPPH]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br i1{{.*}}label %[[FORBODY:[A-Za-z0-9._]+]], label %[[REGEXIT:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[FORBODY]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[BODYCONT:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[BODYCONT]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[FORINC:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[FORINC]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br i1{{.*}}label %[[FORBODY]], label %[[CRITEDGE:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[CRITEDGE]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK-NEXT: br label %[[REGEXIT]]
; CHECK-EMPTY:
; CHECK-NEXT: [[REGEXIT]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[FOREND]],
; CHECK-EMPTY:
; CHECK-NEXT: [[FOREND]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: br label %[[BBRET:[A-Za-z0-9._]+]],
; CHECK-EMPTY:
; CHECK-NEXT: [[BBRET]]:
; CHECK-NOT: br label %
; CHECK-NOT: br i1
; CHECK: ret void

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %a) #0 !dbg !9 {
entry:
  %a.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !17, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !21, metadata !DIExpression()), !dbg !20
  store i32 0, ptr %.omp.lb, align 4, !dbg !22
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !23, metadata !DIExpression()), !dbg !20
  store i32 99, ptr %.omp.ub, align 4, !dbg !22
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ], !dbg !24
  %1 = load i32, ptr %.omp.lb, align 4, !dbg !22
  store i32 %1, ptr %.omp.iv, align 4, !dbg !22
  br label %omp.inner.for.cond, !dbg !25

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4, !dbg !22
  %3 = load i32, ptr %.omp.ub, align 4, !dbg !22
  %cmp = icmp sle i32 %2, %3, !dbg !26
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !25

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.dbg.declare(metadata ptr %i, metadata !27, metadata !DIExpression()), !dbg !28
  %4 = load i32, ptr %.omp.iv, align 4, !dbg !22
  %mul = mul nsw i32 %4, 1, !dbg !29
  %add = add nsw i32 0, %mul, !dbg !29
  store i32 %add, ptr %i, align 4, !dbg !29
  %5 = load i32, ptr %i, align 4, !dbg !30
  %conv = sitofp i32 %5 to float, !dbg !30
  %mul1 = fmul fast float %conv, 0x3FE6666660000000, !dbg !31
  %6 = load ptr, ptr %a.addr, align 8, !dbg !32
  %7 = load i32, ptr %i, align 4, !dbg !33
  %idxprom = sext i32 %7 to i64, !dbg !32
  %arrayidx = getelementptr inbounds float, ptr %6, i64 %idxprom, !dbg !32
  store float %mul1, ptr %arrayidx, align 4, !dbg !34
  br label %omp.body.continue, !dbg !32

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !24

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4, !dbg !22
  %add2 = add nsw i32 %8, 1, !dbg !26
  store i32 %add2, ptr %.omp.iv, align 4, !dbg !26
  br label %omp.inner.for.cond, !dbg !24, !llvm.loop !35

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !24

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !24
  ret void, !dbg !37
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)", isOptimized: false, flags: " --intel --intel-config /path/to/ws/deploy/linux_debug/bin/icx.cfg -O0 -fiopenmp -fopenmp-version=51 -emit-llvm -S -Xclang -disable-llvm-optzns -Xclang -fopenmp-typed-clauses cfg_cleanup_after_paropt.c -g -o temp.ll -fveclib=SVML -fheinous-gnu-extensions", runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "cfg_cleanup_after_paropt.c", directory: "/path/to")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"openmp", i32 51}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!9 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !10, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !14)
!10 = !DISubroutineType(types: !11)
!11 = !{null, !12}
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!14 = !{}
!15 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !1, line: 1, type: !12)
!16 = !DILocation(line: 1, column: 17, scope: !9)
!17 = !DILocalVariable(name: ".omp.iv", scope: !18, type: !19, flags: DIFlagArtificial)
!18 = distinct !DILexicalBlock(scope: !9, file: !1, line: 2, column: 1)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DILocation(line: 0, scope: !18)
!21 = !DILocalVariable(name: ".omp.lb", scope: !18, type: !19, flags: DIFlagArtificial)
!22 = !DILocation(line: 3, column: 8, scope: !18)
!23 = !DILocalVariable(name: ".omp.ub", scope: !18, type: !19, flags: DIFlagArtificial)
!24 = !DILocation(line: 2, column: 1, scope: !18)
!25 = !DILocation(line: 2, column: 1, scope: !9)
!26 = !DILocation(line: 3, column: 3, scope: !18)
!27 = !DILocalVariable(name: "i", scope: !18, file: !1, line: 3, type: !19)
!28 = !DILocation(line: 3, column: 12, scope: !18)
!29 = !DILocation(line: 3, column: 28, scope: !18)
!30 = !DILocation(line: 4, column: 12, scope: !18)
!31 = !DILocation(line: 4, column: 14, scope: !18)
!32 = !DILocation(line: 4, column: 5, scope: !18)
!33 = !DILocation(line: 4, column: 7, scope: !18)
!34 = !DILocation(line: 4, column: 10, scope: !18)
!35 = distinct !{!35, !24, !36}
!36 = !DILocation(line: 2, column: 25, scope: !18)
!37 = !DILocation(line: 5, column: 1, scope: !9)

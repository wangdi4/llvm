; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-slm=false -vpo-paropt-atomic-free-red-local-buf-size=0 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-slm=false -vpo-paropt-atomic-free-red-local-buf-size=0 -S %s | FileCheck %s


; Test src:
;
; int main(void) {
;   int i, sum = 0;
;
; #pragma omp target teams distribute parallel for reduction(+:sum) map(tofrom:sum)
;   for (i = 0; i < 10; i++) {
;     sum += i;
;   }
;
;   return 0;
; }
;
; This test checks that no llvm.dbg.declare is generated for the GEP of global reduction buffer
; as it is a temporary location for intermediate reduction results and should not be observable
; by debugger.

; CHECK: %[[GLOBAL_GEP:[^,]+]] = getelementptr i32, ptr addrspace(1) %red_buf
; CHECK-NOT: call void @llvm.dbg.declare(metadata ptr addrspace(1) %[[GLOBAL_GEP]]
; CHECK: DILocalVariable(name: "sum", scope: ![[SCOPE:[^,]+]] 
; CHECK-NOT: DILocalVariable(name: "sum", scope: ![[SCOPE]]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@0 = private unnamed_addr addrspace(4) constant [21 x i8] c";sum;test.cpp;4;11;;\00", align 1

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 !dbg !13 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.dbg.declare(metadata ptr %i, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata ptr %sum, metadata !17, metadata !DIExpression()), !dbg !18
  store i32 0, ptr addrspace(4) %sum.ascast, align 4, !dbg !18
  call void @llvm.dbg.declare(metadata ptr %.omp.lb, metadata !19, metadata !DIExpression()), !dbg !21
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4, !dbg !22
  call void @llvm.dbg.declare(metadata ptr %.omp.ub, metadata !23, metadata !DIExpression()), !dbg !21
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4, !dbg !22
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %sum.ascast, i64 4, i64 35, ptr addrspace(4) @0, i8* null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
, !dbg !24
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]
, !dbg !25
  call void @llvm.dbg.declare(metadata ptr %.omp.iv, metadata !27, metadata !DIExpression()), !dbg !29
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]
, !dbg !30
  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4, !dbg !31
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4, !dbg !31
  br label %omp.inner.for.cond, !dbg !25

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !dbg !31
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4, !dbg !31
  %cmp = icmp sle i32 %4, %5, !dbg !32
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !25

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !dbg !31
  %mul = mul nsw i32 %6, 1, !dbg !33
  %add = add nsw i32 0, %mul, !dbg !33
  store i32 %add, ptr addrspace(4) %i.ascast, align 4, !dbg !33
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4, !dbg !34
  %8 = load i32, ptr addrspace(4) %sum.ascast, align 4, !dbg !36
  %add1 = add nsw i32 %8, %7, !dbg !36
  store i32 %add1, ptr addrspace(4) %sum.ascast, align 4, !dbg !36
  br label %omp.body.continue, !dbg !37

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !37

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4, !dbg !31
  %add2 = add nsw i32 %9, 1, !dbg !32
  store i32 %add2, ptr addrspace(4) %.omp.iv.ascast, align 4, !dbg !32
  br label %omp.inner.for.cond, !dbg !37, !llvm.loop !38

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !37

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
, !dbg !30
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
, !dbg !25
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
, !dbg !24
  ret i32 0, !dbg !40
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!4}
!llvm.module.flags = !{!5, !6, !7, !8, !9, !10, !11}
!opencl.compiler.options = !{!12}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, imports: !41, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.cpp", directory: "/tmp")
!2 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!3 = !{!2}

!4 = !{i32 0, i32 66313, i32 182591090, !"_Z4main", i32 6, i32 0, i32 0}
!5 = !{i32 7, !"Dwarf Version", i32 4}
!6 = !{i32 2, !"Debug Info Version", i32 3}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{i32 7, !"openmp", i32 50}
!9 = !{i32 7, !"openmp-device", i32 50}
!10 = !{i32 8, !"PIC Level", i32 2}
!11 = !{i32 7, !"frame-pointer", i32 2}
!12 = !{}
!13 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !14, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !12)
!14 = !DISubroutineType(types: !3)
!15 = !DILocalVariable(name: "i", scope: !13, file: !1, line: 4, type: !2)
!16 = !DILocation(line: 4, column: 8, scope: !13)
!17 = !DILocalVariable(name: "sum", scope: !13, file: !1, line: 4, type: !2)
!18 = !DILocation(line: 4, column: 11, scope: !13)
!19 = !DILocalVariable(name: ".omp.lb", scope: !20, type: !2, flags: DIFlagArtificial)
!20 = distinct !DILexicalBlock(scope: !13, file: !1, line: 6, column: 2)
!21 = !DILocation(line: 0, scope: !20)
!22 = !DILocation(line: 7, column: 9, scope: !20)
!23 = !DILocalVariable(name: ".omp.ub", scope: !20, type: !2, flags: DIFlagArtificial)
!24 = !DILocation(line: 6, column: 2, scope: !20)
!25 = !DILocation(line: 6, column: 2, scope: !26)
!26 = distinct !DILexicalBlock(scope: !20, file: !1, line: 6, column: 2)
!27 = !DILocalVariable(name: ".omp.iv", scope: !28, type: !2, flags: DIFlagArtificial)
!28 = distinct !DILexicalBlock(scope: !26, file: !1, line: 6, column: 2)
!29 = !DILocation(line: 0, scope: !28)
!30 = !DILocation(line: 6, column: 2, scope: !28)
!31 = !DILocation(line: 7, column: 9, scope: !28)
!32 = !DILocation(line: 7, column: 4, scope: !28)
!33 = !DILocation(line: 7, column: 24, scope: !28)
!34 = !DILocation(line: 8, column: 13, scope: !35)
!35 = distinct !DILexicalBlock(scope: !28, file: !1, line: 7, column: 29)
!36 = !DILocation(line: 8, column: 10, scope: !35)
!37 = !DILocation(line: 9, column: 4, scope: !35)
!38 = distinct !{!38, !30, !39}
!39 = !DILocation(line: 6, column: 83, scope: !28)
!40 = !DILocation(line: 11, column: 4, scope: !13)
!41 = !{}

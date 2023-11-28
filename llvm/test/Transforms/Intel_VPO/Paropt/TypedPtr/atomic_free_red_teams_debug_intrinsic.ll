; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s

; CHECK: call void @llvm.dbg.value(metadata i32 addrspace(1)* %red_buf

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@0 = private unnamed_addr addrspace(4) constant [20 x i8] c";sum;test.cpp;2;9;;\00", align 1

define protected noundef i32 @main() #0 !dbg !5 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum.ascast = addrspacecast i32* %sum to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  call void @llvm.dbg.declare(metadata i32* %sum, metadata !9, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !10
  store i32 0, i32 addrspace(4)* %sum.ascast, align 4, !dbg !10
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum.ascast, i32 addrspace(4)* %sum.ascast, i64 4, i64 33315, [20 x i8] addrspace(4)* @0, i8* null), ; MAP type: 33315 = 0x8223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1) | UNKNOWN (0x8000)
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(i32 addrspace(4)* %sum.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %tmp.ascast, i32 0, i32 1) ]
, !dbg !12
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.PRIVATE:TYPED"(i32 addrspace(4)* %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(i32 addrspace(4)* %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32 addrspace(4)* %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(i32 addrspace(4)* %.omp.ub.ascast, i32 0) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %8 = load i32, i32 addrspace(4)* %sum.ascast, align 4
  %add1 = add nsw i32 %8, %7
  store i32 %add1, i32 addrspace(4)* %sum.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

declare void @llvm.dbg.declare(metadata, metadata, metadata)

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { "contains-openmp-target"="true" }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!2}
!llvm.module.flags = !{!3, !4}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false)
!1 = !DIFile(filename: "test.cpp", directory: "")
!2 = !{i32 0, i32 2050, i32 49356524, !"_Z4main", i32 3, i32 0, i32 0, i32 0}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !6, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0)
!6 = !DISubroutineType(types: !7)
!7 = !{!8}
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DILocalVariable(name: "sum", scope: !5, file: !1, line: 2, type: !8)
!10 = !DILocation(line: 2, column: 9, scope: !5)
!11 = distinct !DILexicalBlock(scope: !5, file: !1, line: 3, column: 1)
!12 = !DILocation(line: 3, column: 1, scope: !13)
!13 = distinct !DILexicalBlock(scope: !11, file: !1, line: 3, column: 1)

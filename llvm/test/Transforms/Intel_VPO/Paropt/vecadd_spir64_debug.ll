; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; -- vecadd.cpp ---------------------------------------------------------------
; #include <stdio.h>
; #include <stdlib.h>
; #include <omp.h>
;
; void
; init (float *v1, float *v2, const int N)
; {
;   for (int i = 0; i < N; ++i)
;     {
;       v1[i] = 1.0f;
;       v2[i] = 1.0f;
;     }
; }
;
; void __attribute__ ((noinline))
; vecadd ()
; {
;    constexpr int N = 100;
;    float v3[N], v1[N], v2[N];
;    init (v1, v2, N);
;
;    int i; /* before-kernel-launch */
; #pragma omp target teams distribute parallel for map(to: v1, v2) map(from: v3)
;    for (i = 0; i < N; ++i)
;      {
;        v2[i] *= 2; /* kernel-line-1 */
;        v3[i] = v1[i] + v2[i]; /* kernel-line-2 */
;      }
; } /* after-kernel-launch */
;
; int
; main ()
; {
;   vecadd ();
;   return 0;
; }
; -- vecadd.cpp ---------------------------------------------------------------
;
; CHECK: define {{.*}}void @__omp_offloading_{{.*}}vecaddv{{.*}}([100 x float] addrspace(1)* %v1.ascast, [100 x float] addrspace(1)* %v2.ascast, [100 x float] addrspace(1)* noalias %v3.ascast, i32 addrspace(1)* %.omp.lb.ascast, i32 addrspace(1)* %.omp.ub.ascast, i32 addrspace(1)* %i.ascast) {{.*}} !dbg [[OFFLOAD:![0-9]+]] {
; CHECK-DAG: call void @llvm.dbg.value(metadata [100 x float] addrspace(1)* [[V1_STORAGE:%v1.ascast]], metadata [[V1_DIVAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[L36C17:![0-9]+]]
; CHECK-DAG: call void @llvm.dbg.value(metadata [100 x float] addrspace(1)* [[V2_STORAGE:%v2.ascast]], metadata [[V2_DIVAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[L36C24:![0-9]+]]
; CHECK-DAG: call void @llvm.dbg.value(metadata [100 x float] addrspace(1)* [[V3_STORAGE:%v3.ascast]], metadata [[V3_DIVAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[L36C10:![0-9]+]]
; CHECK-DAG: call void @llvm.dbg.value(metadata i32 addrspace(1)* [[I_STORAGE:%i.ascast]], metadata [[I_DIVAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[L39C8:![0-9]+]]
; CHECK: }
;
; CHECK-NOT: {{![0-9]+}} = distinct !DISubprogram(name: "vecadd"
; CHECK: [[UNIT:![0-9]+]] = {{.*}}!DICompileUnit({{.*}}file: [[FILE:![0-9]+]]
; CHECK: [[FILE]] = !DIFile(filename: "vecadd.cpp"{{.*}})
; CHECK: [[EMPTY:![0-9]+]] = !{}
; CHECK: [[OFFLOAD]] = distinct !DISubprogram(name: "_Z6vecaddv.DIR.OMP.TARGET.{{[0-9]+}}.split.split.split", scope: [[FILE]], file: [[FILE]], line: 40, type: [[OFFLOAD_TYPE:![0-9]+]], scopeLine: 40, flags: DIFlagArtificial, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: [[UNIT]], retainedNodes: [[EMPTY]])
; CHECK-DAG: [[OFFLOAD_TYPE]] = !DISubroutineType(types: [[EMPTY]])
; CHECK-DAG: [[V1_DIVAR]] = !DILocalVariable(name: "v1", scope: [[OFFLOAD]]
; CHECK-DAG: [[L36C17]] = !DILocation(line: 36, column: 17, scope: [[OFFLOAD]]
; CHECK-DAG: [[V2_DIVAR]] = !DILocalVariable(name: "v2", scope: [[OFFLOAD]]
; CHECK-DAG: [[L36C24]] = !DILocation(line: 36, column: 24, scope: [[OFFLOAD]]
; CHECK-DAG: [[V3_DIVAR]] = !DILocalVariable(name: "v3", scope: [[OFFLOAD]]
; CHECK-DAG: [[L36C10]] = !DILocation(line: 36, column: 10, scope: [[OFFLOAD]]
; CHECK-DAG: [[I_DIVAR]] = !DILocalVariable(name: "i", scope: [[OFFLOAD]]
; CHECK-DAG: [[L39C8]] = !DILocation(line: 39, column: 8, scope: [[OFFLOAD]]
;

; ModuleID = 'vecadd.cpp'
source_filename = "vecadd.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z6vecaddv() #0 !dbg !8 {
entry:
  %N = alloca i32, align 4
  %N.ascast = addrspacecast i32* %N to i32 addrspace(4)*
  %v3 = alloca [100 x float], align 4
  %v3.ascast = addrspacecast [100 x float]* %v3 to [100 x float] addrspace(4)*
  %v1 = alloca [100 x float], align 4
  %v1.ascast = addrspacecast [100 x float]* %v1 to [100 x float] addrspace(4)*
  %v2 = alloca [100 x float], align 4
  %v2.ascast = addrspacecast [100 x float]* %v2 to [100 x float] addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %N.ascast, metadata !11, metadata !DIExpression()), !dbg !14
  store i32 100, i32 addrspace(4)* %N.ascast, align 4, !dbg !14
  call void @llvm.dbg.declare(metadata [100 x float] addrspace(4)* %v3.ascast, metadata !15, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.declare(metadata [100 x float] addrspace(4)* %v1.ascast, metadata !21, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.declare(metadata [100 x float] addrspace(4)* %v2.ascast, metadata !23, metadata !DIExpression()), !dbg !24
  %arraydecay = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v1.ascast, i64 0, i64 0, !dbg !25
  %arraydecay1 = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v2.ascast, i64 0, i64 0, !dbg !26
  call spir_func void @_Z4initPfS_i(float addrspace(4)* %arraydecay, float addrspace(4)* %arraydecay1, i32 100), !dbg !27
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !28, metadata !DIExpression()), !dbg !29
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.lb.ascast, metadata !30, metadata !DIExpression()), !dbg !32
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !33
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.ub.ascast, metadata !34, metadata !DIExpression()), !dbg !32
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !33
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"([100 x float] addrspace(4)* %v1.ascast, [100 x float] addrspace(4)* %v1.ascast, i64 400, i64 33, i8* null, i8* null), "QUAL.OMP.MAP.TO"([100 x float] addrspace(4)* %v2.ascast, [100 x float] addrspace(4)* %v2.ascast, i64 400, i64 33, i8* null, i8* null), "QUAL.OMP.MAP.FROM"([100 x float] addrspace(4)* %v3.ascast, [100 x float] addrspace(4)* %v3.ascast, i64 400, i64 34, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !35
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v2.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v1.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ], !dbg !36
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %.omp.iv.ascast, metadata !37, metadata !DIExpression()), !dbg !40
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v2.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v1.ascast), "QUAL.OMP.SHARED"([100 x float] addrspace(4)* %v3.ascast) ], !dbg !41
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !dbg !42
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !42
  br label %omp.inner.for.cond, !dbg !41

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !42
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !dbg !42
  %cmp = icmp sle i32 %4, %5, !dbg !43
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !41

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !42
  %mul = mul nsw i32 %6, 1, !dbg !44
  %add = add nsw i32 0, %mul, !dbg !44
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !dbg !44
  %7 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !45
  %idxprom = sext i32 %7 to i64, !dbg !47
  %arrayidx = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v2.ascast, i64 0, i64 %idxprom, !dbg !47
  %8 = load float, float addrspace(4)* %arrayidx, align 4, !dbg !48
  %mul2 = fmul float %8, 2.000000e+00, !dbg !48
  store float %mul2, float addrspace(4)* %arrayidx, align 4, !dbg !48
  %9 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !49
  %idxprom3 = sext i32 %9 to i64, !dbg !50
  %arrayidx4 = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v1.ascast, i64 0, i64 %idxprom3, !dbg !50
  %10 = load float, float addrspace(4)* %arrayidx4, align 4, !dbg !50
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !51
  %idxprom5 = sext i32 %11 to i64, !dbg !52
  %arrayidx6 = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v2.ascast, i64 0, i64 %idxprom5, !dbg !52
  %12 = load float, float addrspace(4)* %arrayidx6, align 4, !dbg !52
  %add7 = fadd float %10, %12, !dbg !53
  %13 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !54
  %idxprom8 = sext i32 %13 to i64, !dbg !55
  %arrayidx9 = getelementptr inbounds [100 x float], [100 x float] addrspace(4)* %v3.ascast, i64 0, i64 %idxprom8, !dbg !55
  store float %add7, float addrspace(4)* %arrayidx9, align 4, !dbg !56
  br label %omp.body.continue, !dbg !57

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !58

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !42
  %add10 = add nsw i32 %14, 1, !dbg !43
  store i32 %add10, i32 addrspace(4)* %.omp.iv.ascast, align 4, !dbg !43
  br label %omp.inner.for.cond, !dbg !58, !llvm.loop !59

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !58

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ], !dbg !41
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ], !dbg !36
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !35
  ret void, !dbg !61

; uselistorder directives
  uselistorder i32 addrspace(4)* %tmp.ascast, { 1, 0 }
  uselistorder i32 addrspace(4)* %.omp.iv.ascast, { 3, 4, 5, 6, 7, 2, 1, 0 }
  uselistorder i32 addrspace(4)* %.omp.ub.ascast, { 3, 2, 1, 0, 4 }
  uselistorder i32 addrspace(4)* %.omp.lb.ascast, { 3, 2, 1, 0, 4 }
  uselistorder i32 addrspace(4)* %i.ascast, { 3, 4, 5, 6, 7, 2, 1, 0 }
  uselistorder token ()* @llvm.directive.region.entry, { 2, 1, 0 }
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @_Z4initPfS_i(float addrspace(4)* %v1, float addrspace(4)* %v2, i32 %N) #2 !dbg !62 {
entry:
  %v1.addr = alloca float addrspace(4)*, align 8
  %v1.addr.ascast = addrspacecast float addrspace(4)** %v1.addr to float addrspace(4)* addrspace(4)*
  %v2.addr = alloca float addrspace(4)*, align 8
  %v2.addr.ascast = addrspacecast float addrspace(4)** %v2.addr to float addrspace(4)* addrspace(4)*
  %N.addr = alloca i32, align 4
  %N.addr.ascast = addrspacecast i32* %N.addr to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store float addrspace(4)* %v1, float addrspace(4)* addrspace(4)* %v1.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata float addrspace(4)* addrspace(4)* %v1.addr.ascast, metadata !66, metadata !DIExpression()), !dbg !67
  store float addrspace(4)* %v2, float addrspace(4)* addrspace(4)* %v2.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata float addrspace(4)* addrspace(4)* %v2.addr.ascast, metadata !68, metadata !DIExpression()), !dbg !69
  store i32 %N, i32 addrspace(4)* %N.addr.ascast, align 4
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %N.addr.ascast, metadata !70, metadata !DIExpression()), !dbg !71
  call void @llvm.dbg.declare(metadata i32 addrspace(4)* %i.ascast, metadata !72, metadata !DIExpression()), !dbg !74
  store i32 0, i32 addrspace(4)* %i.ascast, align 4, !dbg !74
  br label %for.cond, !dbg !75

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !76
  %1 = load i32, i32 addrspace(4)* %N.addr.ascast, align 4, !dbg !78
  %cmp = icmp slt i32 %0, %1, !dbg !79
  br i1 %cmp, label %for.body, label %for.end, !dbg !80

for.body:                                         ; preds = %for.cond
  %2 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %v1.addr.ascast, align 8, !dbg !81
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !83
  %idxprom = sext i32 %3 to i64, !dbg !81
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %2, i64 %idxprom, !dbg !81
  store float 1.000000e+00, float addrspace(4)* %arrayidx, align 4, !dbg !84
  %4 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %v2.addr.ascast, align 8, !dbg !85
  %5 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !86
  %idxprom1 = sext i32 %5 to i64, !dbg !85
  %arrayidx2 = getelementptr inbounds float, float addrspace(4)* %4, i64 %idxprom1, !dbg !85
  store float 1.000000e+00, float addrspace(4)* %arrayidx2, align 4, !dbg !87
  br label %for.inc, !dbg !88

for.inc:                                          ; preds = %for.body
  %6 = load i32, i32 addrspace(4)* %i.ascast, align 4, !dbg !89
  %inc = add nsw i32 %6, 1, !dbg !89
  store i32 %inc, i32 addrspace(4)* %i.ascast, align 4, !dbg !89
  br label %for.cond, !dbg !90, !llvm.loop !91

for.end:                                          ; preds = %for.cond
  ret void, !dbg !93

; uselistorder directives
  uselistorder i32 0, { 3, 1, 0, 2 }
  uselistorder void (metadata, metadata, metadata)* @llvm.dbg.declare, { 4, 5, 6, 7, 0, 1, 2, 3, 8, 9, 10, 11 }
  uselistorder i32 1, { 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5, 6, 12, 13, 14, 15 }
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable "contains-openmp-target"="true" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5, !6}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang based DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "vecadd.cpp", directory: "/path/to")
!2 = !{}
!3 = !{i32 0, i32 2055, i32 92146858, !"_Z6vecaddv", i32 40, i32 0, i32 0}
!4 = !{i32 2, !"Dwarf Version", i32 4}
!5 = !{i32 2, !"Debug Info Version", i32 3}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"DPC++ Compiler 2021.1 (YYYY.8.x.0.MMDD)"}
!8 = distinct !DISubprogram(name: "vecadd", linkageName: "_Z6vecaddv", scope: !1, file: !1, line: 33, type: !9, scopeLine: 34, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !10)
!10 = !{null}
!11 = !DILocalVariable(name: "N", scope: !8, file: !1, line: 35, type: !12)
!12 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !13)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocation(line: 35, column: 18, scope: !8)
!15 = !DILocalVariable(name: "v3", scope: !8, file: !1, line: 36, type: !16)
!16 = !DICompositeType(tag: DW_TAG_array_type, baseType: !17, size: 3200, elements: !18)
!17 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!18 = !{!19}
!19 = !DISubrange(count: 100)
!20 = !DILocation(line: 36, column: 10, scope: !8)
!21 = !DILocalVariable(name: "v1", scope: !8, file: !1, line: 36, type: !16)
!22 = !DILocation(line: 36, column: 17, scope: !8)
!23 = !DILocalVariable(name: "v2", scope: !8, file: !1, line: 36, type: !16)
!24 = !DILocation(line: 36, column: 24, scope: !8)
!25 = !DILocation(line: 37, column: 10, scope: !8)
!26 = !DILocation(line: 37, column: 14, scope: !8)
!27 = !DILocation(line: 37, column: 4, scope: !8)
!28 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 39, type: !13)
!29 = !DILocation(line: 39, column: 8, scope: !8)
!30 = !DILocalVariable(name: ".omp.lb", scope: !31, type: !13, flags: DIFlagArtificial)
!31 = distinct !DILexicalBlock(scope: !8, file: !1, line: 40, column: 1)
!32 = !DILocation(line: 0, scope: !31)
!33 = !DILocation(line: 41, column: 9, scope: !31)
!34 = !DILocalVariable(name: ".omp.ub", scope: !31, type: !13, flags: DIFlagArtificial)
!35 = !DILocation(line: 40, column: 1, scope: !8)
!36 = !DILocation(line: 40, column: 1, scope: !31)
!37 = !DILocalVariable(name: ".omp.iv", scope: !38, type: !13, flags: DIFlagArtificial)
!38 = distinct !DILexicalBlock(scope: !39, file: !1, line: 40, column: 1)
!39 = distinct !DILexicalBlock(scope: !31, file: !1, line: 40, column: 1)
!40 = !DILocation(line: 0, scope: !38)
!41 = !DILocation(line: 40, column: 1, scope: !39)
!42 = !DILocation(line: 41, column: 9, scope: !38)
!43 = !DILocation(line: 41, column: 4, scope: !38)
!44 = !DILocation(line: 41, column: 23, scope: !38)
!45 = !DILocation(line: 43, column: 11, scope: !46)
!46 = distinct !DILexicalBlock(scope: !38, file: !1, line: 42, column: 6)
!47 = !DILocation(line: 43, column: 8, scope: !46)
!48 = !DILocation(line: 43, column: 14, scope: !46)
!49 = !DILocation(line: 44, column: 19, scope: !46)
!50 = !DILocation(line: 44, column: 16, scope: !46)
!51 = !DILocation(line: 44, column: 27, scope: !46)
!52 = !DILocation(line: 44, column: 24, scope: !46)
!53 = !DILocation(line: 44, column: 22, scope: !46)
!54 = !DILocation(line: 44, column: 11, scope: !46)
!55 = !DILocation(line: 44, column: 8, scope: !46)
!56 = !DILocation(line: 44, column: 14, scope: !46)
!57 = !DILocation(line: 45, column: 6, scope: !46)
!58 = !DILocation(line: 40, column: 1, scope: !38)
!59 = distinct !{!59, !58, !60}
!60 = !DILocation(line: 40, column: 79, scope: !38)
!61 = !DILocation(line: 46, column: 1, scope: !8)
!62 = distinct !DISubprogram(name: "init", linkageName: "_Z4initPfS_i", scope: !1, file: !1, line: 23, type: !63, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!63 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !64)
!64 = !{null, !65, !65, !12}
!65 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!66 = !DILocalVariable(name: "v1", arg: 1, scope: !62, file: !1, line: 23, type: !65)
!67 = !DILocation(line: 23, column: 14, scope: !62)
!68 = !DILocalVariable(name: "v2", arg: 2, scope: !62, file: !1, line: 23, type: !65)
!69 = !DILocation(line: 23, column: 25, scope: !62)
!70 = !DILocalVariable(name: "N", arg: 3, scope: !62, file: !1, line: 23, type: !12)
!71 = !DILocation(line: 23, column: 39, scope: !62)
!72 = !DILocalVariable(name: "i", scope: !73, file: !1, line: 25, type: !13)
!73 = distinct !DILexicalBlock(scope: !62, file: !1, line: 25, column: 3)
!74 = !DILocation(line: 25, column: 12, scope: !73)
!75 = !DILocation(line: 25, column: 8, scope: !73)
!76 = !DILocation(line: 25, column: 19, scope: !77)
!77 = distinct !DILexicalBlock(scope: !73, file: !1, line: 25, column: 3)
!78 = !DILocation(line: 25, column: 23, scope: !77)
!79 = !DILocation(line: 25, column: 21, scope: !77)
!80 = !DILocation(line: 25, column: 3, scope: !73)
!81 = !DILocation(line: 27, column: 7, scope: !82)
!82 = distinct !DILexicalBlock(scope: !77, file: !1, line: 26, column: 5)
!83 = !DILocation(line: 27, column: 10, scope: !82)
!84 = !DILocation(line: 27, column: 13, scope: !82)
!85 = !DILocation(line: 28, column: 7, scope: !82)
!86 = !DILocation(line: 28, column: 10, scope: !82)
!87 = !DILocation(line: 28, column: 13, scope: !82)
!88 = !DILocation(line: 29, column: 5, scope: !82)
!89 = !DILocation(line: 25, column: 26, scope: !77)
!90 = !DILocation(line: 25, column: 3, scope: !77)
!91 = distinct !{!91, !80, !92}
!92 = !DILocation(line: 29, column: 5, scope: !73)
!93 = !DILocation(line: 30, column: 1, scope: !62)

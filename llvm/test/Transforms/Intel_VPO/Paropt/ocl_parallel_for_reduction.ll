; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -simplifycfg  -sroa -vpo-cfg-restructuring -vpo-paropt -simplifycfg -switch-to-offload -S -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform 2>&1 | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,simplify-cfg,loop(simplify-cfg),sroa,vpo-cfg-restructuring),vpo-paropt,function(simplify-cfg)' -switch-to-offload -S -pass-remarks=vpo-paropt-transform -pass-remarks-missed=vpo-paropt-transform 2>&1 | FileCheck %s

; Original code:
; int test()
; {
;   int r1 = 0, r2 = 0, r3 = 1, r4 = -1, r5 = 0, r6 = 0, r7 = 1, r8 = 0, r9 = 0, r10 = 0;
; #pragma omp target map(r1,r2,r3,r4,r5,r6,r7,r8,r9,r10)
; #pragma omp parallel for reduction(+: r1) reduction(-: r2) reduction(*: r3) reduction(&: r4) reduction(|: r5) reduction(^: r6) reduction(&&: r7) reduction(||: r8) reduction(min: r9) reduction(max: r10)
;   for (int i = 0; i < 128; i++);
;
;   return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
; }

; CHECK-DAG: ADD reduction update of type i32 made atomic
; CHECK-DAG: SUB reduction update of type i32 made atomic
; CHECK-DAG: MUL reduction update of type i32 made atomic
; CHECK-DAG: BAND reduction update of type i32 made atomic
; CHECK-DAG: BOR reduction update of type i32 made atomic
; CHECK-DAG: BXOR reduction update of type i32 made atomic
; FIXME: we need runtime support for these reduction operations OR
;        we have to encode compare-exchange loop in IR.
; CHECK-DAG: AND reduction update of type i32 cannot be done using atomic API
; CHECK-DAG: OR reduction update of type i32 cannot be done using atomic API
; CHECK-DAG: MIN reduction update of type i32 cannot be done using atomic API
; CHECK-DAG: MAX reduction update of type i32 cannot be done using atomic API
; CHECK: Critical section was generated for reduction update(s)

; CHECK: define dso_local spir_kernel void @__omp_offloading_
; CHECK-DAG: call void @__kmpc_atomic_fixed4_add
; CHECK-DAG: call void @__kmpc_atomic_fixed4_add
; CHECK-DAG: call void @__kmpc_atomic_fixed4_mul
; CHECK-DAG: call void @__kmpc_atomic_fixed4_andb
; CHECK-DAG: call void @__kmpc_atomic_fixed4_orb
; CHECK-DAG: call void @__kmpc_atomic_fixed4_xor

; ModuleID = 'reduc.c'
source_filename = "reduc.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func i32 @test() #0 !dbg !8 {
entry:
  %r1 = alloca i32, align 4
  %r2 = alloca i32, align 4
  %r3 = alloca i32, align 4
  %r4 = alloca i32, align 4
  %r5 = alloca i32, align 4
  %r6 = alloca i32, align 4
  %r7 = alloca i32, align 4
  %r8 = alloca i32, align 4
  %r9 = alloca i32, align 4
  %r10 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %r1, align 4, !dbg !10
  store i32 0, i32* %r2, align 4, !dbg !10
  store i32 1, i32* %r3, align 4, !dbg !10
  store i32 -1, i32* %r4, align 4, !dbg !10
  store i32 0, i32* %r5, align 4, !dbg !10
  store i32 0, i32* %r6, align 4, !dbg !10
  store i32 1, i32* %r7, align 4, !dbg !10
  store i32 0, i32* %r8, align 4, !dbg !10
  store i32 0, i32* %r9, align 4, !dbg !10
  store i32 0, i32* %r10, align 4, !dbg !10
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %r1), "QUAL.OMP.MAP.TOFROM"(i32* %r2), "QUAL.OMP.MAP.TOFROM"(i32* %r3), "QUAL.OMP.MAP.TOFROM"(i32* %r4), "QUAL.OMP.MAP.TOFROM"(i32* %r5), "QUAL.OMP.MAP.TOFROM"(i32* %r6), "QUAL.OMP.MAP.TOFROM"(i32* %r7), "QUAL.OMP.MAP.TOFROM"(i32* %r8), "QUAL.OMP.MAP.TOFROM"(i32* %r9), "QUAL.OMP.MAP.TOFROM"(i32* %r10), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ], !dbg !11
  store i32 0, i32* %.omp.lb, align 4, !dbg !12
  store i32 127, i32* %.omp.ub, align 4, !dbg !12
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %r1), "QUAL.OMP.REDUCTION.SUB"(i32* %r2), "QUAL.OMP.REDUCTION.MUL"(i32* %r3), "QUAL.OMP.REDUCTION.BAND"(i32* %r4), "QUAL.OMP.REDUCTION.BOR"(i32* %r5), "QUAL.OMP.REDUCTION.BXOR"(i32* %r6), "QUAL.OMP.REDUCTION.AND"(i32* %r7), "QUAL.OMP.REDUCTION.OR"(i32* %r8), "QUAL.OMP.REDUCTION.MIN"(i32* %r9), "QUAL.OMP.REDUCTION.MAX"(i32* %r10), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ], !dbg !13
  %2 = load i32, i32* %.omp.lb, align 4, !dbg !12
  store i32 %2, i32* %.omp.iv, align 4, !dbg !12
  br label %omp.inner.for.cond, !dbg !13

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4, !dbg !12
  %4 = load i32, i32* %.omp.ub, align 4, !dbg !12
  %cmp = icmp sle i32 %3, %4, !dbg !12
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end, !dbg !13

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4, !dbg !12
  %mul = mul nsw i32 %5, 1, !dbg !12
  %add = add nsw i32 0, %mul, !dbg !12
  store i32 %add, i32* %i, align 4, !dbg !12
  br label %omp.body.continue, !dbg !13

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc, !dbg !13

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4, !dbg !12
  %add1 = add nsw i32 %6, 1, !dbg !12
  store i32 %add1, i32* %.omp.iv, align 4, !dbg !12
  br label %omp.inner.for.cond, !dbg !13, !llvm.loop !14

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit, !dbg !13

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ], !dbg !13
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !11
  %7 = load i32, i32* %r1, align 4, !dbg !15
  %8 = load i32, i32* %r2, align 4, !dbg !15
  %add2 = add nsw i32 %7, %8, !dbg !15
  %9 = load i32, i32* %r3, align 4, !dbg !15
  %add3 = add nsw i32 %add2, %9, !dbg !15
  %10 = load i32, i32* %r4, align 4, !dbg !15
  %add4 = add nsw i32 %add3, %10, !dbg !15
  %11 = load i32, i32* %r5, align 4, !dbg !15
  %add5 = add nsw i32 %add4, %11, !dbg !15
  %12 = load i32, i32* %r6, align 4, !dbg !15
  %add6 = add nsw i32 %add5, %12, !dbg !15
  %13 = load i32, i32* %r7, align 4, !dbg !15
  %add7 = add nsw i32 %add6, %13, !dbg !15
  %14 = load i32, i32* %r8, align 4, !dbg !15
  %add8 = add nsw i32 %add7, %14, !dbg !15
  %15 = load i32, i32* %r9, align 4, !dbg !15
  %add9 = add nsw i32 %add8, %15, !dbg !15
  %16 = load i32, i32* %r10, align 4, !dbg !15
  %add10 = add nsw i32 %add9, %16, !dbg !15
  ret i32 %add10, !dbg !15
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!3}
!llvm.module.flags = !{!4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang based icx (ICX) 2019.8.2.0", isOptimized: false, runtimeVersion: 0, emissionKind: NoDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "reduc.c", directory: "")
!2 = !{}
!3 = !{i32 0, i32 2052, i32 85985690, !"test", i32 6, i32 0, i32 0}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 9.0.0"}
!8 = distinct !DISubprogram(name: "test", scope: !1, file: !1, line: 3, type: !9, scopeLine: 4, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !2)
!10 = !DILocation(line: 5, scope: !8)
!11 = !DILocation(line: 6, scope: !8)
!12 = !DILocation(line: 8, scope: !8)
!13 = !DILocation(line: 7, scope: !8)
!14 = distinct !{!14, !13, !13}
!15 = !DILocation(line: 10, scope: !8)

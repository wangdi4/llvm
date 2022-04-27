; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -sroa -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,loop-simplify,sroa,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Verify that the reference to '@a' is seen during the outlining
; of the target region, when device_triples contain SPIRV,
; so that extraction for host and SPIRV follows the same pattern.
; If the reference to '@a' is not present after PARALLEL.LOOP
; outlining on the host, then it will not be represented
; as an argument to the target outline function.  At the same time,
; the reference will be seen during target outlining in SPIRV
; compilation, and will have the corresponding outlined function
; argument.  This causes interface mismatch for the target outline
; function on the host and SPIRV target.
; Original code:
; int a[100];
;
; void foo() {
;   int i;
; #pragma omp target map(a)
; #pragma omp parallel for
;   for (i = 0; i < 100; ++i) {
;     a[i] = 0;
;   }
; }

; CHECK-DAG: define internal void @__omp_offloading_804_52009c5_foo_l5([100 x i32]*{{ *%[^,]*}}, i32*{{ *%[^,]*}})
; In addition, check that the outlined function for PARALLEL.LOOP does not
; take 'a' by a pointer argument, because it may access it as a global variable.
; CHECK-DAG: define internal void @foo.DIR.OMP.PARALLEL.LOOP{{.*}}(i32*{{ *%[^,]*}}, i32*{{ *%[^,]*}}, [100 x i32]*{{ *%[^,]*}}, i32*{{ *%[^,]*}}, i32*{{ *%[^,]*}})

source_filename = "target.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@a = common dso_local global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([100 x i32]* @a, [100 x i32]* @a, i64 400, i64 35, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %.omp.iv) ]
; %.omp.iv was manually added to PRIVATE clause above.  This will be done automatically in FE,
; when omp.iv privatization for the inner region is enabled in Paropt.
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !3
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !3
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([100 x i32]* @a) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !3
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !3
  %10 = load i32, i32* %i, align 4, !tbaa !3
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %idxprom, !intel-tbaa !7
  store i32 0, i32* %arrayidx, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %add1 = add nsw i32 %11, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %12 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2052, i32 85985733, !"foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA100_i", !4, i64 0}

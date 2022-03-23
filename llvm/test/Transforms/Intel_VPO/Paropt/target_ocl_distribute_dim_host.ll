; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp target parallel for collapse(3)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j)
;       for (int k = 0; k < n; ++k);
; }

; Check that ND-range is created for the runtime:
; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i32, i32, i64, i64, i64, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i32 2, i32* [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 1, i32* [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, i64* [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: store i64 0, i64* [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, i64* [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: store i64 0, i64* [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i64, i64* [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i64 [[UB1]], i64* [[M6]]
; CHECK: [[M7:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 7
; CHECK: store i64 1, i64* [[M7]]
; CHECK: call void @__omp_offloading_806_1c1e7b__Z3foo_l2({{.*}}i64* [[UBPTR1]]{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.lb1 = alloca i32, align 4
  %.omp.uncollapsed.ub2 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv4 = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp10 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %k = alloca i32, align 4
  %0 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  store i32 0, i32* %.omp.uncollapsed.lb, align 4, !tbaa !3
  %1 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  store i32 76, i32* %.omp.uncollapsed.ub, align 4, !tbaa !3
  %2 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  store i32 0, i32* %.omp.uncollapsed.lb1, align 4, !tbaa !3
  %3 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  store i32 98, i32* %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv4), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub2), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp3), "QUAL.OMP.PRIVATE"(i32* %tmp10) ]
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv), "QUAL.OMP.SHARED"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.SHARED"(i32* %.omp.uncollapsed.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv4), "QUAL.OMP.SHARED"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.SHARED"(i32* %.omp.uncollapsed.ub2), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp3), "QUAL.OMP.PRIVATE"(i32* %tmp10) ]
  %6 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  %7 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.uncollapsed.iv, i32* %.omp.uncollapsed.iv4), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.uncollapsed.ub, i32* %.omp.uncollapsed.ub2), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.PRIVATE"(i32* %tmp10) ]
  %9 = load i32, i32* %.omp.uncollapsed.lb, align 4, !tbaa !3
  store i32 %9, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc17, %entry
  %10 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %11 = load i32, i32* %.omp.uncollapsed.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end19

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %12 = load i32, i32* %.omp.uncollapsed.lb1, align 4, !tbaa !3
  store i32 %12, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %13 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %14 = load i32, i32* %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %cmp6 = icmp sle i32 %13, %14
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #1
  %16 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #1
  %17 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %17, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !3
  %18 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %mul8 = mul nsw i32 %18, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32* %j, align 4, !tbaa !3
  %19 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %19) #1
  %20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %20) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !3
  %21 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %21) #1
  store i32 31, i32* %.omp.ub, align 4, !tbaa !3
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %k) ]
  %23 = load i32, i32* %.omp.lb, align 4, !tbaa !3
  store i32 %23, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.uncollapsed.loop.body7
  %24 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %25 = load i32, i32* %.omp.ub, align 4, !tbaa !3
  %cmp11 = icmp sle i32 %24, %25
  br i1 %cmp11, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %26 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %26) #1
  %27 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %mul12 = mul nsw i32 %27, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, i32* %k, align 4, !tbaa !3
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %28 = bitcast i32* %k to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %29 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %add14 = add nsw i32 %29, 1
  store i32 %add14, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %30 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #1
  %31 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %31) #1
  %32 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #1
  br label %omp.body.continue15

omp.body.continue15:                              ; preds = %omp.loop.exit
  %33 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %33) #1
  %34 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %34) #1
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue15
  %35 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %add16 = add nsw i32 %35, 1
  store i32 %add16, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc17

omp.uncollapsed.loop.inc17:                       ; preds = %omp.uncollapsed.loop.end
  %36 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %add18 = add nsw i32 %36, 1
  store i32 %add18, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end19:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE"() ]
  %37 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %37) #1
  %38 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %38) #1
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  %39 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %39) #1
  %40 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %40) #1
  %41 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %41) #1
  %42 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define internal void @.omp_offloading.requires_reg() #3 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

declare dso_local void @__tgt_register_requires(i64)

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2054, i32 1842811, !"_Z3foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
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
; CHECK: store i32 2, ptr [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i32 1, ptr [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: store i64 0, ptr [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: store i64 0, ptr [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 1, ptr [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: store i64 0, ptr [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i64, ptr [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i64 [[UB1]], ptr [[M6]]
; CHECK: [[M7:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 7
; CHECK: store i64 1, ptr [[M7]]
; CHECK: call void @__omp_offloading_806_1c1e7b__Z3foo_l2({{.*}}ptr [[UBPTR1]]{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 0, ptr @.omp_offloading.requires_reg, ptr null }]

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
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.lb) #1
  store i32 0, ptr %.omp.uncollapsed.lb, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.ub) #1
  store i32 76, ptr %.omp.uncollapsed.ub, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.lb1) #1
  store i32 0, ptr %.omp.uncollapsed.lb1, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.ub2) #1
  store i32 98, ptr %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
 "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv4, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.ub2, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp3, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp10, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.uncollapsed.iv4, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
 "QUAL.OMP.SHARED:TYPED"(ptr %.omp.uncollapsed.ub2, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp3, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp10, i32 0, i32 1) ]

  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.uncollapsed.iv4) #1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
 "QUAL.OMP.COLLAPSE"(i32 2),
 "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.uncollapsed.iv, i32 0, ptr %.omp.uncollapsed.iv4, i32 0),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb, i32 0, i32 1),
 "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.uncollapsed.ub, i32 0, ptr %.omp.uncollapsed.ub2, i32 0),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.uncollapsed.lb1, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp10, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.uncollapsed.lb, align 4, !tbaa !3
  store i32 %3, ptr %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc17, %entry
  %4 = load i32, ptr %.omp.uncollapsed.iv, align 4, !tbaa !3
  %5 = load i32, ptr %.omp.uncollapsed.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end19

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %6 = load i32, ptr %.omp.uncollapsed.lb1, align 4, !tbaa !3
  store i32 %6, ptr %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %7 = load i32, ptr %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %8 = load i32, ptr %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %cmp6 = icmp sle i32 %7, %8
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #1
  %9 = load i32, ptr %.omp.uncollapsed.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4, !tbaa !3
  %10 = load i32, ptr %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %mul8 = mul nsw i32 %10, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, ptr %j, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 31, ptr %.omp.ub, align 4, !tbaa !3
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
 "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
 "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
 "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %k, i32 0, i32 1) ]

  %12 = load i32, ptr %.omp.lb, align 4, !tbaa !3
  store i32 %12, ptr %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.uncollapsed.loop.body7
  %13 = load i32, ptr %.omp.iv, align 4, !tbaa !3
  %14 = load i32, ptr %.omp.ub, align 4, !tbaa !3
  %cmp11 = icmp sle i32 %13, %14
  br i1 %cmp11, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %k) #1
  %15 = load i32, ptr %.omp.iv, align 4, !tbaa !3
  %mul12 = mul nsw i32 %15, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, ptr %k, align 4, !tbaa !3
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr %k) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4, !tbaa !3
  %add14 = add nsw i32 %16, 1
  store i32 %add14, ptr %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  br label %omp.body.continue15

omp.body.continue15:                              ; preds = %omp.loop.exit
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #1
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue15
  %17 = load i32, ptr %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %add16 = add nsw i32 %17, 1
  store i32 %add16, ptr %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc17

omp.uncollapsed.loop.inc17:                       ; preds = %omp.uncollapsed.loop.end
  %18 = load i32, ptr %.omp.uncollapsed.iv, align 4, !tbaa !3
  %add18 = add nsw i32 %18, 1
  store i32 %add18, ptr %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end19:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.iv4) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.iv) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.ub2) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.lb1) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.uncollapsed.lb) #1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

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

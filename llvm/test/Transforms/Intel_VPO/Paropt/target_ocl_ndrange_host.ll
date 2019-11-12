; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Original code:
; void foo() {
; #pragma omp target parallel for collapse(2)
;   for (int i = 0; i <= 19; ++i)
;     for (int j = 0; j <= 23; ++j);
; }

; CHECK: store i32 19, i32* [[UBPTR1:%[a-zA-Z._0-9]+]]
; CHECK: store i32 23, i32* [[UBPTR2:%[a-zA-Z._0-9]+]]
; CHECK: [[NDDESC:%[a-zA-Z._0-9]+]] = alloca { i64, i64, i64, i64, i64, i64, i64 }
; CHECK: [[M0:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 0
; CHECK: store i64 2, i64* [[M0]]
; CHECK: [[M1:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 1
; CHECK: store i64 0, i64* [[M1]]
; CHECK: [[M2:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 2
; CHECK: [[UB1:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBPTR1]]
; CHECK: [[UB1_:%[a-zA-Z._0-9]+]] = sext i32 [[UB1]] to i64
; CHECK: store i64 [[UB1_]], i64* [[M2]]
; CHECK: [[M3:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 3
; CHECK: store i64 1, i64* [[M3]]
; CHECK: [[M4:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 4
; CHECK: store i64 0, i64* [[M4]]
; CHECK: [[M5:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 5
; CHECK: [[UB2:%[a-zA-Z._0-9]+]] = load i32, i32* [[UBPTR2]]
; CHECK: [[UB2_:%[a-zA-Z._0-9]+]] = sext i32 [[UB2]] to i64
; CHECK: store i64 [[UB2_]], i64* [[M5]]
; CHECK: [[M6:%[a-zA-Z._0-9]+]] = getelementptr inbounds{{.*}} [[NDDESC]], i32 0, i32 6
; CHECK: store i64 1, i64* [[M6]]

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
  %0 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #1
  store i32 0, i32* %.omp.uncollapsed.lb, align 4, !tbaa !3
  %1 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  store i32 19, i32* %.omp.uncollapsed.ub, align 4, !tbaa !3
  %2 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  store i32 0, i32* %.omp.uncollapsed.lb1, align 4, !tbaa !3
  %3 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  store i32 23, i32* %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.uncollapsed.iv4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.ub2), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp3) ]
  %5 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  %6 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.uncollapsed.iv, i32* %.omp.uncollapsed.iv4), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.uncollapsed.ub, i32* %.omp.uncollapsed.ub2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.uncollapsed.lb1), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j) ]
  %8 = load i32, i32* %.omp.uncollapsed.lb, align 4, !tbaa !3
  store i32 %8, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc11, %entry
  %9 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %10 = load i32, i32* %.omp.uncollapsed.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end13

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %11 = load i32, i32* %.omp.uncollapsed.lb1, align 4, !tbaa !3
  store i32 %11, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.cond5:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %12 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %13 = load i32, i32* %.omp.uncollapsed.ub2, align 4, !tbaa !3
  %cmp6 = icmp sle i32 %12, %13
  br i1 %cmp6, label %omp.uncollapsed.loop.body7, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body7:                       ; preds = %omp.uncollapsed.loop.cond5
  %14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %14) #1
  %15 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #1
  %16 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %16, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !3
  %17 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %mul8 = mul nsw i32 %17, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32* %j, align 4, !tbaa !3
  %18 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #1
  %19 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body7
  %20 = load i32, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  %add10 = add nsw i32 %20, 1
  store i32 %add10, i32* %.omp.uncollapsed.iv4, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond5

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond5
  br label %omp.uncollapsed.loop.inc11

omp.uncollapsed.loop.inc11:                       ; preds = %omp.uncollapsed.loop.end
  %21 = load i32, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  %add12 = add nsw i32 %21, 1
  store i32 %add12, i32* %.omp.uncollapsed.iv, align 4, !tbaa !3
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end13:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %22 = bitcast i32* %.omp.uncollapsed.iv4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  %23 = bitcast i32* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23) #1
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]
  %24 = bitcast i32* %.omp.uncollapsed.ub2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #1
  %25 = bitcast i32* %.omp.uncollapsed.lb1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #1
  %26 = bitcast i32* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #1
  %27 = bitcast i32* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #1
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

!0 = !{i32 0, i32 2052, i32 85987529, !"foo", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

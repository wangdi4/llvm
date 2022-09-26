; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "loop" construct is mapped to "simd"
; after prepare pass.

; int aaa[1000];
; void foo() {
; #pragma omp target teams distribute parallel for
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }



; ModuleID = 'generic_loop_in_dist_parfor.c'
source_filename = "generic_loop_in_dist_parfor.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.iv2 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1000 x i32]* @aaa, [1000 x i32]* @aaa, i64 4000, i64 547), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.iv2), "QUAL.OMP.PRIVATE"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %.omp.iv2), "QUAL.OMP.PRIVATE"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp1) ]
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !3
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #1
  store i32 999, i32* %.omp.ub, align 4, !tbaa !3
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.iv2), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp1) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !3
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc14, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end16

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #1
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !3
  %11 = bitcast i32* %.omp.iv2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #1
  %12 = bitcast i32* %.omp.lb3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #1
  store i32 0, i32* %.omp.lb3, align 4, !tbaa !3
  %13 = bitcast i32* %.omp.ub4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #1
  store i32 99, i32* %.omp.ub4, align 4, !tbaa !3

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.SIMD
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
; CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"
; CHECK-NOT: "QUAL.OMP.SHARED"
; CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}), "QUAL.OMP.PRIVATE"({{.*}})

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb3), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv2), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  %15 = load i32, i32* %.omp.lb3, align 4, !tbaa !3
  store i32 %15, i32* %.omp.iv2, align 4, !tbaa !3
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %16 = load i32, i32* %.omp.iv2, align 4, !tbaa !3
  %17 = load i32, i32* %.omp.ub4, align 4, !tbaa !3
  %cmp6 = icmp sle i32 %16, %17
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %18 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %18) #1
  %19 = load i32, i32* %.omp.iv2, align 4, !tbaa !3
  %mul8 = mul nsw i32 %19, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32* %j, align 4, !tbaa !3
  %20 = load i32, i32* %i, align 4, !tbaa !3
  %21 = load i32, i32* %j, align 4, !tbaa !3
  %add10 = add nsw i32 %20, %21
  %22 = load i32, i32* %i, align 4, !tbaa !3
  %idxprom = sext i32 %22 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !7
  %23 = load i32, i32* %arrayidx, align 4, !tbaa !7
  %add11 = add nsw i32 %23, %add10
  store i32 %add11, i32* %arrayidx, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  %24 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %25 = load i32, i32* %.omp.iv2, align 4, !tbaa !3
  %add12 = add nsw i32 %25, 1
  store i32 %add12, i32* %.omp.iv2, align 4, !tbaa !3
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.SIMD"() {{.*}}

  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.GENERICLOOP"() ]
  %26 = bitcast i32* %.omp.ub4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #1
  %27 = bitcast i32* %.omp.lb3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #1
  %28 = bitcast i32* %.omp.iv2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #1
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.loop.exit
  %29 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #1
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %30 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %add15 = add nsw i32 %30, 1
  store i32 %add15, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  %31 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %31) #1
  %32 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #1
  %33 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %33) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2052, i32 41162780, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_i", !4, i64 0}

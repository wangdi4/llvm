; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "loop" construct is mapped to "simd"
; after prepare pass.

; int aaa[1000];
; void foo() {
; #pragma omp for
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }



; ModuleID = 'generic_loop_in_for.c'
source_filename = "generic_loop_in_for.c"
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
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 999, i32* %.omp.ub, align 4, !tbaa !2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.iv2), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %tmp1) ]
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %4, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc14, %entry
  %5 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end16

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !2
  %9 = bitcast i32* %.omp.iv2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #2
  %10 = bitcast i32* %.omp.lb3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  store i32 0, i32* %.omp.lb3, align 4, !tbaa !2
  %11 = bitcast i32* %.omp.ub4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #2
  store i32 99, i32* %.omp.ub4, align 4, !tbaa !2

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.SIMD
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
; CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"
; CHECK-NOT: "QUAL.OMP.SHARED"
; CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}), "QUAL.OMP.PRIVATE"({{.*}})

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb3), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv2), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  %13 = load i32, i32* %.omp.lb3, align 4, !tbaa !2
  store i32 %13, i32* %.omp.iv2, align 4, !tbaa !2
  br label %omp.inner.for.cond5

omp.inner.for.cond5:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %14 = load i32, i32* %.omp.iv2, align 4, !tbaa !2
  %15 = load i32, i32* %.omp.ub4, align 4, !tbaa !2
  %cmp6 = icmp sle i32 %14, %15
  br i1 %cmp6, label %omp.inner.for.body7, label %omp.inner.for.end

omp.inner.for.body7:                              ; preds = %omp.inner.for.cond5
  %16 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #2
  %17 = load i32, i32* %.omp.iv2, align 4, !tbaa !2
  %mul8 = mul nsw i32 %17, 1
  %add9 = add nsw i32 0, %mul8
  store i32 %add9, i32* %j, align 4, !tbaa !2
  %18 = load i32, i32* %i, align 4, !tbaa !2
  %19 = load i32, i32* %j, align 4, !tbaa !2
  %add10 = add nsw i32 %18, %19
  %20 = load i32, i32* %i, align 4, !tbaa !2
  %idxprom = sext i32 %20 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !6
  %21 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add11 = add nsw i32 %21, %add10
  store i32 %add11, i32* %arrayidx, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body7
  %22 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, i32* %.omp.iv2, align 4, !tbaa !2
  %add12 = add nsw i32 %23, 1
  store i32 %add12, i32* %.omp.iv2, align 4, !tbaa !2
  br label %omp.inner.for.cond5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond5
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.SIMD"() {{.*}}

  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.GENERICLOOP"() ]
  %24 = bitcast i32* %.omp.ub4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.lb3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.iv2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #2
  br label %omp.body.continue13

omp.body.continue13:                              ; preds = %omp.loop.exit
  %27 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #2
  br label %omp.inner.for.inc14

omp.inner.for.inc14:                              ; preds = %omp.body.continue13
  %28 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add15 = add nsw i32 %28, 1
  store i32 %add15, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end16:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit17

omp.loop.exit17:                                  ; preds = %omp.inner.for.end16
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  %29 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #2
  %30 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #2
  %31 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %31) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_i", !3, i64 0}

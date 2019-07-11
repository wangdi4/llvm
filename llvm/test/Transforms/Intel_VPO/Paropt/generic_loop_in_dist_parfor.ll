; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -S | FileCheck %s

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
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.iv1 = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.ub3 = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.ub3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp2) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.ub3), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.PRIVATE"(i32* %tmp2) ]
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !3
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #1
  store i32 999, i32* %.omp.ub, align 4, !tbaa !3
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.ub3), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp2) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !3
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc13, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !3
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end15

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #1
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !3
  %11 = bitcast i32* %.omp.iv1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #1
  %12 = bitcast i32* %.omp.ub3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #1
  store i32 99, i32* %.omp.ub3, align 4, !tbaa !3

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.SIMD
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}) ]

  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv1), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub3) ]
  store i32 0, i32* %.omp.iv1, align 4, !tbaa !3
  br label %omp.inner.for.cond4

omp.inner.for.cond4:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %14 = load i32, i32* %.omp.iv1, align 4, !tbaa !3
  %15 = load i32, i32* %.omp.ub3, align 4, !tbaa !3
  %cmp5 = icmp sle i32 %14, %15
  br i1 %cmp5, label %omp.inner.for.body6, label %omp.inner.for.end

omp.inner.for.body6:                              ; preds = %omp.inner.for.cond4
  %16 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #1
  %17 = load i32, i32* %.omp.iv1, align 4, !tbaa !3
  %mul7 = mul nsw i32 %17, 1
  %add8 = add nsw i32 0, %mul7
  store i32 %add8, i32* %j, align 4, !tbaa !3
  %18 = load i32, i32* %i, align 4, !tbaa !3
  %19 = load i32, i32* %j, align 4, !tbaa !3
  %add9 = add nsw i32 %18, %19
  %20 = load i32, i32* %i, align 4, !tbaa !3
  %idxprom = sext i32 %20 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !7
  %21 = load i32, i32* %arrayidx, align 4, !tbaa !7
  %add10 = add nsw i32 %21, %add9
  store i32 %add10, i32* %arrayidx, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body6
  %22 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, i32* %.omp.iv1, align 4, !tbaa !3
  %add11 = add nsw i32 %23, 1
  store i32 %add11, i32* %.omp.iv1, align 4, !tbaa !3
  br label %omp.inner.for.cond4

omp.inner.for.end:                                ; preds = %omp.inner.for.cond4
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.SIMD"() {{.*}}

  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.GENERICLOOP"() ]
  %24 = bitcast i32* %.omp.ub3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #1
  %25 = bitcast i32* %.omp.iv1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #1
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.loop.exit
  %26 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #1
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %27 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %add14 = add nsw i32 %27, 1
  store i32 %add14, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  %28 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #1
  %29 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #1
  %30 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2051, i32 4471420, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"icx (ICX) dev.8.x.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_i", !4, i64 0}

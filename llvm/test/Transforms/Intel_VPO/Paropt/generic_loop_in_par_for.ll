; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -S | FileCheck %s

; This test checks that the "loop" construct is mapped to "simd"
; after prepare pass.

; int aaa[1000];
; void foo() {
; #pragma omp parallel for
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }


; ModuleID = 'generic_loop_in_par_for.c'
source_filename = "generic_loop_in_par_for.c"
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
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 999, i32* %.omp.ub, align 4, !tbaa !2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.ub3), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp2) ]
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %4, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc13, %entry
  %5 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end15

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !2
  %9 = bitcast i32* %.omp.iv1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #2
  %10 = bitcast i32* %.omp.ub3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  store i32 99, i32* %.omp.ub3, align 4, !tbaa !2

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.SIMD
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}) ]

  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv1), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub3) ]
  store i32 0, i32* %.omp.iv1, align 4, !tbaa !2
  br label %omp.inner.for.cond4

omp.inner.for.cond4:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %12 = load i32, i32* %.omp.iv1, align 4, !tbaa !2
  %13 = load i32, i32* %.omp.ub3, align 4, !tbaa !2
  %cmp5 = icmp sle i32 %12, %13
  br i1 %cmp5, label %omp.inner.for.body6, label %omp.inner.for.end

omp.inner.for.body6:                              ; preds = %omp.inner.for.cond4
  %14 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %14) #2
  %15 = load i32, i32* %.omp.iv1, align 4, !tbaa !2
  %mul7 = mul nsw i32 %15, 1
  %add8 = add nsw i32 0, %mul7
  store i32 %add8, i32* %j, align 4, !tbaa !2
  %16 = load i32, i32* %i, align 4, !tbaa !2
  %17 = load i32, i32* %j, align 4, !tbaa !2
  %add9 = add nsw i32 %16, %17
  %18 = load i32, i32* %i, align 4, !tbaa !2
  %idxprom = sext i32 %18 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !6
  %19 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add10 = add nsw i32 %19, %add9
  store i32 %add10, i32* %arrayidx, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body6
  %20 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, i32* %.omp.iv1, align 4, !tbaa !2
  %add11 = add nsw i32 %21, 1
  store i32 %add11, i32* %.omp.iv1, align 4, !tbaa !2
  br label %omp.inner.for.cond4

omp.inner.for.end:                                ; preds = %omp.inner.for.cond4
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.SIMD"() {{.*}}

  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.GENERICLOOP"() ]
  %22 = bitcast i32* %.omp.ub3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  %23 = bitcast i32* %.omp.iv1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23) #2
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.loop.exit
  %24 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %25 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add14 = add nsw i32 %25, 1
  store i32 %add14, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %26 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #2
  %27 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #2
  %28 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #2
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_i", !3, i64 0}

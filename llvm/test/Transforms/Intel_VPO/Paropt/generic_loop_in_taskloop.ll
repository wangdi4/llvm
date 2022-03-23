; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "loop" construct is mapped to "simd"
; after prepare pass.

; int aaa[1000];
; void foo() {
; #pragma omp taskloop
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }



; ModuleID = 'generic_loop_in_taskloop.c'
source_filename = "generic_loop_in_taskloop.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.iv4 = alloca i32, align 4
  %.omp.lb5 = alloca i32, align 4
  %.omp.ub6 = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !2
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #2
  store i64 999, i64* %.omp.ub, align 8, !tbaa !2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.iv4), "QUAL.OMP.PRIVATE"(i32* %.omp.lb5), "QUAL.OMP.PRIVATE"(i32* %.omp.ub6), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp3) ]
  %4 = load i64, i64* %.omp.lb, align 8, !tbaa !2
  %conv = trunc i64 %4 to i32
  store i32 %conv, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc17, %entry
  %5 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %conv1 = sext i32 %5 to i64
  %6 = load i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp = icmp ule i64 %conv1, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end19

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !6
  %9 = bitcast i32* %.omp.iv4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #2
  %10 = bitcast i32* %.omp.lb5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  store i32 0, i32* %.omp.lb5, align 4, !tbaa !6
  %11 = bitcast i32* %.omp.ub6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #2
  store i32 99, i32* %.omp.ub6, align 4, !tbaa !6

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.SIMD
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
; CHECK-NOT: "QUAL.OMP.FIRSTPRIVATE"
; CHECK-NOT: "QUAL.OMP.SHARED"
; CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}), "QUAL.OMP.PRIVATE"({{.*}})

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb5), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv4), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub6), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  %13 = load i32, i32* %.omp.lb5, align 4, !tbaa !6
  store i32 %13, i32* %.omp.iv4, align 4, !tbaa !6
  br label %omp.inner.for.cond7

omp.inner.for.cond7:                              ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %14 = load i32, i32* %.omp.iv4, align 4, !tbaa !6
  %15 = load i32, i32* %.omp.ub6, align 4, !tbaa !6
  %cmp8 = icmp sle i32 %14, %15
  br i1 %cmp8, label %omp.inner.for.body10, label %omp.inner.for.end

omp.inner.for.body10:                             ; preds = %omp.inner.for.cond7
  %16 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #2
  %17 = load i32, i32* %.omp.iv4, align 4, !tbaa !6
  %mul11 = mul nsw i32 %17, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, i32* %j, align 4, !tbaa !6
  %18 = load i32, i32* %i, align 4, !tbaa !6
  %19 = load i32, i32* %j, align 4, !tbaa !6
  %add13 = add nsw i32 %18, %19
  %20 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom = sext i32 %20 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !8
  %21 = load i32, i32* %arrayidx, align 4, !tbaa !8
  %add14 = add nsw i32 %21, %add13
  store i32 %add14, i32* %arrayidx, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body10
  %22 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, i32* %.omp.iv4, align 4, !tbaa !6
  %add15 = add nsw i32 %23, 1
  store i32 %add15, i32* %.omp.iv4, align 4, !tbaa !6
  br label %omp.inner.for.cond7

omp.inner.for.end:                                ; preds = %omp.inner.for.cond7
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.SIMD"() {{.*}}

  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.GENERICLOOP"() ]
  %24 = bitcast i32* %.omp.ub6 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  %25 = bitcast i32* %.omp.lb5 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #2
  %26 = bitcast i32* %.omp.iv4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #2
  br label %omp.body.continue16

omp.body.continue16:                              ; preds = %omp.loop.exit
  %27 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %27) #2
  br label %omp.inner.for.inc17

omp.inner.for.inc17:                              ; preds = %omp.body.continue16
  %28 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add18 = add nsw i32 %28, 1
  store i32 %add18, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end19:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit20

omp.loop.exit20:                                  ; preds = %omp.inner.for.end19
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKLOOP"() ]
  %29 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %29) #2
  %30 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %30) #2
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
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !7, i64 0}
!9 = !{!"array@_ZTSA1000_i", !7, i64 0}

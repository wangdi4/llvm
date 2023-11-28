; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int i = 0, j = 0, y = 0;
;
; void foo() {
; #pragma omp parallel for schedule(dynamic) reduction(+ : y)
;   for (j = 0; j < 10; j++) {
; #pragma omp cancellation point for
;     printf("j = %d\n", j);
; #pragma omp cancel for
;     y++;
;   }
;   printf("y = %d\n", y);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = dso_local global i32 0, align 4
@j = dso_local global i32 0, align 4
@y = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  store i32 9, ptr %.omp.ub, align 4, !tbaa !4

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
  %1 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %3 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr @j, align 4, !tbaa !4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(),
    "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; CHECK: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; CHECK: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; CHECK: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+split]], label %{{[a-zA-Z._0-9]+}}

  %6 = load i32, ptr @j, align 4, !tbaa !4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %6) #2
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; CHECK: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; CHECK: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; CHECK: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %8 = load i32, ptr @y, align 4, !tbaa !4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr @y, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  %10 = load i32, ptr @y, align 4, !tbaa !4
  %call2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %10)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(ptr noundef, ...) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

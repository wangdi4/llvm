; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL

; Test src:
;
; #include <stdio.h>
;
; int i = 0, j = 0, y = 0;
;
; void foo() {
;   #pragma omp parallel
;   {
;     printf("i = %d\n", i++);
;
;     #pragma omp barrier
;
;     #pragma omp for schedule(static) reduction(+ : y)
;     for (j = 0; j < 10; j++) {
;       #pragma omp cancellation point for
;       printf("j = %d\n", j);
;       #pragma omp cancel for if (0)
;       y++;
;     }
;   }
;   printf("y = %d\n", y);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = dso_local global i32 0, align 4
@j = dso_local global i32 0, align 4
@y = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
; TFORM-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; TFORM-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
; #pragma omp parallel
; TFORM: call void {{.+}} @__kmpc_fork_call

  %1 = load i32, ptr @i, align 4, !tbaa !4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr @i, align 4, !tbaa !4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %1) #1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier (should still be kmpc_barrier, not kmpc_cancel_barrier)
; ALL: call void @__kmpc_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})

  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 9, ptr %.omp.ub, align 4, !tbaa !4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 0),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @y, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR: %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; PREPR-SAME: "QUAL.OMP.CANCELLATION.POINTS"(ptr [[CP1ALLOCA:%[a-zA-Z._0-9]+]], ptr [[CP3ALLOCA:%[a-zA-Z._0-9]+]], ptr [[CP2ALLOCA:%[a-zA-Z._0-9]+]])

  %4 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %4, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %6 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr @j, align 4, !tbaa !4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(),
    "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; ALL: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CANCEL1]], ptr [[CP1ALLOCA]]
; TFORM: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+_crit_edge]], label %{{[a-zA-Z._0-9]+}}

  %9 = load i32, ptr @j, align 4, !tbaa !4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %9) #1
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.LOOP"(),
    "QUAL.OMP.IF"(i1 false) ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; ALL: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CANCEL2]], ptr [[CP2ALLOCA]]
; TFORM: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

; Cancellation point for if(cancel_expr) {kmpc_cancel()}; else {kmpc_cancellationpoint()}
; ALL: [[CP2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CP2]], ptr [[CP3ALLOCA]]
; TFORM: [[CHECK3:%cancel.check[0-9]*]] = icmp ne i32 [[CP2]], 0
; TFORM: br i1 [[CHECK3]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %11 = load i32, ptr @y, align 4, !tbaa !4
  %inc2 = add nsw i32 %11, 1
  store i32 %inc2, ptr @y, align 4, !tbaa !4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add3 = add nsw i32 %12, 1
  store i32 %add3, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %13 = load i32, ptr @y, align 4, !tbaa !4
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %13)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nocallback nofree nosync nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

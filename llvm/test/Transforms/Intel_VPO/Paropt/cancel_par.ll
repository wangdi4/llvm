; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=CRITICAL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=FASTRED
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL

; Test src:
;
; #include <stdio.h>
;
; int i = 0, j = 0, x = 0;
;
; void foo() {
;   #pragma omp parallel reduction(+ : x)
;   {
;     #pragma omp single
;     printf("i = %d\n", i++);
;     #pragma omp cancel parallel
;     #pragma omp barrier
;
;     #pragma omp for
;     for (j = 0; j < 10; j++) {
;       printf("j = %d\n", j);
;     }
;     #pragma omp cancellation point parallel
;     x++;
;   }
;   printf("x = %d\n", x);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = dso_local global i32 0, align 4
@j = dso_local global i32 0, align 4
@x = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

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
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @x, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
; #pragma omp parallel
; Updated region exit intrinsic after vpo-paropt-prepare
; PREPR: %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.CANCELLATION.POINTS"(ptr [[CP1ALLOCA:%[a-zA-Z._0-9]+]], ptr [[CP2ALLOCA:%[a-zA-Z._0-9]+]], ptr [[CP3ALLOCA:%[a-zA-Z._0-9]+]], ptr [[CP4ALLOCA:%[a-zA-Z._0-9]+]])
; TFORM: call void {{.+}} @__kmpc_fork_call

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  fence acquire
  %2 = load i32, ptr @i, align 4, !tbaa !4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr @i, align 4, !tbaa !4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %2) #1
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
; #pragama omp single
; ALL: %{{[a-zA-Z._0-9]+}} = call i32 @__kmpc_single({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; ALL  call void @__kmpc_end_single({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; Implicit barrier for single
; ALL: [[CBARRIER1:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; PREPR-NEXT: store i32 [[CBARRIER1]], ptr [[CP1ALLOCA]]
; TFORM-NOT: store i32 [[CBARRIER1]], ptr {{%[0-9]+}}
; TFORM: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER1]], 0
; TFORM: br i1 [[CHECK1]], label %[[PAREXITLABEL:[a-zA-Z._0-9]+]], label %{{[a-zA-Z._0-9]+}}

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; ALL: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 1)
; PREPR-NEXT: store i32 [[CANCEL1]], ptr [[CP2ALLOCA]]
; TFORM-NOT: store i32 [[CANCEL1]], ptr {{%[0-9]+}}
; TFORM: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM: br i1 [[CHECK2]], label %[[PAREXITLABEL1:[a-zA-Z._0-9]+\.split_crit_edge]], label %{{[a-zA-Z._0-9]+}}

; Exit label for cancel/cancellationpoint with cancel_barrier
; TFORM: [[PAREXITLABEL1]]:{{.*}}
; TFORM: [[CBARRIER4:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; TFORM-NEXT: br label %[[PAREXITLABEL]]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier
; ALL: [[CBARRIER2:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; PREPR-NEXT: store i32 [[CBARRIER2]], ptr [[CP3ALLOCA]]
; TFORM-NOT: store i32 [[CBARRIER2]], ptr {{%[0-9]+}}
; TFORM-NEXT: [[CHECK3:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER2]], 0
; TFORM-NEXT: br i1 [[CHECK3]], label %[[PAREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #1
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #1
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #1
  store i32 9, ptr %.omp.ub, align 4, !tbaa !4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]
; #pragma omp for
; TFORM:  call void @__kmpc_for_static_init_4({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i32 34, ptr %is.last, ptr %lower.bnd, ptr %upper.bnd, ptr %stride, i32 1, i32 1)

  %6 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %6, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %8 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr @j, align 4, !tbaa !4
  %10 = load i32, ptr @j, align 4, !tbaa !4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %10) #1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add2 = add nsw i32 %11, 1
  store i32 %add2, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.LOOP"() ]
; TFORM: call void @__kmpc_for_static_fini({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}})
; Implicit barrier for omp for
; TFORM: [[CBARRIER3:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; TFORM-NEXT: [[CHECK4:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER3]], 0
; TFORM-NEXT: br i1 [[CHECK4]], label %[[PAREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #1
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(),
    "QUAL.OMP.CANCEL.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellation point
; ALL: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 1)
; PREPR-NEXT: store i32 [[CANCEL2]], ptr [[CP4ALLOCA]]
; TFORM-NOT: store i32 [[CANCEL2]], ptr {{%[0-9]+}}
; TFORM-NEXT: [[CHECK5:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM-NEXT: br i1 [[CHECK5]], label %[[PAREXITLABEL1]], label %{{[a-zA-Z._0-9]+}}

  %13 = load i32, ptr @x, align 4, !tbaa !4
  %inc3 = add nsw i32 %13, 1
  store i32 %inc3, ptr @x, align 4, !tbaa !4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
; Reduction Handling
; CRITICAL: call void @__kmpc_critical({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, ptr  @{{[a-zA-Z._0-9]*}})
; CRITICAL: call void @__kmpc_end_critical({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, ptr  @{{[a-zA-Z._0-9]*}})

; FASTRED: call i32 @__kmpc_reduce({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 {{[0-9]*}}, i32 {{[0-9]*}}, ptr %{{[a-zA-Z._0-9]*}}, ptr @{{[a-zA-Z._0-9]*}}, ptr @{{[a-zA-Z._0-9]*}})
; FASTRED: call void @__kmpc_end_reduce({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, ptr  @{{[a-zA-Z._0-9]*}})

  %14 = load i32, ptr @x, align 4, !tbaa !4
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %14)
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

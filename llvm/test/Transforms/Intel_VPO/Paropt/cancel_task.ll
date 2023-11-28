; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL

; Test src:
;
; #include <stdio.h>
;
; int i = 0, j = 0, x = 0;
;
; void foo() {
;   #pragma omp parallel
;   {
;     #pragma omp taskgroup
;     {
;       #pragma omp task
;       printf("i = %d\n", i++);
;
;       #pragma omp barrier
;
;       #pragma omp task
;       {
;         printf("j = %d\n", j++);
;         #pragma omp cancel taskgroup
;       }
;
;       #pragma omp task
;       {
;         #pragma omp cancellation point taskgroup
;         x++;
;       }
;     }
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @j, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @x, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @i, i32 0, i32 1) ]
  %3 = load i32, ptr @i, align 4, !tbaa !4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr @i, align 4, !tbaa !4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %3) #1
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier (should still be kmpc_barrier, not kmpc_cancel_barrier)
; ALL-DAG: call void @__kmpc_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @j, i32 0, i32 1) ]
  %6 = load i32, ptr @j, align 4, !tbaa !4
  %inc1 = add nsw i32 %6, 1
  store i32 %inc1, ptr @j, align 4, !tbaa !4
  %call2 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %6) #1
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancell
; ALL-DAG: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 4)
; PREPR-DAG: store i32 [[CANCEL1]], ptr [[CP1ALLOCA:%[a-zA-Z._0-9]+]]
; TFORM-DAG: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM-DAG: br i1 [[CHECK1]], label %{{[a-zA-Z._0-9]+}}, label %{{[a-zA-Z._0-9]+}}

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASK"() ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR-DAG:  %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; PREPR-DAG-SAME: "QUAL.OMP.CANCELLATION.POINTS"(ptr [[CP1ALLOCA]]) ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @x, i32 0, i32 1) ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(),
    "QUAL.OMP.CANCEL.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellation point
; ALL-DAG: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 4)
; PREPR-DAG: store i32 [[CANCEL2]], ptr [[CP2ALLOCA:%[a-zA-Z._0-9]+]]
; TFORM-DAG: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM-DAG: br i1 [[CHECK2]], label %{{[a-zA-Z._0-9]+}}, label %{{[a-zA-Z._0-9]+}}

  %10 = load i32, ptr @x, align 4, !tbaa !4
  %inc3 = add nsw i32 %10, 1
  store i32 %inc3, ptr @x, align 4, !tbaa !4
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASK"() ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR-DAG:  %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; PREPR-DAG-SAME: "QUAL.OMP.CANCELLATION.POINTS"(ptr [[CP2ALLOCA]]) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %11 = load i32, ptr @x, align 4, !tbaa !4
  %call4 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %11)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(ptr noundef, ...) #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

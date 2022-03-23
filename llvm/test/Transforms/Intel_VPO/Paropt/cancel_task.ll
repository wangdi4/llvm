; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL

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

; ModuleID = 'cancel_task.c'
source_filename = "cancel_task.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = global i32 0, align 4
@j = global i32 0, align 4
@x = global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* @i), "QUAL.OMP.SHARED"(i32* @j), "QUAL.OMP.SHARED"(i32* @x) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  %3 = load i32, i32* @i, align 4, !tbaa !2
  %inc = add nsw i32 %3, 1
  store i32 %inc, i32* @i, align 4, !tbaa !2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %3)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier (should still be kmpc_barrier, not kmpc_cancel_barrier)
; ALL-DAG: call void @__kmpc_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})


  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]

  %6 = load i32, i32* @j, align 4, !tbaa !2
  %inc1 = add nsw i32 %6, 1
  store i32 %inc1, i32* @j, align 4, !tbaa !2
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %6)
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancell
; ALL-DAG: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 4)
; PREPR-DAG: store i32 [[CANCEL1]], i32* [[CP1ALLOCA:%[a-zA-Z._0-9]+]]
; TFORM-DAG: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM-DAG: br i1 [[CHECK1]], label %{{[a-zA-Z._0-9]+}}, label %{{[a-zA-Z._0-9]+}}

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TASK"() ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR-DAG:  %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; PREPR-DAG-SAME: "QUAL.OMP.CANCELLATION.POINTS"(i32* [[CP1ALLOCA]]) ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(), "QUAL.OMP.CANCEL.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellation point
; ALL-DAG: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 4)
; PREPR-DAG: store i32 [[CANCEL2]], i32* [[CP2ALLOCA:%[a-zA-Z._0-9]+]]
; TFORM-DAG: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM-DAG: br i1 [[CHECK2]], label %{{[a-zA-Z._0-9]+}}, label %{{[a-zA-Z._0-9]+}}

  %10 = load i32, i32* @x, align 4, !tbaa !2
  %inc3 = add nsw i32 %10, 1
  store i32 %inc3, i32* @x, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASK"() ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR-DAG:  %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"()
; PREPR-DAG-SAME: "QUAL.OMP.CANCELLATION.POINTS"(i32* [[CP2ALLOCA]]) ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %11 = load i32, i32* @x, align 4, !tbaa !2
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i32 0, i32 0), i32 %11)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @printf(i8*, ...) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

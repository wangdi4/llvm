; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int i = 0, j = 0, x = 0;
;
; void foo() {
;   printf("enter\n");
;   #pragma omp sections reduction(+:x)
;   {
;     printf("x = %d\n", i++);
;
;     #pragma omp section
;       printf("i = %d\n", i++);
;
;     #pragma omp section
;     {
;       printf("j = %d\n", j++);
;       x++;
;       #pragma omp cancel sections if (0)
;     }
;
;     #pragma omp section
;     {
;       #pragma omp cancellation point sections
;       x++;
;     }
;   }
;     printf("x = %d\n", x);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = dso_local global i32 0, align 4
@j = dso_local global i32 0, align 4
@x = dso_local global i32 0, align 4
@.str = private unnamed_addr constant [7 x i8] c"enter\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.3 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr @x, i32 0, i32 1) ]
  %1 = load i32, ptr @i, align 4, !tbaa !4
  %inc = add nsw i32 %1, 1
  store i32 %inc, ptr @i, align 4, !tbaa !4
  %call1 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %1) #2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %3 = load i32, ptr @i, align 4, !tbaa !4
  %inc2 = add nsw i32 %3, 1
  store i32 %inc2, ptr @i, align 4, !tbaa !4
  %call3 = call i32 (ptr, ...) @printf(ptr noundef @.str.2, i32 noundef %3) #2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %5 = load i32, ptr @j, align 4, !tbaa !4
  %inc4 = add nsw i32 %5, 1
  store i32 %inc4, ptr @j, align 4, !tbaa !4
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str.3, i32 noundef %5) #2
  %6 = load i32, ptr @x, align 4, !tbaa !4
  %inc6 = add nsw i32 %6, 1
  store i32 %inc6, ptr @x, align 4, !tbaa !4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(),
    "QUAL.OMP.CANCEL.SECTIONS"(),
    "QUAL.OMP.IF"(i1 false) ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; CHECK: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 3)
; CHECK: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; CHECK: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+_crit_edge]], label %{{[a-zA-Z._0-9]+}}

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SECTION"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(),
    "QUAL.OMP.CANCEL.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; CHECK: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 3)
; CHECK: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; CHECK: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %10 = load i32, ptr @x, align 4, !tbaa !4
  %inc7 = add nsw i32 %10, 1
  store i32 %inc7, ptr @x, align 4, !tbaa !4
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SECTION"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  %11 = load i32, ptr @x, align 4, !tbaa !4
  %call8 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i32 noundef %11)
  ret void
}

declare dso_local i32 @printf(ptr noundef, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

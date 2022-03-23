; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

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

; ModuleID = 'cancel_section.c'
source_filename = "cancel_section.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = global i32 0, align 4
@j = global i32 0, align 4
@x = global i32 0, align 4
@.str = private unnamed_addr constant [7 x i8] c"enter\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.3 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0))
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTIONS"(), "QUAL.OMP.REDUCTION.ADD"(i32* @x) ]
  %1 = load i32, i32* @i, align 4, !tbaa !2
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* @i, align 4, !tbaa !2
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %1)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %3 = load i32, i32* @i, align 4, !tbaa !2
  %inc2 = add nsw i32 %3, 1
  store i32 %inc2, i32* @i, align 4, !tbaa !2
  %call3 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i32 0, i32 0), i32 %3)
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SECTION"() ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %5 = load i32, i32* @j, align 4, !tbaa !2
  %inc4 = add nsw i32 %5, 1
  store i32 %inc4, i32* @j, align 4, !tbaa !2
  %call5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.3, i32 0, i32 0), i32 %5)
  %6 = load i32, i32* @x, align 4, !tbaa !2
  %inc6 = add nsw i32 %6, 1
  store i32 %inc6, i32* @x, align 4, !tbaa !2
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.SECTIONS"(), "QUAL.OMP.IF"(i32 0) ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; CHECK: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 3)
; CHECK: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; CHECK: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+_crit_edge]], label %{{[a-zA-Z._0-9]+}}

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.SECTION"() ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.SECTION"() ]
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(), "QUAL.OMP.CANCEL.SECTIONS"() ]
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; CHECK: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 3)
; CHECK: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; CHECK: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %10 = load i32, i32* @x, align 4, !tbaa !2
  %inc7 = add nsw i32 %10, 1
  store i32 %inc7, i32* @x, align 4, !tbaa !2
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.SECTION"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SECTIONS"() ]
  %11 = load i32, i32* @x, align 4, !tbaa !2
  %call8 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %11)
  ret void
}

declare i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

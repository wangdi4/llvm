; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
; #include <omp.h>
;
; void foo() {
; #pragma omp parallel num_threads(12)
;   {
;
;     int i = omp_get_thread_num();
;     printf("%d: before\n", i);
;
; #pragma omp cancel parallel if (i == 9)
;
;     printf("%d: after\n", i);
;   }
; }
;
; ModuleID = 'cancel_par_if.c'
source_filename = "cancel_par_if.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [12 x i8] c"%d: before\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"%d: after\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 12), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %call = call i32 @omp_get_thread_num()
  store i32 %call, i32* %i, align 4
  %1 = load i32, i32* %i, align 4
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i64 0, i64 0), i32 %1)
  %2 = load i32, i32* %i, align 4
  %cmp = icmp eq i32 %2, 9
  %conv = zext i1 %cmp to i32
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.PARALLEL"(), "QUAL.OMP.IF"(i32 %conv) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.CANCEL"() ]

  ; CHECK: %[[CANCEL:[^ ]+]] = call i32 @__kmpc_cancel(%struct.ident_t* @{{[^ ]+}}, i32 %{{[^ ]+}}, i32 1)
  ; CHECK: %[[CHECK1:[^ ]+]] = icmp ne i32 %[[CANCEL]], 0
  ; CHECK: br i1 %[[CHECK1]], label %[[CANCELLED:[^ ]+]], label %{{[^ ]+}}

  ; CHECK: %[[CP:[^ ]+]] = call i32 @__kmpc_cancellationpoint(%struct.ident_t* @{{[^ ]+}}, i32 %{{[^ ]+}}, i32 1)
  ; CHECK: %[[CHECK2:[^ ]+]] = icmp ne i32 %[[CP]], 0
  ; CHECK: br i1 %[[CHECK2]], label %[[CANCELLED]], label %{{[^ ]+}}

  %4 = load i32, i32* %i, align 4
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), i32 %4)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @omp_get_thread_num() #2

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}

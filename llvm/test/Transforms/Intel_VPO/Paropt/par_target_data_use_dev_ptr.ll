; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Check that the local copy of %array_device created for the use_device_ptr
; clause on the inner target-data directive, is private to the outer parallel
; construct, i.e., it is not in the param list for the parallel construct.

; #include <stdio.h>
;
; int main() {
;   int a[10];
;   int *array_device = &a[0];
;   printf("%p\n", &array_device[0]);
; #pragma omp parallel num_threads(1)
; //#pragma omp target data map(tofrom: array_device[0:10])
;   {
; #pragma omp target data use_device_ptr(array_device)
;     {
; //#pragma omp target is_device_ptr(array_device)
;       {
;         printf("%p\n", &array_device[0]);
;       } // end target
;     } // end target data
; //  printf("%p\n", &array_device[0]);
;   } // end target data
; }

; Check that only %array_device is passed into the outlined function for DIR.OMP.PARALLEL
; CHECK:  call void {{.+}} @__kmpc_fork_call(ptr {{.+}}, i32 1, ptr @main.DIR.OMP.PARALLEL{{.+}}, ptr %array_device)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %array_device = alloca ptr, align 8
  %arrayidx = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 0
  store ptr %arrayidx, ptr %array_device, align 8
  %0 = load ptr, ptr %array_device, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %0, i64 0
  %call = call i32 (ptr, ...) @printf(ptr @.str, ptr %arrayidx1)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 1),
    "QUAL.OMP.SHARED"(ptr %array_device) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %array_device) ]

  %3 = load ptr, ptr %array_device, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %3, i64 0
  %call3 = call i32 (ptr, ...) @printf(ptr @.str, ptr %arrayidx2) #2

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  ret i32 0
}

declare dso_local i32 @printf(ptr, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

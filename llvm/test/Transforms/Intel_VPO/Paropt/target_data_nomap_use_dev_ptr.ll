; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
;
; int main() {
;   int a[10];
;   int *array_device = &a[0];
;   printf("%p\n", array_device);
;   {
; #pragma omp target data use_device_ptr(array_device)
;     {
;       printf("%p\n", array_device); // Should print "(nil)"
;     }
;   }
; }

; Check that map size of 0 is used for the map created for use_device_ptr.
; CHECK: @.offload_sizes = private unnamed_addr constant [1 x i64] zeroinitializer
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 64]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@.str = private unnamed_addr constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %array_device = alloca i32*, align 8
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %a, i64 0, i64 0
  store i32* %arrayidx, i32** %array_device, align 8
  %0 = load i32*, i32** %array_device, align 8
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %0)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(i32** %array_device) ]

  %2 = load i32*, i32** %array_device, align 8
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32* %2) #2

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret i32 0
}

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}

; RUN: opt -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s | FileCheck %s

; Test for TARGET VARIANT DISPATCH construct with the return value of the function used
; outside the region, which should cause the emission of a PHI after transformation of
; the variant construct.

; C Source:
; #include <stdio.h>
; int __attribute__((nothrow)) foo_gpu(void* interop) { return 456; }
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int __attribute__((nothrow)) foo(void) { return 123; }
;
; int main() {
;   #pragma omp target variant dispatch
;      printf("%d\n", foo());
; }

; Check for is_device_available call
; CHECK: %[[DEFAULT_DEVICE:[^ ]+]] = call i32 @omp_get_default_device()
; CHECK: %[[DEFAULT_DEVICE_CAST:[^ ]+]] = zext i32 %[[DEFAULT_DEVICE]] to i64
; CHECK: %[[IS_AVAILABLE:[^ ]+]] = call i32 @__tgt_is_device_available(i64 %[[DEFAULT_DEVICE_CAST]], i8* inttoptr (i64 15 to i8*))
; CHECK: %[[DISPATCH:[^ ]+]] = icmp ne i32 %[[IS_AVAILABLE]], 0

; CHECK: br i1 %[[DISPATCH]], label %[[VARIANT:[^ ,]+]], label %[[BASE:[^ ,]+]]

; Check that the variant function wrapper takes an i32*, which contains the value of
; the return value of foo_gpu.
; CHECK-DAG:[[VARIANT]]:
; CHECK: call void @{{[^ ]*}}foo_gpu.wrapper{{[^ ]*}}(i64 %[[DEFAULT_DEVICE_CAST]], i32* %[[FOO_GPU_RET_PTR:[^ ,)]+]])
; CHECK: %[[FOO_GPU_RET:[^ ]+]] = load i32, i32* %[[FOO_GPU_RET_PTR]], align 4

; CHECK-DAG:[[BASE]]:
; CHECK: %[[FOO_RET:[^ ]+]] = call i32 @foo()
; CHECK: br label %[[BASE_SUCC:[^ ]+]]

; Check that the value used in the printf is a PHI which takes the value of FOO_RET or FOO_GPU_RET
; CHECK-DAG: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %[[VAL_IN_PRINTF:[^ ,)]+]])
; CHECK-DAG: %[[VAL_IN_PRINTF]] = phi i32 [ %[[FOO_GPU_RET]], %[[VARIANT]] ], [ %[[FOO_RET]], %[[BASE_SUCC]] ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %call = call i32 @foo() #3
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %call) #3
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable
declare i32 @foo_gpu(i8*) #1

; Function Attrs: noinline nounwind optnone uwtable
declare i32 @foo() #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

declare dso_local i32 @printf(i8*, ...) #4

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}

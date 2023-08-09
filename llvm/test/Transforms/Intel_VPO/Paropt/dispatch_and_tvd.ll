; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; Test src:

; #include <stdio.h>
; #include <omp.h>
;
; void foo_gpu(double *x, void* interop_obj) {
;   printf("enter variant function\n");
; }
;
; #pragma omp declare variant (foo_gpu) match(construct={dispatch}, device={arch(gen)}) append_args(interop(targetsync)) adjust_args(need_device_ptr:x)
; #pragma omp declare variant (foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; void foo(double *x) {
;   printf("enter base function\n");
; }
;
; void bar(double *y) {
; #pragma omp target variant dispatch
;   foo(y);
;
; #pragma omp dispatch
;   foo(y);
; }

; ALL: call void @__tgt_target_data_begin_mapper({{.*}})
; ALL: call void @bar.foo_gpu.wrapper(ptr %y.addr.new, i64 %{{.*}})
; ALL: call void @__tgt_target_data_end_mapper({{.*}})

; OCG: %[[INTEROPOBJ:[^ ]+]] = call ptr @__tgt_create_interop_obj({{.*}})
; NCG: %[[INTEROPOBJ:[^ ]+]] = call ptr @__tgt_get_interop_obj({{.*}})
; ALL: call void @__tgt_target_data_begin_mapper({{.*}})
; ALL: call void @foo_gpu(ptr %y.val1.updated.val, ptr %[[INTEROPOBJ]])
; ALL: call void @__tgt_target_data_end_mapper({{.*}})

; ALL: define internal void @bar.foo_gpu.wrapper(ptr [[YADDR:%[^ ,]+]], i64 %{{.*}})
; ALL: [[YVAL:%[^ ]+]] = load ptr, ptr [[YADDR]], align 8
; ALL: [[INTEROPOBJ2:%[^ ]+]] = call ptr @__tgt_create_interop_obj({{.*}})
; ALL: call void @foo_gpu(ptr [[YVAL]], ptr [[INTEROPOBJ2]])


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
declare void @foo_gpu(ptr, ptr) #0

; Function Attrs: noinline nounwind optnone uwtable
declare void @foo(ptr) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(ptr %y) #2 {
entry:
  %y.addr = alloca ptr, align 8
  store ptr %y, ptr %y.addr, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %y.addr) ]

  %y.val = load ptr, ptr %y.addr, align 8
  call void @foo(ptr %y.val) #3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]

  %y.val1 = load ptr, ptr %y.addr, align 8
  call void @foo(ptr %y.val1) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen;;name:foo_gpu;construct:dispatch;arch:gen;need_device_ptr:T;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}

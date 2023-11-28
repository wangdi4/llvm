; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test src:

; void __attribute__((nothrow)) foo_gpu(void *ptr);
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; void __attribute__((nothrow)) foo(void *ptr);

; void bar() {
;   float *host_ptr;
; #pragma omp target variant dispatch use_device_ptr(host_ptr)
;   foo(host_ptr);
; }

; CHECK: call void @__tgt_target_data_begin({{.+}})
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*foo_gpu.wrapper[^ (]*]](ptr %host_ptr.new)
; CHECK: call void @__tgt_target_data_end({{.+}})

; CHECK: define internal void @[[VARIANT_WRAPPER]](ptr [[HOST_PTR:%[^, )]+]])
; CHECK: [[LOAD:%[^ ]+]] = load ptr, ptr [[HOST_PTR]]
; CHECK: call void @foo_gpu(ptr [[LOAD]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar() #0 {
entry:
  %host_ptr = alloca ptr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr %host_ptr) ]

  %1 = load ptr, ptr %host_ptr, align 8
  call void @foo(ptr noundef %1) #1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare dso_local void @foo(ptr noundef) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

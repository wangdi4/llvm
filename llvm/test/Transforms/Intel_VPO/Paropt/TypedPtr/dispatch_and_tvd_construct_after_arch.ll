; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S -debug-only=vpo-paropt-target -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S -debug-only=vpo-paropt-target -disable-output %s 2>&1 | FileCheck %s
;
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

; This test is a hand-modified version of dispatch_and_tvd.ll which tests that
; we can correctly parse the openmp-variant attribute string when the
; "construct" field is after the "arch" field for the first variant, and there
; is no "arch" field for the second.

; CHECK: getVariantInfo: Found variant function: foo_gpu and device bits 0x000F for construct 'target_variant_dispatch'
; CHECK: genTargetVariantDispatchCode: Found variant function name: foo_gpu
; CHECK: getVariantInfo: Found variant function: foo_gpu with no device arch specified for construct 'dispatch'
; CHECK: genDispatchCode: Found variant function name: foo_gpu ; need_device_ptr: T ; interop: targetsync

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
declare void @foo_gpu(double* %x, i8* %interop_obj) #0

; Function Attrs: noinline nounwind optnone uwtable
declare void @foo(double* %x) #2

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(double* %y) #3 {
entry:
  %y.addr = alloca double*, align 8
  store double* %y, double** %y.addr, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(double** %y.addr) ]
  %y.val = load double*, double** %y.addr, align 8
  call void @foo(double* %y.val) #4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]
  %y.val1 = load double*, double** %y.addr, align 8
  call void @foo(double* %y.val1) #4 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;arch:gen;construct:target_variant_dispatch;;name:foo_gpu;construct:dispatch;need_device_ptr:T;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}

; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S -debug-only=vpo-paropt-target -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S -debug-only=vpo-paropt-target -disable-output %s 2>&1 | FileCheck %s

; Test src:
;
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

@.str = private unnamed_addr constant [24 x i8] c"enter variant function\0A\00", align 1
@.str.1 = private unnamed_addr constant [21 x i8] c"enter base function\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo_gpu(ptr noundef %x, ptr noundef %interop_obj) #0 {
entry:
  %x.addr = alloca ptr, align 8
  %interop_obj.addr = alloca ptr, align 8
  store ptr %x, ptr %x.addr, align 8
  store ptr %interop_obj, ptr %interop_obj.addr, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  ret void
}

declare dso_local i32 @printf(ptr noundef, ...) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %x) #2 {
entry:
  %x.addr = alloca ptr, align 8
  store ptr %x, ptr %x.addr, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(ptr noundef %y) #3 {
entry:
  %y.addr = alloca ptr, align 8
  store ptr %y, ptr %y.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  %1 = load ptr, ptr %y.addr, align 8
  call void @foo(ptr noundef %1) #4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]
  %3 = load ptr, ptr %y.addr, align 8
  call void @foo(ptr noundef %3) #4 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISPATCH"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen;;name:foo_gpu;construct:dispatch;need_device_ptr:T;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s
;
; Test src:

; void use(int v);
;
; int foo_gpu();
;
; #pragma omp declare variant (foo_gpu) match(construct={dispatch}, device={arch(gen)})
; int foo();
;
; void bar(double *y) {
; #pragma omp dispatch
;   use(foo());
; }

; The IR is hand-modified from that emitted with the above test, to ensure:
; 1. DISPATCH.CALL bundle is on the call to foo.
; 2. There are two uses of the return value of foo, i.e. two calls to use
;    within the dispatch region.

; Make sure both uses of %call have been replaced by the PHI combining the
; variant/original return values.

; CHECK: [[IS_DEVICE_AVAILABLE:%[^ ]+]] = call i32 @__tgt_is_device_available({{.*}})
; CHECK: %available = icmp ne i32 [[IS_DEVICE_AVAILABLE]], 0
; CHECK: br i1 %available, label %if.then, label %if.else
; CHECK: if.then:
; CHECK:   %variant = call i32 (...) @foo_gpu()
; CHECK: if.else:
; CHECK:   %call = call i32 (...) @foo()

; CHECK: %callphi = phi i32 [ %variant, %if.then ], [ %call, %if.else ]
; CHECK: call void @use(i32 %callphi)
; CHECK: call void @use(i32 %callphi)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar(double* %y) #0 {
entry:
  %y.addr = alloca double*, align 8
  store double* %y, double** %y.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]
  %call = call i32 (...) @foo() #1 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @use(i32 %call) #1
  call void @use(i32 %call) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @use(i32) #2

declare dso_local i32 @foo(...) #3

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:foo_gpu;construct:dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}

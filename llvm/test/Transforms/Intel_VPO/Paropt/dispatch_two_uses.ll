; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s
;
; INTEL_CUSTOMIZATION
;  CMPLRLLVM-33526:
;  This test simulates the code that Fortran frontend generates in case of a function with
;  complex return type, which causes two uses of the return value within the region.
; end INTEL_CUSTOMIZATION

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
define dso_local void @bar(ptr %y) {
entry:
  %y.addr = alloca ptr, align 8
  store ptr %y, ptr %y.addr, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]
  %call = call i32 (...) @foo() [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @use(i32 %call)
  call void @use(i32 %call)
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @use(i32)
declare dso_local i32 @foo(...) #3

attributes #3 = { "openmp-variant"="name:foo_gpu;construct:dispatch;arch:gen" }

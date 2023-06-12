; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S -vpo-paropt-use-interop=false %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S -vpo-paropt-use-interop=false %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct without a DEVICE clause
; and the associated function returns an INT that is used downstream.
;
; This test is hand-modified based on target_variant_dispatch_intfunc.ll
; I manually added 2 empty BasicBlocks (EmptyBB1 and EmptyBB2) into the
; target variant dispatch region to ensure that the codegen logic handles
; them properly instead of aborting.

; C source:
;
; int foo_gpu(int dnum) { return 456; }
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int foo(int dnum) { return 123; }
; int dnum;
; int main() {
;   int rrr;
;   #pragma omp target variant dispatch
;   {
;      rrr = foo(dnum);
;   }
;   return rrr;
; }
;
; The dispatch code looks like this:
;
;   %available = call i32 @__tgt_is_device_available(i64 %{{.*}}, ptr inttoptr (i64 15 to ptr))
;   %dispatch = icmp ne i32 %available, 0
;   br label %dispatch.check
;
; dispatch.check:
;   br i1 %dispatch, label %variant.call, label %base.call
;
; variant.call:
;   call void @main.foo_gpu.wrapper(ptr %rrr)
;   br label %if.end
;
; base.call:
;   %2 = load i32, ptr @dnum
;   %call.clone = call i32 @foo(i32 %2)
;   store i32 %call.clone, ptr %rrr
;   br label %if.end
; ...
;
; define internal void @main.foo_gpu.wrapper(ptr %rrr) {
;   %0 = load i32, ptr @dnum
;   %variant = call i32 @foo_gpu(i32 %0)
;   store i32 %variant, ptr %rrr
; }
;

;
; CHECK: [[CALL:%[a-zA-Z._0-9]+]] = call i32 @__tgt_is_device_available(i64 %{{.*}}, ptr inttoptr (i64 15 to ptr))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne i32 [[CALL]], 0
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]

; CHECK-DAG: [[VARIANTLBL]]:
; CHECK-NEXT: call void @[[VARIANT_WRAPPER:[^ ,]*foo_gpu.wrapper[^ ,)]*]](ptr %rrr)

; CHECK-DAG: [[BASELBL]]:
; CHECK: [[BASE_ARG:%[^ ]+]] = load i32, ptr @dnum
; CHECK: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(i32 [[BASE_ARG]])
; CHECK-NEXT: store i32 [[BASE]], ptr %rrr

; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](ptr %rrr)
; CHECK: [[ARG:%[^ ]+]] = load i32, ptr @dnum
; CHECK: [[VARIANT:%[^ ]+]] = call i32 @foo_gpu(i32 [[ARG]])
; CHECK: store i32 [[VARIANT]], ptr %rrr

; ModuleID = 'target_variant_dispatch_intfunc.c'
source_filename = "target_variant_dispatch_intfunc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@dnum = common dso_local global i32 0, align 4

define dso_local i32 @foo_gpu(i32 %dnum) {
entry:
  %dnum.addr = alloca i32, align 4
  store i32 %dnum, ptr %dnum.addr, align 4
  ret i32 456
}

define dso_local i32 @foo(i32 %dnum) #1 {
entry:
  %dnum.addr = alloca i32, align 4
  store i32 %dnum, ptr %dnum.addr, align 4
  ret i32 123
}

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %rrr = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %EmptyBB1

; Manually added these two empty BBs
EmptyBB1:
  br label %EmptyBB2

EmptyBB2:
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = load i32, ptr @dnum, align 4
  %call = call i32 @foo(i32 %1)
  store i32 %call, ptr %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %2 = load i32, ptr %rrr, align 4
  ret i32 %2
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" }


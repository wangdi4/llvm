; RUN: not opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: not opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; void bar_variant(int AAA) {}
;
; #pragma omp declare variant(bar_variant) match(construct={target variant dispatch}, device={arch(gen)})
; void bar(int BBB, ...) {}
;
; void foo(int aaa) {
;   #pragma omp target variant dispatch
;   bar(aaa);
; }
;
; Note: Test src will not be allowed by Frontend

; CHECK: Function '_Z11bar_variantiz' exists, but has an unexpected type.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @_Z11bar_variantiz(i32 noundef %AAA)  {
entry:
  %AAA.addr = alloca i32, align 4
  store i32 %AAA, ptr %AAA.addr, align 4
  ret void
}

define dso_local void @_Z3bariz(i32 noundef %BBB, ...) #1 {
entry:
  %BBB.addr = alloca i32, align 4
  store i32 %BBB, ptr %BBB.addr, align 4
  ret void
}

define dso_local void @_Z3fooi(i32 noundef %aaa) {
entry:
  %aaa.addr = alloca i32, align 4
  store i32 %aaa, ptr %aaa.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  %1 = load i32, ptr %aaa.addr, align 4
  call void (i32, ...) @_Z3bariz(i32 noundef %1)  [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z11bar_variantiz;construct:target_variant_dispatch;arch:gen" }


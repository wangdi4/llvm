; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s 2>&1 | FileCheck %s

; // C++ source
; // #include <stdio.h>
; void __attribute__((nothrow,noinline))  foo_XeHP(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION for XeHP ***\n");
; }
; void __attribute__((nothrow,noinline))  foo_XeLP(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION for XeLP ***\n");
; }
; #pragma omp declare variant(foo_XeHP) match(construct={dispatch}, device={arch(XeLP)})
; #pragma omp declare variant(foo_XeLP) match(construct={dispatch}, device={arch(XeHP)})
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0)
;     foo(0,ptr);
;   return 0;
; }
;
; CHECK: warning:{{.*}} Found multiple variants for dispatch. Only one will be used in the current implementation. The variant to be used is '_Z8foo_XeLPiPi'
;
; Variant call
; CHECK-LABEL: if.then:
; CHECK:         call void @_Z8foo_XeLPiPi(i32 0, ptr %{{.*}})
;
; Base call
; CHECK-LABEL: if.else:
; CHECK:         call void @_Z3fooiPi(i32 noundef 0, ptr noundef %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z8foo_XeHPiPi(i32 noundef %aaa, ptr noundef %bbb) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

define dso_local void @_Z8foo_XeLPiPi(i32 noundef %aaa, ptr noundef %bbb) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

define dso_local void @_Z3fooiPi(i32 noundef %aaa, ptr noundef %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca ptr, align 8
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  %1 = load ptr, ptr %ptr, align 8
  call void @_Z3fooiPi(i32 noundef 0, ptr noundef %1) [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z8foo_XeLPiPi;construct:dispatch;arch:XeHP;;name:_Z8foo_XeHPiPi;construct:dispatch;arch:XeLP" }

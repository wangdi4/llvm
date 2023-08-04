; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; Check that OMP5.1 dispatch with interop works with vararg functions.
; The interop obj is expected to be the last explicit formal parameter in the
; variant function's declaration. In the variant function call the interop obj
; must be inserted into the corresponding position among the actual arguments.
;
; // C source
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int a, int b, omp_interop_t interop, ...) {
;   // printf("VARIANT FUNCTION\n");
; }
;
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(target))
; void __attribute__((nothrow,noinline))  foo(int a, int b, ...) {
;   // printf("BASE FUNCTION\n");
; }
;
; int main() {
;   #pragma omp dispatch
;     foo(111, 222, 333);
;   return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z7foo_gpuiiPvz(i32 noundef %a, i32 noundef %b, ptr noundef %interop, ...) {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %interop.addr = alloca ptr, align 8
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  store ptr %interop, ptr %interop.addr, align 8
  ret void
}

define dso_local void @_Z3fooiiz(i32 noundef %a, i32 noundef %b, ...) #1 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  ret void
}

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]

  call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333) [ "QUAL.OMP.DISPATCH.CALL"() ]

; variant call: interop obj is right before the vararg list (333)
; CHECK-DAG: call void (i32, i32, ptr, ...) @_Z7foo_gpuiiPvz(i32 111, i32 222, ptr %interop.obj{{.*}}, i32 333)

; base call
; CHECK-DAG: call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333)


  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiiPvz;construct:dispatch;arch:gen;interop:target" }

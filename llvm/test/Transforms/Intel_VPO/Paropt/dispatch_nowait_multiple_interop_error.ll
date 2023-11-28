; RUN: not opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s 2>&1 | FileCheck %s
; RUN: not opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s 2>&1 | FileCheck %s

; // C++ source:
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, omp_interop_t iop1, omp_interop_t iop2) {
;   // printf("\n *** VARIANT FUNCTION (NOWAIT) ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync), interop(target))
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0) nowait
;     foo(0,ptr);
;   return 0;
; }

; OMP5.1 allows multiple interop operands in append_args,
; but currently we only support one interop in append_args.
;
; CHECK: error:{{.*}} Found multiple interop in append_args. This is still unsupported.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z7foo_gpuiPiPvS0_(i32 noundef %aaa, ptr noundef %bbb, ptr noundef %iop1, ptr noundef %iop2) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %iop1.addr = alloca ptr, align 8
  %iop2.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %iop1, ptr %iop1.addr, align 8
  store ptr %iop2, ptr %iop2.addr, align 8
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %ptr, ptr null, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.NOWAIT"() ]

  %2 = load ptr, ptr %ptr, align 8
  call void @_Z3fooiPi(i32 noundef 0, ptr noundef %2) [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiPiPvS0_;construct:dispatch;arch:gen;interop:targetsync;interop:target" }

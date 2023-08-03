; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; Check that target variant dispatch with interop works with vararg functions.
; This test is for the (default) "-vpo-paropt-put-interop-after-vararg=false" behavior.
; The interop obj is expected to be the last explicit formal parameter in the
; variant function's declaration. In the variant function call the interop obj
; must be inserted into the corresponding position among the actual arguments.
;
; // C source
; #include <stdio.h>
; #include <omp.h>
;
; void __attribute__((nothrow,noinline))  foo_gpu(int a, int b, omp_interop_t interop, ...) {
;   printf("VARIANT FUNCTION\n");
; }
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; void __attribute__((nothrow,noinline))  foo(int a, int b, ...) {
;   printf("BASE FUNCTION\n");
; }
;
; int main() {
;
;   #pragma omp target variant dispatch
;     foo(111, 222, 333);
;
;   return 0;
; }
;
; base call
; CHECK-DAG: call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333)
; variant call: interop obj is right before the vararg list (333)
; CHECK-DAG: call void (i32, i32, ptr, ...) @_Z7foo_gpuiiPvz(i32 111, i32 222, ptr %interop.obj{{.*}}, i32 333)

; ModuleID = 'target_variant_dispatch_interop_before_vararg.cpp'
source_filename = "target_variant_dispatch_interop_before_vararg.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [18 x i8] c"VARIANT FUNCTION\0A\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"BASE FUNCTION\0A\00", align 1

define dso_local void @_Z7foo_gpuiiPvz(i32 noundef %a, i32 noundef %b, ptr noundef %interop, ...) {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %interop.addr = alloca ptr, align 8
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  store ptr %interop, ptr %interop.addr, align 8
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str)
  ret void
}

declare dso_local i32 @printf(ptr noundef, ...)

define dso_local void @_Z3fooiiz(i32 noundef %a, i32 noundef %b, ...) #2 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, ptr %a.addr, align 4
  store i32 %b, ptr %b.addr, align 4
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str.1)
  ret void
}

define dso_local noundef i32 @main()  {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]

  call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333) [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #2 = { "openmp-variant"="name:_Z7foo_gpuiiPvz;construct:target_variant_dispatch;arch:gen" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}

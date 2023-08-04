; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER0,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER1,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER1,ALL

; // Test the interop_position attribute used for cases where the
; // interop obj is not the last parameter of the variant function.
; // Currently Clang does not emit the interop_position attribute so we
; // hand-modified the IR as follows to create this LIT test:
; //  - swap order of foo_gpu's 2nd and 3rd args (interop and bbb)
; //  - add the "interop_position:2" substring to the openmp-variant string attribute.
;
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, omp_interop_t interop){}
; // foo_gpu's IR is hand-modified into:  foo_gpu(int aaa, omp_interop_t interop, int *bbb){}
;
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync))
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb){}
;
; int main() {
;   int *ptr;
;   #pragma omp dispatch device(0)
;     foo(0,ptr);
;   return 0;
; }
;
; Check that we call foo_gpu with interop_obj as the second argument instead of the last.
;
; VER0: [[INTEROPOBJ:%[^ ]+]] = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
; VER1: [[INTEROPOBJ:%[^ ]+]] = call ptr @__tgt_get_interop_obj(ptr @.kmpc_loc{{.*}}, i32 1, i32 0, ptr null, i64 0, i32 %my.tid, ptr %current.task)
; ALL:  call void @_Z7foo_gpuiPiPv(i32 0, ptr [[INTEROPOBJ]], ptr %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z7foo_gpuiPiPv(i32 noundef %aaa, ptr noundef %interop, ptr noundef %bbb) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %interop.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %interop, ptr %interop.addr, align 8
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

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;interop:targetsync;interop_position:2" }

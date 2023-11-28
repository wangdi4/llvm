; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefixes=VER1,ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefixes=VER1,ALL

; // C++ source
; int __attribute__((nothrow,noinline))  foo_gpu(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
;   return 0;
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)})
; int __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
;   return 0;
; }
; int main() {
;   int *ptr;
;   int aaa, rrr;
;   #pragma omp dispatch device(0) novariants(aaa>5)
;     rrr = foo(0,ptr);
;   return rrr;
; }
;
; ALL:       call i32 @__tgt_is_device_available
; ALL:       %dovariants = icmp eq i1 %tobool, false
; ALL-NEXT:  %available = and i1 %{{.*}}, %dovariants
; ALL-NEXT:  br i1 %available, label %if.then, label %if.else
;
; ALL-DAG:   if.then:
; ALL-NEXT:  %variant = call i32 @_Z7foo_gpuiPi(i32 0, ptr %{{.*}})
; VER1-NEXT: %my.tid = load i32, ptr @"@tid.addr"
; VER1-NEXT: %current.task = call ptr @__kmpc_get_current_task(i32 %my.tid)
; VER1-NEXT: call void @__tgt_target_sync(ptr @.kmpc_loc{{.*}}, i32 %my.tid, ptr %current.task, ptr null)
; ALL-NEXT:  br label %if.end
;
; ALL-DAG:   if.else:
; ALL-NEXT:  %call = call noundef i32 @_Z3fooiPi(i32 noundef 0, ptr noundef %{{.*}})
; ALL-NEXT:  br label %if.end
;
; ALL:       if.end:
; ALL-NEXT:  %callphi = phi i32 [ %variant, %if.then ], [ %call, %if.else ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local noundef i32 @_Z7foo_gpuiPi(i32 noundef %aaa, ptr noundef %bbb) {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret i32 0
}

define dso_local noundef i32 @_Z3fooiPi(i32 noundef %aaa, ptr noundef %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret i32 0
}

define dso_local noundef i32 @main() {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca ptr, align 8
  %aaa = alloca i32, align 4
  %rrr = alloca i32, align 4
  %.capture_expr.0 = alloca i8, align 1
  store i32 0, ptr %retval, align 4
  %0 = load i32, ptr %aaa, align 4
  %cmp = icmp sgt i32 %0, 5
  %frombool = zext i1 %cmp to i8
  store i8 %frombool, ptr %.capture_expr.0, align 1
  %1 = load i8, ptr %.capture_expr.0, align 1
  %tobool = trunc i8 %1 to i1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.NOVARIANTS"(i1 %tobool) ]

  %3 = load ptr, ptr %ptr, align 8
  %call = call noundef i32 @_Z3fooiPi(i32 noundef 0, ptr noundef %3) [ "QUAL.OMP.DISPATCH.CALL"() ]
  store i32 %call, ptr %rrr, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISPATCH"() ]

  %4 = load i32, ptr %rrr, align 4
  ret i32 %4
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:_Z7foo_gpuiPi;construct:dispatch;arch:gen" }

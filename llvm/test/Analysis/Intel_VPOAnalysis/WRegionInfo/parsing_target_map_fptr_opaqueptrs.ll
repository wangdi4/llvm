; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -disable-output -S %s 2>&1 | FileCheck %s

; Original code:
;
; // extern void foo(void);
; void bar() {
;   void (*fptr)(void) ;// = foo;
; #pragma omp target firstprivate(fptr)
;   fptr();
; }

; CHECK: MAP clause (size=1): CHAIN,FPTR(<ptr %fptr.val, ptr %fptr.val, i64 0, 32 (0x0000000000000020), null, null>

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @bar() #0 {
entry:
  %fptr = alloca ptr, align 8
  %fptr.map.ptr.tmp = alloca ptr, align 8
  %fptr.val = load ptr, ptr %fptr, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:FPTR"(ptr %fptr.val, ptr %fptr.val, i64 0, i64 32, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %fptr.map.ptr.tmp, ptr null, i32 1) ]

  store ptr %fptr.val, ptr %fptr.map.ptr.tmp, align 8
  %1 = load ptr, ptr %fptr.map.ptr.tmp, align 8
  call void %1()

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 47770844, !"_Z3bar", i32 3, i32 0, i32 0}

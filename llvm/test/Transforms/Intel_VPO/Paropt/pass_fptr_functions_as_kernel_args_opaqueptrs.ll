; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s

; Test src:
;
; extern void foo(void);
; void bar() {
;   void (*fptr)(void)  = foo;
; #pragma omp target firstprivate(fptr)
;   fptr();
; }

; Make sure when a function is used as an FPTR map operand, we create an
; argument for it in the outlined function.

; CHECK: define internal void @__omp_offloading{{.*}}bar_l3(ptr %foo)
; CHECK: store ptr %foo, ptr %fptr.map.ptr.tmp.priv, align 8
; CHECK: %fptr.load = load ptr, ptr %fptr.map.ptr.tmp.priv, align 8
; CHECK: call void %fptr.load()

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @bar() #0 {
entry:
  %fptr = alloca ptr, align 8
  %fptr.map.ptr.tmp = alloca ptr, align 8
  store ptr @foo, ptr %fptr, align 8
  %fptr.val = load ptr, ptr %fptr, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:FPTR"(ptr @foo, ptr @foo, i64 0, i64 32, ptr null, ptr null), ; MAP type: 32 = 0x20 = TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %fptr.map.ptr.tmp, ptr null, i32 1) ]

  store ptr @foo, ptr %fptr.map.ptr.tmp, align 8
  %fptr.load = load ptr, ptr %fptr.map.ptr.tmp, align 8
  call void %fptr.load()

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare dso_local void @foo()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 47770844, !"_Z3bar", i32 3, i32 0, i32 0}

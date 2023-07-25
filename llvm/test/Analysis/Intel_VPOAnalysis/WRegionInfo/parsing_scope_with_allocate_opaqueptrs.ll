; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s

; This test checks if 'allocate' clause on 'scope' construct is parsed correctly.
; omp_large_cap_mem_alloc is an enum and it's value is 2.

; Test src:
;
; void foo() {
;   float array[100];
;   float scalar = 15.0;
; #pragma omp scope private(array) firstprivate(scalar)                       \
;     allocate(omp_large_cap_mem_alloc                                           \
;              : array) allocate(scalar)
;   { array[50] = scalar; }
; }

; CHECK: PRIVATE clause (size=1): TYPED(ptr [[ARRAY:%[a-zA-Z.0-9]+]], TYPE: float, NUM_ELEMENTS: i64 100) 
; CHECK: FIRSTPRIVATE clause (size=1): TYPED(ptr [[SCALAR:%[a-zA-Z.0-9]+]], TYPE: float, NUM_ELEMENTS: i32 1) 
; CHECK: ALLOCATE clause (size=2): (Align(4), ptr [[ARRAY]], i64 2) (Align(4), ptr [[SCALAR]], NULL) 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %array = alloca [100 x float], align 16
  %scalar = alloca float, align 4
  store float 1.500000e+01, ptr %scalar, align 4
  %array.begin = getelementptr inbounds [100 x float], ptr %array, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %array, float 0.000000e+00, i64 100),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %scalar, float 0.000000e+00, i32 1),
    "QUAL.OMP.ALLOCATE"(i64 4, ptr %array, i64 2),
    "QUAL.OMP.ALLOCATE"(i64 4, ptr %scalar) ]

  %1 = load float, ptr %scalar, align 4
  %arrayidx = getelementptr inbounds [100 x float], ptr %array, i64 0, i64 50
  store float %1, ptr %arrayidx, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare token @llvm.directive.region.entry() 
declare void @llvm.directive.region.exit(token)

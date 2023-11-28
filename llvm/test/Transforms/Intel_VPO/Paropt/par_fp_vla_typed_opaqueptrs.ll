; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; Test src:

; void f(int n) {
;   int a[n];
;   #pragma omp parallel firstprivate(a)
;     ;
; }
;
; (Test IR was reduced from the IR for the above test)

; Make sure that firstprivate initialization of a typed VLA uses memcpy
; with the size of the array.

; CHECK: define internal void @f{{.*}}(ptr %tid, ptr %bid, ptr [[SIZE_ADDR:%size.addr]], ptr [[A_ORIG:%vla]])
; CHECK:  [[SIZE:%size.*]] = load i64, ptr [[SIZE_ADDR]], align 8
; CHECK:  [[A_FP:%vla.fpriv]] = alloca i32, i64 [[SIZE]], align 4
; CHECK:  [[SIZE_BYTES:%.*]] = mul i64 4, [[SIZE]]
; CHECK:  call void @llvm.memcpy.p0.p0.i64(ptr align 4 [[A_FP]], ptr align 4 [[A_ORIG]], i64 [[SIZE_BYTES]], i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
define void @f(ptr %vla, i64 %size) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %vla, i32 0, i64 %size) ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

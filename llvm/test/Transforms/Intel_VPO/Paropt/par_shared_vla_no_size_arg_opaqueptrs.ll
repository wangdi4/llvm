; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; Test src:

; void f(int n) {
;   int a[n];
;   #pragma omp parallel shared(a)
;     ;
; }
;
; (Test IR was reduced from the IR for the above test)

; Make sure that the size of the SHARED VLA is not passed into the
; outlined function.

; CHECK: define internal void @f{{.*}}(ptr %tid, ptr %bid, ptr %vla)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
define void @f(ptr %vla, i64 %size) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %vla, i32 0, i64 %size) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

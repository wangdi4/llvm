; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s | FileCheck %s -check-prefix=DBG
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s | FileCheck %s -check-prefix=DBG
;
; Test src:
;
; #define N 1000000
; void bar() {
;     int  B[N];
; #pragma omp task firstprivate(B) // OK with private(B) or shared(B)
;     {
;         B[44] = 123;
;     }
; }

; Make sure that memcpy is used to copy the array into the task thunk.
; In the outlined task, we should not copy the whole array, but use the
; data directly from the thunk.

; Caller side
; TFORM: [[BPRIV:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %.privates, i32 0, i32 0
; TFORM: call void @llvm.memcpy{{.*}}align 4 [[BPRIV]], ptr align 4 %B, i64 4000000

; Make sure that IR verification doesn't fail with -debug.
; DBG-NOT: verification of{{.*}}failed!

; Task side
; TFORM: define{{.*}}TASK
; TFORM-NOT: fpriv
; TFORM-NOT: call{{.*}}memcpy

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @bar() {
entry:
  %B = alloca [1000000 x i32], align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
  "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %B, i32 0, i64 1000000) ]

  %arrayidx = getelementptr inbounds [1000000 x i32], ptr %B, i64 0, i64 44
  store i32 123, ptr %arrayidx, align 16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

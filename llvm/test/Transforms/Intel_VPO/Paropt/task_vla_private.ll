; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; #include <stdio.h>
;
; void foo(int n) {
;   int a[n];
;   a[1] = 1;
; #pragma omp task private(a)
;   {
;     a[1] = 2;
;     printf("%d\n", a[1]);
;   }
;   printf("%d\n", a[1]);
; }
;
; // int main() {
; //   foo(10);
; // }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; Check for the space allocated for the private copy.
; CHECK: %__struct.kmp_privates.t = type { ptr, i64, i64 }

define dso_local void @foo(i32 %n) {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 1
  store i32 1, ptr %arrayidx, align 4
  store i64 %1, ptr %omp.vla.tmp, align 8

; Check for computation of VLA size in bytes
; CHECK: %vla = alloca i32, i64 [[NUM_ELEMENTS:%[^, ]+]]
; CHECK: [[VLA_SIZE_IN_BYTES:%[^ ]+]] = mul i64 [[NUM_ELEMENTS]], 4

; Check that we call _task_alloc with total size of task_t_with_privates + vla_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 96, [[VLA_SIZE_IN_BYTES]]
; CHECK: {{[^ ]+}} = call ptr @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that VLA size and offset are stored in the thunk
; CHECK: [[VLA_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[VLA_SIZE_IN_BYTES]], ptr [[VLA_SIZE_GEP]]
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 96, ptr [[VLA_OFFSET_GEP]]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, i32 0, i64 %1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]

; Inside the outlined function, check that the gep in the privates thunk, is linked to the array allocated in the buffer at the end.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}
; CHECK: %.privates = getelementptr inbounds %__struct.kmp_task_t_with_privates, ptr %taskt.withprivates, i32 0, i32 1
; CHECK: [[VLA_PRIV_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %.privates, i32 0, i32 0
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %.privates, i32 0, i32 2
; CHECK: [[VLA_OFFSET:%[^ ]+]] = load i64, ptr [[VLA_OFFSET_GEP]]
; CHECK: [[VLA_DATA:%[^ ]+]]  = getelementptr i8, ptr %taskt.withprivates, i64 [[VLA_OFFSET]]
; CHECK: store ptr [[VLA_DATA]], ptr [[VLA_PRIV_GEP]]

; Check that a load from VLA_PRIV_GEP is done to get the private copy of %vla, which replaces it inside the region.
; CHECK: [[VLA_PRIV_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[VLA_NEW:%[^ ]+]] = load ptr, ptr [[VLA_PRIV_GEP1]]
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds i32, ptr [[VLA_NEW]], i64 1
; CHECK: store i32 2, ptr [[GEP]]

  %4 = load i64, ptr %omp.vla.tmp, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %vla, i64 1
  store i32 2, ptr %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds i32, ptr %vla, i64 1
  %5 = load i32, ptr %arrayidx2, align 4
  %call = call i32 (ptr, ...) @printf(ptr @.str, i32 %5)

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]


  %arrayidx3 = getelementptr inbounds i32, ptr %vla, i64 1
  %6 = load i32, ptr %arrayidx3, align 4
  %call4 = call i32 (ptr, ...) @printf(ptr @.str, i32 %6)
  %7 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %7)
  ret void
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
declare void @llvm.stackrestore(ptr)

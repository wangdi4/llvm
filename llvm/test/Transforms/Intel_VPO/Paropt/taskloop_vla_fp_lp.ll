; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; #include <stdio.h>
;
; void foo(int n) {
;   double a[n];
;   a[1] = 1;
;
; //#pragma omp parallel
; //#pragma omp single
; #pragma omp taskloop firstprivate(a) lastprivate(a) num_tasks(10)
;   for (int i = 0; i < n; i++)
;   {
;     a[1] = i+1;
;     printf("%lf %p\n", a[1], &a[1]);
;   }
;   printf("%lf %p\n", a[1], &a[1]);
; }
;
; //int main() {
; //  foo(10);
; //}

; Check for the space allocated for the vla in the thunk.
; In privates struct, there should be one ptr, followed by two i64s.
; CHECK: %__struct.kmp_privates.t = type { ptr, i64, i64, i64, i64, i32 }
; There should be one ptr in the shared struct for lastprivate copyout.
; CHECK: %__struct.shared.t = type { ptr, ptr }

; Check for computation of VLA size in bytes
; CHECK: %vla = alloca double, i64 [[NUM_ELEMENTS:%[^, ]+]]
; CHECK: [[VLA_SIZE_IN_BYTES:%[^ ]+]] = mul i64 [[NUM_ELEMENTS]], 8

; Check that the address of %vla is stored into the shareds struct to be used in lastprivate copyout later.
; CHECK: [[VLA_SHR_GEP:%[^ ]+]] = getelementptr inbounds %__struct.shared.t, ptr {{[^ ]*}}, i32 0, i32 0
; CHECK: store ptr %vla, ptr [[VLA_SHR_GEP]]

; Check that we call _task_alloc with total size of task_t_with_privates + vla_size
; CHECK: [[TOTAL_SIZE:%[^ ]+]] = add i64 120, [[VLA_SIZE_IN_BYTES]]
; CHECK: [[TASK_ALLOC:[^ ]+]] = call ptr @__kmpc_omp_task_alloc({{.*}}i64 [[TOTAL_SIZE]]{{.*}})

; Check that VLA size and offset are stored in the thunk
; CHECK: [[VLA_SIZE_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 1
; CHECK: store i64 [[VLA_SIZE_IN_BYTES]], ptr [[VLA_SIZE_GEP]]
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ]+}}, i32 0, i32 2
; CHECK: store i64 120, ptr [[VLA_OFFSET_GEP]]

; Check that the local buffer space for %vla, allocated with __kmpc_alloc, is initialized (as it is firstprivate).
; CHECK: [[VLA_BUFFER:%[^ ]+]] = getelementptr i8, ptr [[TASK_ALLOC]], i64 120
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[VLA_BUFFER]], ptr align 8 %vla, i64 [[VLA_SIZE_IN_BYTES]], i1 false)


; Inside the outlined function.
; CHECK: define internal void @{{.*}}DIR.OMP.TASK{{.*}}

; Check that the double* in the privates thunk is linked to the array allocated in the buffer at the end.
; CHECK: [[VLA_PRIV_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^, ]+}}, i32 0, i32 0
; CHECK: [[VLA_OFFSET_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^, ]+}}, i32 0, i32 2
; CHECK: [[VLA_OFFSET:%[^ ]+]] = load i64, ptr [[VLA_OFFSET_GEP]]
; CHECK: [[VLA_DATA:%[^ ]+]]  = getelementptr i8, ptr %taskt.withprivates, i64 [[VLA_OFFSET]]
; CHECK: store ptr [[VLA_DATA]], ptr [[VLA_PRIV_GEP]]

; Check that a load from VLA_PRIV_GEP is done to get a double*.
; CHECK: [[VLA_PRIV_GEP1:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr %{{[^ ,]+}}, i32 0, i32 0
; CHECK: [[VLA_NEW:%[^ ]+]] = load ptr, ptr [[VLA_PRIV_GEP1]]

; Check that we load the size of vla from the privates thunk.
; CHECK: [[VLA_SIZE_NEW_GEP:%[^ ]+]] = getelementptr inbounds %__struct.kmp_privates.t, ptr {{[^ ]+}}, i32 0, i32 1
; CHECK: [[VLA_SIZE_NEW:%[^ ]+]] = load i64, ptr [[VLA_SIZE_NEW_GEP]]


; Check that we access the original %vla through the shareds thunk for lastprivate copyout later.
; CHECK: [[VLA_ORIG_GEP:%[^ ]+]] = getelementptr inbounds %__struct.shared.t, ptr {{[^ ]+}}, i32 0, i32 0
; CHECK: [[VLA_ORIG:%[^ ]+]] = load ptr, ptr [[VLA_ORIG_GEP]]


; Check for replacement of %vla with VLA_NEW inside the region.
; CHECK: [[GEP1:%[^ ]+]] = getelementptr inbounds double, ptr [[VLA_NEW]], i64 1
; CHECK: [[GEP2:%[^ ]+]] = getelementptr inbounds double, ptr [[VLA_NEW]], i64 1
; CHECK: [[GEP3:%[^ ]+]] = getelementptr inbounds double, ptr [[VLA_NEW]], i64 1
; CHECK: {{[^ ]+}} = call i32 (ptr, ...) @printf(ptr @.str, double {{.*}}, ptr [[GEP3]])

; Check for the lastprivate copyout code.
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[VLA_ORIG]], ptr align 8 [[VLA_NEW]], i64 [[VLA_SIZE_NEW]], i1 false)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"%lf %p\0A\00", align 1

define dso_local void @foo(i32 %n) {
entry:
  %n.addr = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %i = alloca i32, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = call ptr @llvm.stacksave()
  store ptr %2, ptr %saved_stack, align 8
  %vla = alloca double, i64 %1, align 16
  store i64 %1, ptr %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds double, ptr %vla, i64 1
  store double 1.000000e+00, ptr %arrayidx, align 8
  %3 = load i32, ptr %n.addr, align 4
  store i32 %3, ptr %.capture_expr., align 4
  %4 = load i32, ptr %.capture_expr., align 4
  %sub = sub nsw i32 %4, 0
  %sub2 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, ptr %.capture_expr.1, align 4
  %5 = load i32, ptr %.capture_expr., align 4
  %cmp = icmp slt i32 0, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i64 0, ptr %.omp.lb, align 8
  %6 = load i32, ptr %.capture_expr.1, align 4
  %conv = sext i32 %6 to i64
  store i64 %conv, ptr %.omp.ub, align 8
  store i64 %1, ptr %omp.vla.tmp, align 8
  %7 = load i64, ptr %omp.vla.tmp, align 8
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %vla, double 0.000000e+00, i64 %1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %vla, double 0.000000e+00, i64 %1),
    "QUAL.OMP.NUM_TASKS"(i32 10),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i64 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i64 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1) ]

  %9 = load i64, ptr %omp.vla.tmp, align 8
  %10 = load i64, ptr %.omp.lb, align 8
  %conv4 = trunc i64 %10 to i32
  store i32 %conv4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %11 = load i32, ptr %.omp.iv, align 4
  %conv5 = sext i32 %11 to i64
  %12 = load i64, ptr %.omp.ub, align 8
  %cmp6 = icmp ule i64 %conv5, %12
  br i1 %cmp6, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %13, 1
  %add8 = add nsw i32 0, %mul
  store i32 %add8, ptr %i, align 4
  %14 = load i32, ptr %i, align 4
  %add9 = add nsw i32 %14, 1
  %conv10 = sitofp i32 %add9 to double
  %arrayidx11 = getelementptr inbounds double, ptr %vla, i64 1
  store double %conv10, ptr %arrayidx11, align 8
  %arrayidx12 = getelementptr inbounds double, ptr %vla, i64 1
  %15 = load double, ptr %arrayidx12, align 8
  %arrayidx13 = getelementptr inbounds double, ptr %vla, i64 1
  %call = call i32 (ptr, ...) @printf(ptr @.str, double %15, ptr %arrayidx13)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4
  %add14 = add nsw i32 %16, 1
  store i32 %add14, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASKLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %arrayidx15 = getelementptr inbounds double, ptr %vla, i64 1
  %17 = load double, ptr %arrayidx15, align 8
  %arrayidx16 = getelementptr inbounds double, ptr %vla, i64 1
  %call17 = call i32 (ptr, ...) @printf(ptr @.str, double %17, ptr %arrayidx16)
  %18 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %18)
  ret void
}

declare ptr @llvm.stacksave()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr, ...)
declare void @llvm.stackrestore(ptr)

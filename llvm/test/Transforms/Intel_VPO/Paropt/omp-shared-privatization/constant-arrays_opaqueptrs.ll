; RUN: opt -passes='default<O3>' -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void car(float *A, float *B, int N) {
;   float C[2] = { 3.3f, 4.4f };
;   float D[2] = { 2.6f, 6.7f };
;
; #pragma omp parallel for
;   for(int i = 0; i < N-1; ++i)
;     A[i] += (B[i] * C[0]) + D[1];
; }

; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__const.car.C = private unnamed_addr constant [2 x float] [float 0x400A666660000000, float 0x40119999A0000000], align 4
@__const.car.D = private unnamed_addr constant [2 x float] [float 0x4004CCCCC0000000, float 0x401ACCCCC0000000], align 4

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @car(ptr noundef %A, ptr noundef %B, i32 noundef %N) {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %C = alloca [2 x float], align 4
  %D = alloca [2 x float], align 4
  %tmp = alloca i32, align 4
  %.capture_expr.8 = alloca i32, align 4
  %.capture_expr.9 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %C, ptr align 4 @__const.car.C, i64 8, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %D, ptr align 4 @__const.car.D, i64 8, i1 false)
  %0 = load i32, ptr %N.addr, align 4
  %sub = sub nsw i32 %0, 1
  store i32 %sub, ptr %.capture_expr.8, align 4
  %1 = load i32, ptr %.capture_expr.8, align 4
  %sub1 = sub nsw i32 %1, 0
  %sub2 = sub nsw i32 %sub1, 1
  %add = add nsw i32 %sub2, 1
  %div = sdiv i32 %add, 1
  %sub3 = sub nsw i32 %div, 1
  store i32 %sub3, ptr %.capture_expr.9, align 4
  %2 = load i32, ptr %.capture_expr.8, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.9, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %array.begin = getelementptr inbounds [2 x float], ptr %C, i32 0, i32 0
  %array.begin14 = getelementptr inbounds [2 x float], ptr %D, i32 0, i32 0

; The behavior of argument promotion used to be different in typed and opaque
; pointers. In both the typed and opaque pointer cases, only one element of
; the 2 element array is used in the callee.

; In the typed pointer case, both elements of C and D were passed through the
; fork() interface as i64 values: "i64 4651317697086436147, i64 4672034252792424038".

; In the opaque pointer case, only a single element of C and D is passed.
; Because only a single element of C and D is passed in the opaque pointer
; case, it is necessary to sign extend each value to 64-bits before it is
; passed to the fork() on the caller side and then truncate it when it gets
; to the callee. So, the values passed are: "i64 1079194419, i64 1087792742".
; CHECK: call {{.*}} @__kmpc_fork_call({{.*}}, i32 6, ptr nonnull [[OUTLINED_CAR:@.+]], ptr %A, ptr %B, i64 1079194419, i64 1087792742, i64 0,

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %C, float 0.000000e+00, i64 2),
    "QUAL.OMP.SHARED:TYPED"(ptr %D, float 0.000000e+00, i64 2),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp4 = icmp sle i32 %6, %7
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add5 = add nsw i32 0, %mul
  store i32 %add5, ptr %i, align 4
  %9 = load ptr, ptr %B.addr, align 8
  %10 = load i32, ptr %i, align 4
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds float, ptr %9, i64 %idxprom
  %11 = load float, ptr %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [2 x float], ptr %C, i64 0, i64 0
  %12 = load float, ptr %arrayidx6, align 4
  %mul7 = fmul fast float %11, %12
  %arrayidx8 = getelementptr inbounds [2 x float], ptr %D, i64 0, i64 1
  %13 = load float, ptr %arrayidx8, align 4
  %add9 = fadd fast float %mul7, %13
  %14 = load ptr, ptr %A.addr, align 8
  %15 = load i32, ptr %i, align 4
  %idxprom10 = sext i32 %15 to i64
  %arrayidx11 = getelementptr inbounds float, ptr %14, i64 %idxprom10
  %16 = load float, ptr %arrayidx11, align 4
  %add12 = fadd fast float %16, %add9
  store float %add12, ptr %arrayidx11, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, ptr %.omp.iv, align 4
  %add13 = add nsw i32 %17, 1
  store i32 %add13, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

; CHECK: define internal void [[OUTLINED_CAR]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture [[ABASE:%.+]], ptr nocapture readonly [[BBASE:%.+]], i64 [[CVAL:%.+]], i64 [[DVAL:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, ptr [[BBASE]],
; CHECK:   [[BVAL:%.+]] = load float, ptr [[BADDR]]
; CHECK:   [[MUL0:%.+]] = fmul fast float [[BVAL]], 0x400A666660000000
; CHECK:   [[ADD0:%.+]] = fadd fast float [[MUL0]], 0x401ACCCCC0000000
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]],
; CHECK:   [[AVAL:%.+]] = load float, ptr [[AADDR]]
; CHECK:   [[ADD1:%.+]] = fadd fast float [[ADD0]], [[AVAL]]
; CHECK:   store float [[ADD1]], ptr [[AADDR]]
; CHECK: }

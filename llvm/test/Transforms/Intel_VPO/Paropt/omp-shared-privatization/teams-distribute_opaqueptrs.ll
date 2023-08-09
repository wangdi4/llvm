; RUN: opt -passes='default<O3>' -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void baz(float * A, float * B, int N) {
; #pragma omp teams distribute parallel for
;   for (int I = 0; I < N; ++I)
;     A[I] = 3.0f * B[I];
; }

; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @baz(ptr noundef %A, ptr noundef %B, i32 noundef %N) {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.10 = alloca i32, align 4
  %.capture_expr.11 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  store i32 %N, ptr %N.addr, align 4

; CHECK: call {{.*}} @__kmpc_fork_teams({{.*}}, i32 3, ptr nonnull [[OUTLINED_BAZ_TEAMS:@.+]], ptr %A, ptr %B, i64 %{{.+}})

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %N.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.11, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.10, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.10, align 4
  %2 = load i32, ptr %.capture_expr.10, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.11, align 4
  %3 = load i32, ptr %.capture_expr.10, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %4 = load i32, ptr %.capture_expr.11, align 4
  store i32 %4, ptr %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1) ]
  %6 = load i32, ptr %.omp.lb, align 4
  store i32 %6, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %7 = load i32, ptr %.omp.iv, align 4
  %8 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %7, %8
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %9, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, ptr %I, align 4
  %10 = load ptr, ptr %B.addr, align 8
  %11 = load i32, ptr %I, align 4
  %idxprom = sext i32 %11 to i64
  %arrayidx = getelementptr inbounds float, ptr %10, i64 %idxprom
  %12 = load float, ptr %arrayidx, align 4
  %mul5 = fmul fast float 3.000000e+00, %12
  %13 = load ptr, ptr %A.addr, align 8
  %14 = load i32, ptr %I, align 4
  %idxprom6 = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %13, i64 %idxprom6
  store float %mul5, ptr %arrayidx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr %.omp.iv, align 4
  %add8 = add nsw i32 %15, 1
  store i32 %add8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

; CHECK: define internal void [[OUTLINED_BAZ_LOOP:@.+]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture writeonly [[ABASE:%.+]], ptr nocapture readonly [[BBASE:%.+]], i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, ptr [[BBASE]],
; CHECK:   [[VAL:%.+]] = load float, ptr [[BADDR]]
; CHECK:   [[MUL:%.+]] = fmul fast float [[VAL]], 3.000000e+00
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]],
; CHECK:   store float [[MUL]], ptr [[AADDR]]
; CHECK: }

; CHECK: define internal void [[OUTLINED_BAZ_TEAMS]](ptr nocapture readnone %tid, ptr nocapture readnone %bid, ptr nocapture writeonly [[ABASE:%.+]], ptr nocapture readonly [[BBASE:%.+]], i64 %{{.+}}) #{{[0-9]+}} {
; CHECK:   call {{.*}} @__kmpc_fork_call({{.*}}, i32 4, ptr nonnull [[OUTLINED_BAZ_LOOP]], ptr [[ABASE]], ptr [[BBASE]], i64 0, i64 %{{.+}})
; CHECK: }

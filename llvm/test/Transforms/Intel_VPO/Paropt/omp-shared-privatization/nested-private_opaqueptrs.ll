; RUN: opt -passes='default<O3>' -paropt=31 -S %s | FileCheck %s

; Test src:
;
; void test_nested_private(float * A, float * B, int N, int K) {
;   float X = 3.0f;
;
; #pragma omp teams
;   {
; #pragma omp distribute parallel for
;     for (int I = 0; I < N; ++I)
;       A[I] = X * B[I];
;
; #pragma omp distribute
;    for (int J = 0; J < N; J += K)
; #pragma omp parallel for private(X)
;      for (int I = J; I < J+K; ++I) {
;        X = fabsf(A[I]);
;        A[I] += X;
;      }
;   }
; }

; Check that shared A and B are passed to the outlined function by value. That
; depends on A's and B's references to be privatized inside the outlined parallel
; region by Paropt and then promoted to values by the argument promotion pass.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define dso_local void @test_nested_private(ptr noundef %A, ptr noundef %B, i32 noundef %N, i32 noundef %K) {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %K.addr = alloca i32, align 4
  %X = alloca float, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.12 = alloca i32, align 4
  %.capture_expr.13 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp9 = alloca i32, align 4
  %.capture_expr.17 = alloca i32, align 4
  %.capture_expr.18 = alloca i32, align 4
  %.capture_expr.19 = alloca i32, align 4
  %.omp.iv17 = alloca i32, align 4
  %.omp.lb18 = alloca i32, align 4
  %.omp.ub19 = alloca i32, align 4
  %J = alloca i32, align 4
  %tmp25 = alloca i32, align 4
  %.capture_expr.14 = alloca i32, align 4
  %.capture_expr.15 = alloca i32, align 4
  %.capture_expr.16 = alloca i32, align 4
  %.omp.iv34 = alloca i32, align 4
  %.omp.lb35 = alloca i32, align 4
  %.omp.ub36 = alloca i32, align 4
  %I41 = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  store i32 %K, ptr %K.addr, align 4
  store float 3.000000e+00, ptr %X, align 4

; CHECK: call {{.*}} @__kmpc_fork_teams({{.+}}, i32 5, ptr nonnull [[NESTED_PRIVATE_TEAMS:@.+]], ptr %A, ptr %B, i64 %{{.+}}, i64 %{{.+}}, i64 1077936128)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %N.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %K.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %X, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.13, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.12, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv17, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.17, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.16, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub36, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I41, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp25, i32 0, i32 1) ]
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.12, align 4
  %2 = load i32, ptr %.capture_expr.12, align 4
  %sub = sub nsw i32 %2, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.13, align 4
  %3 = load i32, ptr %.capture_expr.12, align 4
  %cmp = icmp slt i32 0, %3
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %4 = load i32, ptr %.capture_expr.13, align 4
  store i32 %4, ptr %.omp.ub, align 4
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %B.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %X, float 0.000000e+00, i32 1),
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
  %10 = load float, ptr %X, align 4
  %11 = load ptr, ptr %B.addr, align 8
  %12 = load i32, ptr %I, align 4
  %idxprom = sext i32 %12 to i64
  %arrayidx = getelementptr inbounds float, ptr %11, i64 %idxprom
  %13 = load float, ptr %arrayidx, align 4
  %mul5 = fmul fast float %10, %13
  %14 = load ptr, ptr %A.addr, align 8
  %15 = load i32, ptr %I, align 4
  %idxprom6 = sext i32 %15 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %14, i64 %idxprom6
  store float %mul5, ptr %arrayidx7, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, ptr %.omp.iv, align 4
  %add8 = add nsw i32 %16, 1
  store i32 %add8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %17 = load i32, ptr %N.addr, align 4
  store i32 %17, ptr %.capture_expr.17, align 4
  %18 = load i32, ptr %K.addr, align 4
  store i32 %18, ptr %.capture_expr.18, align 4
  %19 = load i32, ptr %.capture_expr.17, align 4
  %20 = load i32, ptr %.capture_expr.18, align 4
  %sub10 = sub nsw i32 0, %20
  %add11 = add nsw i32 %sub10, 1
  %sub12 = sub nsw i32 %19, %add11
  %21 = load i32, ptr %.capture_expr.18, align 4
  %div13 = sdiv i32 %sub12, %21
  %sub14 = sub nsw i32 %div13, 1
  store i32 %sub14, ptr %.capture_expr.19, align 4
  %22 = load i32, ptr %.capture_expr.17, align 4
  %cmp15 = icmp slt i32 0, %22
  br i1 %cmp15, label %omp.precond.then16, label %omp.precond.end60

omp.precond.then16:                               ; preds = %omp.precond.end
  store i32 0, ptr %.omp.lb18, align 4
  %23 = load i32, ptr %.capture_expr.19, align 4
  store i32 %23, ptr %.omp.ub19, align 4
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv17, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb18, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub19, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.16, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub36, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I41, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.14, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.capture_expr.15, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp25, i32 0, i32 1) ]
  %25 = load i32, ptr %.omp.lb18, align 4
  store i32 %25, ptr %.omp.iv17, align 4
  br label %omp.inner.for.cond20

omp.inner.for.cond20:                             ; preds = %omp.inner.for.inc56, %omp.precond.then16
  %26 = load i32, ptr %.omp.iv17, align 4
  %27 = load i32, ptr %.omp.ub19, align 4
  %cmp21 = icmp sle i32 %26, %27
  br i1 %cmp21, label %omp.inner.for.body22, label %omp.inner.for.end58

omp.inner.for.body22:                             ; preds = %omp.inner.for.cond20
  %28 = load i32, ptr %.omp.iv17, align 4
  %29 = load i32, ptr %.capture_expr.18, align 4
  %mul23 = mul nsw i32 %28, %29
  %add24 = add nsw i32 0, %mul23
  store i32 %add24, ptr %J, align 4
  %30 = load i32, ptr %J, align 4
  store i32 %30, ptr %.capture_expr.14, align 4
  %31 = load i32, ptr %J, align 4
  %32 = load i32, ptr %K.addr, align 4
  %add26 = add nsw i32 %31, %32
  store i32 %add26, ptr %.capture_expr.15, align 4
  %33 = load i32, ptr %.capture_expr.15, align 4
  %34 = load i32, ptr %.capture_expr.14, align 4
  %sub27 = sub i32 %33, %34
  %sub28 = sub i32 %sub27, 1
  %add29 = add i32 %sub28, 1
  %div30 = udiv i32 %add29, 1
  %sub31 = sub i32 %div30, 1
  store i32 %sub31, ptr %.capture_expr.16, align 4
  %35 = load i32, ptr %.capture_expr.14, align 4
  %36 = load i32, ptr %.capture_expr.15, align 4
  %cmp32 = icmp slt i32 %35, %36
  br i1 %cmp32, label %omp.precond.then33, label %omp.precond.end54

omp.precond.then33:                               ; preds = %omp.inner.for.body22
  store i32 0, ptr %.omp.lb35, align 4
  %37 = load i32, ptr %.capture_expr.16, align 4
  store i32 %37, ptr %.omp.ub36, align 4
  %38 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %X, float 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %A.addr, ptr null, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv34, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb35, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub36, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I41, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.14, i32 0, i32 1) ]
  %39 = load i32, ptr %.omp.lb35, align 4
  store i32 %39, ptr %.omp.iv34, align 4
  br label %omp.inner.for.cond37

omp.inner.for.cond37:                             ; preds = %omp.inner.for.inc50, %omp.precond.then33
  %40 = load i32, ptr %.omp.iv34, align 4
  %41 = load i32, ptr %.omp.ub36, align 4
  %add38 = add i32 %41, 1
  %cmp39 = icmp ult i32 %40, %add38
  br i1 %cmp39, label %omp.inner.for.body40, label %omp.inner.for.end52

omp.inner.for.body40:                             ; preds = %omp.inner.for.cond37
  %42 = load i32, ptr %.capture_expr.14, align 4
  %43 = load i32, ptr %.omp.iv34, align 4
  %mul42 = mul i32 %43, 1
  %add43 = add i32 %42, %mul42
  store i32 %add43, ptr %I41, align 4
  %44 = load ptr, ptr %A.addr, align 8
  %45 = load i32, ptr %I41, align 4
  %idxprom44 = sext i32 %45 to i64
  %arrayidx45 = getelementptr inbounds float, ptr %44, i64 %idxprom44
  %46 = load float, ptr %arrayidx45, align 4
  %47 = call fast float @llvm.fabs.f32(float %46)
  store float %47, ptr %X, align 4
  %48 = load float, ptr %X, align 4
  %49 = load ptr, ptr %A.addr, align 8
  %50 = load i32, ptr %I41, align 4
  %idxprom46 = sext i32 %50 to i64
  %arrayidx47 = getelementptr inbounds float, ptr %49, i64 %idxprom46
  %51 = load float, ptr %arrayidx47, align 4
  %add48 = fadd fast float %51, %48
  store float %add48, ptr %arrayidx47, align 4
  br label %omp.body.continue49

omp.body.continue49:                              ; preds = %omp.inner.for.body40
  br label %omp.inner.for.inc50

omp.inner.for.inc50:                              ; preds = %omp.body.continue49
  %52 = load i32, ptr %.omp.iv34, align 4
  %add51 = add nuw i32 %52, 1
  store i32 %add51, ptr %.omp.iv34, align 4
  br label %omp.inner.for.cond37

omp.inner.for.end52:                              ; preds = %omp.inner.for.cond37
  br label %omp.loop.exit53

omp.loop.exit53:                                  ; preds = %omp.inner.for.end52
  call void @llvm.directive.region.exit(token %38) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end54

omp.precond.end54:                                ; preds = %omp.loop.exit53, %omp.inner.for.body22
  br label %omp.body.continue55

omp.body.continue55:                              ; preds = %omp.precond.end54
  br label %omp.inner.for.inc56

omp.inner.for.inc56:                              ; preds = %omp.body.continue55
  %53 = load i32, ptr %.omp.iv17, align 4
  %add57 = add nsw i32 %53, 1
  store i32 %add57, ptr %.omp.iv17, align 4
  br label %omp.inner.for.cond20

omp.inner.for.end58:                              ; preds = %omp.inner.for.cond20
  br label %omp.loop.exit59

omp.loop.exit59:                                  ; preds = %omp.inner.for.end58
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %omp.precond.end60

omp.precond.end60:                                ; preds = %omp.loop.exit59, %omp.precond.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

declare float @llvm.fabs.f32(float)

; CHECK: define internal void [[NESTED_PRIVATE_LOOP2:@.+]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture [[ABASE:%.+]], i64 %{{.+}}, i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.+}}:
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]],
; CHECK:   [[AVAL:%.+]] = load float, ptr [[AADDR]]
; CHECK:   [[FABS:%.+]] = call fast float @llvm.fabs.f32(float [[AVAL]])
; CHECK:   [[FADD:%.+]] = fadd fast float [[FABS]], [[AVAL]]
; CHECK:   store float [[FADD]], ptr [[AADDR]]
; CHECK: }

; CHECK: define internal void [[NESTED_PRIVATE_LOOP1:@.+]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture writeonly [[ABASE:%.+]], ptr nocapture readonly [[BBASE:%.+]], i64 %{{.+}}, i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK: omp.inner.for.body{{.*}}:
; CHECK:   [[BADDR:%.+]] = getelementptr inbounds float, ptr [[BBASE]],
; CHECK:   [[BVAL:%.+]] = load float, ptr [[BADDR]]
; CHECK:   [[FMUL:%.+]] = fmul fast float [[BVAL]], 3.000000e+00
; CHECK:   [[AADDR:%.+]] = getelementptr inbounds float, ptr [[ABASE]],
; CHECK:   store float [[FMUL]], ptr [[AADDR]]
; CHECK: }

; CHECK: define internal void [[NESTED_PRIVATE_TEAMS]](ptr nocapture readonly %tid, ptr nocapture readnone %bid, ptr nocapture [[ABASE:%.+]], ptr nocapture readonly [[BBASE:%.+]], i64 %{{.+}}, i64 %{{.+}}, i64 %{{.+}}) #{{[0-9]+}} {
; CHECK:   call {{.*}} @__kmpc_fork_call({{.*}}, i32 5, ptr nonnull [[NESTED_PRIVATE_LOOP1]], ptr [[ABASE]], ptr [[BBASE]], i64 1077936128, i64 0, i64 %{{.+}})
; CHECK:   call {{.*}} @__kmpc_fork_call({{.*}}, i32 4, ptr nonnull [[NESTED_PRIVATE_LOOP2]], ptr [[ABASE]], i64 %{{.+}}, i64 0, i64 %{{.+}})
; CHECK: }

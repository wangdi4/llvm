; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; This test checks that alloca created for private variables for simd directive
; are inserted into the function entry block if there is no parent region that
; needs outlining or into the entry block of the outlined function otherwise.
;
; Original code:
; void foo()
; {
;   float a, b;
; #pragma omp simd private(a) lastprivate(b)
;   for (int i = 0; i < 10000; ++i) {
;     a = i;
;     b = i;
;   }
; }
;
; void bar()
; {
;   float a, b;
; #pragma omp parallel
; #pragma omp simd private(a) lastprivate(b)
;   for (int i = 0; i < 10000; ++i) {
;     a = i;
;     b = i;
;   }
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: void @foo
; CHECK:   [[BPRIV:%.+]] = alloca float, align 4
; CHECK:   [[APRIV:%.+]] = alloca float, align 4
; CHECK:   [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CHECK:   br i1 [[ZTT]], label %[[PHB:[^,]+]], label %{{.*}}
; CHECK: [[PHB]]:
; CHECK:   [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr [[APRIV]], float 0.000000e+00, i32 1), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[BPRIV]], float 0.000000e+00, i32 1)
; CHECK:   br label %[[LOOPBODY:[^,]+]]
; CHECK: [[LOOPBODY]]:
; CHECK:   store float %{{.*}}, ptr [[APRIV]]
; CHECK:   store float %{{.*}}, ptr [[BPRIV]]
; CHECK:   br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CHECK: [[LEXIT]]:
; CHECK:   call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK:   [[BVAL:%.+]] = load float, ptr [[BPRIV]]
; CHECK:   store float [[BVAL]], ptr %{{.+}}

define dso_local void @foo() {
entry:
  %a = alloca float, align 4
  %b = alloca float, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 9999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %a, float 0.000000e+00, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %b, float 0.000000e+00, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %4 = load i32, ptr %i, align 4
  %conv = sitofp i32 %4 to float
  store float %conv, ptr %a, align 4
  %5 = load i32, ptr %i, align 4
  %conv1 = sitofp i32 %5 to float
  store float %conv1, ptr %b, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %6, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

; CHECK-LABEL: void @bar
; CHECK: call void {{.*}} @__kmpc_fork_call(ptr {{.*}}, i32 2, ptr [[OUTLINED_BAR:@.+]], ptr %{{.*}}, ptr %{{.*}})
; CHECK: define internal void [[OUTLINED_BAR]](ptr %{{.+}}, ptr %{{.*}}, ptr [[A:%.*]], ptr [[B:%.*]]) #{{[0-9]+}} {
; CHECK:   [[BPRIV:%.+]] = alloca float, align 4
; CHECK:   [[APRIV:%.+]] = alloca float, align 4
; CHECK:   [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CHECK:   br i1 [[ZTT]], label %[[PHB:[^,]+]], label %{{.+}}
; CHECK: [[PHB]]:
; CHECK:   [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr [[APRIV]], float 0.000000e+00, i32 1), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr [[BPRIV]], float 0.000000e+00, i32 1)
; CHECK:   br label %[[LOOPBODY:[^,]+]]
; CHECK: [[LOOPBODY]]:
; CHECK:   store float %{{.*}}, ptr [[APRIV]]
; CHECK:   store float %{{.*}}, ptr [[BPRIV]]
; CHECK:   br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CHECK: [[LEXIT]]:
; CHECK:   call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK:   [[BVAL:%.+]] = load float, ptr [[BPRIV]]
; CHECK:   store float [[BVAL]], ptr [[B]]

define dso_local void @bar() {
entry:
  %a = alloca float, align 4
  %b = alloca float, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, float 0.000000e+00, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %b, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 9999, ptr %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %a, float 0.000000e+00, i32 1),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %b, float 0.000000e+00, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load i32, ptr %i, align 4
  %conv = sitofp i32 %5 to float
  store float %conv, ptr %a, align 4
  %6 = load i32, ptr %i, align 4
  %conv1 = sitofp i32 %6 to float
  store float %conv1, ptr %b, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %7, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

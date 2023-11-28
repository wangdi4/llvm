; RUN: opt -bugpoint-enable-legacy-pm -mattr=avx512f -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefixes=CHECK
; RUN: opt -mattr=avx512f -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK

; Original code:
; void foo(int *A, int *B, unsigned N) {
; #pragma omp simd aligned(A:32, B)
;   for (int I = 0; I < N; ++I) {
;     A[I] = I;
;     B[I] = I;
;   }
; }

; CHECK-LABEL: define {{[^@]+}}@foo
; CHECK-SAME:    (ptr [[PTR_A:%.+]], ptr [[PTR_B:%.+]], i32 %{{.+}})
;
; CHECK:         call void @llvm.assume(i1 true) [ "align"(ptr [[PTR_A]], i64 32) ]
; CHECK:         call void @llvm.assume(i1 true) [ "align"(ptr [[PTR_B]], i64 64) ]
;
; CHECK:         [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED"(ptr null, i32 32),
; CHECK-NEXT:    br label %[[LOOPBODY:[^,]+]]
; CHECK:       [[LOOPBODY]]:
; CHECK:         br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LOOPEXIT:[^,]+]]
; CHECK:       [[LOOPEXIT]]:
; CHECK:         call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr %A, ptr %B, i32 %N) {
entry:
  %A.addr = alloca ptr, align 8
  %B.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store ptr %A, ptr %A.addr, align 8
  store ptr %B, ptr %B.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  %0 = bitcast ptr %.capture_expr.0 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0)
  %1 = load i32, ptr %N.addr, align 4
  store i32 %1, ptr %.capture_expr.0, align 4
  %2 = bitcast ptr %.capture_expr.1 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %2)
  %3 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub i32 %3, 0
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %4 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp ult i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %5 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %5)
  %6 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %6)
  %7 = load i32, ptr %.capture_expr.1, align 4
  store i32 %7, ptr %.omp.ub, align 4 
  %8 = load ptr, ptr %A.addr, align 8
  %9 = load ptr, ptr %B.addr, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.ALIGNED"(ptr %8, i32 32),
    "QUAL.OMP.ALIGNED"(ptr %9, i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %I, i32 0, i32 1, i32 1) ]

  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %11 = load i32, ptr %.omp.iv, align 4
  %12 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp ule i32 %11, %12
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = bitcast ptr %I to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %13)
  %14 = load i32, ptr %.omp.iv, align 4
  %mul = mul i32 %14, 1
  %add4 = add i32 0, %mul
  store i32 %add4, ptr %I, align 4
  %15 = load i32, ptr %I, align 4
  %16 = load ptr, ptr %A.addr, align 8
  %17 = load i32, ptr %I, align 4
  %idxprom = sext i32 %17 to i64
  %ptridx = getelementptr inbounds i32, ptr %16, i64 %idxprom
  store i32 %15, ptr %ptridx, align 4
  %18 = load i32, ptr %I, align 4
  %19 = load ptr, ptr %B.addr, align 8
  %20 = load i32, ptr %I, align 4
  %idxprom5 = sext i32 %20 to i64
  %ptridx6 = getelementptr inbounds i32, ptr %19, i64 %idxprom5
  store i32 %18, ptr %ptridx6, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast ptr %I to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %21)
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, ptr %.omp.iv, align 4
  %add7 = add nuw i32 %22, 1
  store i32 %add7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.SIMD"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %23 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %23)
  %24 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %24)
  %25 = bitcast ptr %.capture_expr.1 to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %25)
  %26 = bitcast ptr %.capture_expr.0 to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %26)
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) 

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)


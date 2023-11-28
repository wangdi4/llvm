; Verify that OMP SIMD directives are removed by the clean-up pass or by vectorizer after executing VPO passes at all optimization levels.
; Input LLVM IR generated for below C program:

; float arr[1024];
;
; float  foo(int n1)
; {
;     int index;
;
; #pragma omp simd
;     for (index = 0; index < 1024; index++) {
;         if (arr[index] > 0) {
;             arr[index + n1] = index + n1 * n1 + 3;
;         }
;     }
;     return arr[0];
; }

; RUN: opt -O0 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O1 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O2 -paropt=0x7 -S %s | FileCheck %s
; RUN: opt -O3 -paropt=0x7 -S %s | FileCheck %s

; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-NOT: call void @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.SIMD"() ]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x float] zeroinitializer, align 16

define dso_local float @foo(i32 %n1) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0) ]

  %mul2 = mul nsw i32 %n1, %n1
  %add3 = add nuw i32 %mul2, 3
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %omp.inner.for.body.lr.ph ]
  %arrayidx = getelementptr inbounds [1024 x float], ptr @arr, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx, align 4
  %cmp1 = fcmp ogt float %1, 0.000000e+00
  br i1 %cmp1, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %2 = trunc i64 %indvars.iv to i32
  %add4 = add i32 %add3, %2
  %conv = sitofp i32 %add4 to float
  %add5 = add nsw i32 %2, %n1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [1024 x float], ptr @arr, i64 0, i64 %idxprom6
  store float %conv, ptr %arrayidx7, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  %3 = load float, ptr @arr, align 16
  ret float %3
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)


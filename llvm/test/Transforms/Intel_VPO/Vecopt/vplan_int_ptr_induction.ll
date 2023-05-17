; Test to verify that VPlan vectorizer handles integer induction with update in function call. C/C++ source:

; __declspec(noinline) void foo(int* a) { *a = *a + 3; }
; int main() {
;   int a = 0;
;   #pragma omp simd linear(a:3)
;   for(int k=0; k<N; k++) foo(&a);
;   return 0;
; }

; RUN: opt -passes=vplan-vec -vplan-force-vf=4 -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=IR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -vplan-print-after-vpentity-instrs -vplan-entities-dump -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=HIR

; Check that we vectorize loop with such induction
; IR: IntInduction(+) Start: i32 0 Step: i32 1 StartVal: i32 0 EndVal: i32 64
; IR: IntInduction(+) Start: i32 undef Step: i32 3
; HIR: IntInduction(+) Start: i32 0 Step: i32 1 StartVal: i32 0 EndVal: i32 63
; HIR: IntInduction(+) Start: i32 undef Step: i32 3

define dso_local void @_Z3fooPi(ptr nocapture noundef %a) local_unnamed_addr {
entry:
  %0 = load i32, ptr %a, align 4
  %add = add nsw i32 %0, 3
  store i32 %add, ptr %a, align 4
  ret void
}

define dso_local noundef i32 @main() local_unnamed_addr {
DIR.OMP.SIMD.1:
  %a.linear = alloca i32, align 4
  %k.linear.iv = alloca i32, align 4
  store i32 0, ptr %a.linear, align 4
  br label %DIR.OMP.SIMD.111

DIR.OMP.SIMD.111:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:TYPED"(ptr %a.linear, i32 0, i32 1, i32 3), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %k.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.111, %omp.inner.for.body
  %.omp.iv.local.09 = phi i32 [ 0, %DIR.OMP.SIMD.111 ], [ %add1, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %k.linear.iv)
  call void @_Z3fooPi(ptr noundef nonnull %a.linear) #1
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %k.linear.iv)
  %add1 = add nuw nsw i32 %.omp.iv.local.09, 1
  %exitcond.not = icmp eq i32 %add1, 64
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1.split

DIR.OMP.END.SIMD.1.split:                         ; preds = %DIR.OMP.END.SIMD.1
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1.split
  ret i32 0
}

declare void @llvm.lifetime.start.p0(i64 immarg %0, ptr nocapture %1)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
declare void @llvm.lifetime.end.p0(i64 immarg %0, ptr nocapture %1)

attributes #1 = { nounwind }



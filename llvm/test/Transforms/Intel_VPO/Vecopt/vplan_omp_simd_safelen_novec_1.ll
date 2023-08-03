; Check that vectorizer is bailed out and VPlans are not constructed when user specifies `#pragma omp simd safelen(1)`

; Input C code:
; int arr[1024];
;
; void  foo(int n1)
; {
;     int index;
;
; #pragma omp simd safelen(1)
;     for (index = 0; index < 1024; index++) {
;         if (arr[index] > 0) {
;             arr[index + n1] = index + n1 * n1 + 3;
;         }
;     }
; }


; REQUIRES: asserts

; RUN: opt -S -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=CHECK-HIR

; RUN: opt -S -passes="vplan-vec" -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s --check-prefix=CHECK-LLVM

; RUN: opt -passes=hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-optreport-emitter -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI-HIR

; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -intel-opt-report=medium < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTMED
; RUN: opt -passes=vplan-vec,intel-ir-optreport-emitter -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPTHI

; Check debug info from LoopVectorizationPlanner
; CHECK-HIR: LVP: Safelen: 1
; CHECK-HIR-NEXT: The forced vectorization factor or safelen specified by the user is 1.
; Check that loop is not vectorized
; CHECK-HIR: DO i1 = 0, 1023, 1

; CHECK-LLVM: LVP: Safelen: 1
; CHECK-LLVM-NEXT: The forced vectorization factor or safelen specified by the user is 1.
; Check that loop is not vectorized
; CHECK-LLVM-NOT: add <{{[0-9]+}} x i32>

; OPTRPTMED: remark #15436: loop was not vectorized: The forced vectorization factor or safelen specified by the user is 1.
; OPTRPTHI: remark #15436: loop was not vectorized: The forced vectorization factor or safelen specified by the user is 1.
; OPTRPTHI-HIR: remark #15436: loop was not vectorized: HIR: The forced vectorization factor or safelen specified by the user is 1.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 %n1) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SAFELEN"(i32 1), "QUAL.OMP.NORMALIZED.IV"(ptr null), "QUAL.OMP.NORMALIZED.UB"(ptr null) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                  ; preds = %omp.inner.for.body.lr.ph
  %mul2 = mul nsw i32 %n1, %n1
  %add3 = add nuw i32 %mul2, 3
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.1 ]
  %arrayidx = getelementptr inbounds [1024 x i32], ptr @arr, i64 0, i64 %indvars.iv, !intel-tbaa !2
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %1, 0
  br i1 %cmp1, label %if.then, label %omp.inner.for.inc

if.then:                                          ; preds = %omp.inner.for.body
  %2 = trunc i64 %indvars.iv to i32
  %add4 = add i32 %add3, %2
  %3 = trunc i64 %indvars.iv to i32
  %add5 = add nsw i32 %3, %n1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [1024 x i32], ptr @arr, i64 0, i64 %idxprom6, !intel-tbaa !2
  store i32 %add4, ptr %arrayidx7, align 4, !tbaa !2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then, %omp.inner.for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %DIR.OMP.END.SIMD.1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA1024_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}

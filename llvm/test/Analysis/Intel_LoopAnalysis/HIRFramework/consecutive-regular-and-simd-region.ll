; RUN: opt < %s -hir-ssa-deconstruction -print-after=hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print,print<hir>" 2>&1 | FileCheck %s

; There are two loops with header %for.cond.cleanup3 and
; %omp.inner.for.body in the incoming IR. The first one is a regular loop
; whereas the second one is a simd loop. The successor block %DIR.OMP.SIMD.1 of
; the first loop is the entry block of the second loop creating a cross-region
; jump.

; To avoid this jump, we split the entry bblock of the second loop.
; Note that both regions are empty because the empty loops were optimized away.

; CHECK: DIR.OMP.SIMD.1.split:

; CHECK:  BEGIN REGION { }

; CHECK:  END REGION

; CHECK:  BEGIN REGION { }
; CHECK:   %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%d8.linear.iv)[0])1) ]
; CHECK:   @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; CHECK: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local local_unnamed_addr global i32 0, align 4
@b = dso_local local_unnamed_addr global i32 0, align 4

define dso_local void @c() {
entry:
  %d8.linear.iv = alloca i32, align 4
  br label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond.cleanup3, %entry
  %cmp = phi i1 [ false, %for.cond.cleanup3 ], [ true, %entry ]
  br i1 %cmp, label %for.cond.cleanup3, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %for.cond.cleanup3
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %d8.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.129

DIR.OMP.SIMD.129:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.129, %omp.inner.for.body
  %cmp7.not = phi i1 [ true, %DIR.OMP.SIMD.129 ], [ false, %omp.inner.for.body ]
  br i1 %cmp7.not, label %omp.inner.for.body, label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.230

DIR.OMP.END.SIMD.230:                             ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }


; RUN: opt -hir-ssa-deconstruction -print-after=hir-ssa-deconstruction -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction" -print-after=hir-ssa-deconstruction -disable-output < %s 2>&1 | FileCheck %s

; Verify that the last entry intrinsic is picked up from the entry block when
; there are multiple intrinsics in the same bblock.

; Checks that region entry block is split at %entry2 intrinsic.

; CHECK: region.entry.split:
; CHECK-NEXT: %entry2 =

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* nocapture %a) {
entry:
  br label %region.entry

region.entry:
  %ld = load i32, i32* %a
  %entry0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  call void @llvm.directive.region.exit(token %entry0) [ "DIR.OMP.END.SIMD"() ]
  %entry1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  call void @llvm.directive.region.exit(token %entry1) [ "DIR.OMP.END.SIMD"() ]
  %entry2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 4
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %omp.inner.for.body.lr.ph
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %region.entry ]
  %t1 = sub nuw nsw i64 32, %indvars.iv
  %t2 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %t2, 20
  br i1 %cmp1, label %omp.inner.for.inc, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx2 = getelementptr inbounds i32, i32* %a, i64 %t1
  %t3 = trunc i64 %t1 to i32
  store i32 %t3, i32* %arrayidx2, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.inner.for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 31
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %entry2) [ "DIR.OMP.END.SIMD"() ]
  br label %end

end:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }



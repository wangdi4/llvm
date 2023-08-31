; Test to verify that imf-use-svml forces VPlan vectorizer to emit SVML call
; even when fast math flags are missing.

; RUN: opt -passes=vplan-vec -vector-library=SVML -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vector-library=SVML -disable-output < %s 2>&1 | FileCheck %s

; CHECK: @__svml_cos4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture noalias %A, i64 %n) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %loop ]
  %A.idx = getelementptr inbounds double, ptr %A, i64 %iv
  %A.ld = load double, ptr %A.idx, align 8
  %exp = call double @cos(double %A.ld) #0
  store double %exp, ptr %A.idx, align 8
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv, %n
  br i1 %exitcond, label %exit, label %loop

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @cos(double) local_unnamed_addr
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind "imf-use-svml"="true" }


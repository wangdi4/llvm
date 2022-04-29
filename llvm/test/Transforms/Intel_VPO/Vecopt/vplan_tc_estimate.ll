; Test to check estimated TC.
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -disable-output -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; RUN: opt -enable-new-pm=0 -vplan-vec -disable-output -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s
; RUN: opt -passes="vplan-vec" -disable-output -debug-only=LoopVectorizationPlanner < %s 2>&1 | FileCheck %s

; CHECK: Scalar Cost = 1073741823 x
;
define void @foo(i32* %lp, i32 %N) {
entry:
  %N2 = udiv i32 %N, 4
  br label %prehead

prehead:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),"QUAL.OMP.SIMDLEN"(i32 8) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.05 = phi i32 [ 0, %prehead], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %lp, i32 %l1.05
  store i32 %l1.05, i32* %arrayidx, align 8
  %inc = add nuw nsw i32 %l1.05, 1
  %exitcond.not = icmp ult i32 %inc, %N2
  br i1 %exitcond.not, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0)

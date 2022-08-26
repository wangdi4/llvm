; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


; Test for loop is not vectorized if there are non-recognized phis. Currently,
; the last private phi are not processed correctly.
;
define i64 @foo(i64* nocapture %larr) {
; CHECK: Function: foo
; CHECK:         BEGIN REGION { }
; CHECK-NOT:           + DO i1 = 0, 99, 4
; CHECK:         END REGION
;
entry:
%entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()]
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %else ]
  %priv_phi = phi i64 [ 0, %entry ], [ %merge, %else ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %l1.010
  %cmp = icmp eq i64 %l1.010, 100
  br i1 %cmp, label %then, label %else

then:
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, %l1.010
  store i64 %add, i64* %arrayidx, align 8
  br label %else

else:
  %merge = phi i64 [ %priv_phi, %for.body ], [ %0, %then ]
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret i64 %merge
}
; Function Attrs: nounwind
declare token @llvm.directive.region.entry()
; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)


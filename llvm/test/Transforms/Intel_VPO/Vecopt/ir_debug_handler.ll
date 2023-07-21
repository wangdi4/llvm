; REQUIRES: asserts
; RUN: opt %s -passes=vplan-vec -vplan-debug-error-handler -disable-output 2>&1 | FileCheck %s

; Test to check that VecErrorHandler is called on bailout.
; In this test we bailout on a multi-exit loop
;
; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

;CHECK: Bailout signaled on foo
;
define void @foo(double* nocapture %A, i32 %n) local_unnamed_addr {
entry:
  %cmp242 = icmp sgt i32 %n, 2
  br label %for.body.lr.ph2

for.body.lr.ph2:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv53 = phi i32 [ 0, %for.body.lr.ph2 ], [ %indvars.iv.next54, %for.cond ]
  br i1 %cmp242, label %for.end, label %for.cond

for.cond:
  %indvars.iv.next54 = add nuw nsw i32 %indvars.iv53, 1
  %exitcond = icmp eq i32 %indvars.iv.next54, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void
}

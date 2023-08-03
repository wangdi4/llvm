; RUN: opt < %s -passes=vplan-vec -S -vplan-force-vf=4 | FileCheck %s

; The test verifies that the compiler doesn't assert an internal error on
; trying to vectorize the direct call which is not to a Function but rather
; another type inherited from Constant.

; CHECK-COUNT-4: call ptr @bar()

define void @foo(ptr nocapture %a) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %body

body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %body ]
  %idx = getelementptr inbounds float, ptr %a, i64 %iv
  %ptr = call ptr @bar()
  %r = load float, ptr %ptr, align 4
  store float %r, ptr %idx, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 128
  br i1 %exitcond, label %exit, label %body

exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
declare ptr @bar()

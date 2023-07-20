; REQUIRES: asserts
; RUN: opt < %s -passes='print<hir-region-identification>' -debug-only=hir-region-identification 2>&1 | FileCheck %s

;; Only innermost multi-exit loops are supported at this point.

;; DO i1 = ...
;;     DO i2 = ...
;;          if () goto label_1
;;          DO i3 = ...
;;               if () goto label_2
;;          end do i3
;;          ...
;;          label_2:
;;          ...
;;      end do i2
;;       ...
;;      label_1:
;;       ...
;; end do i1

; CHECK: Loop %middle.body: Outer multi-exit loop throttled for compile time reasons.

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define void @foo(ptr %arr, i32 %N1, i32 %N2, i32 %N3) {
DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %outer.ph

outer.ph:
  br label %outer.body

outer.body:
  %outer.iv = phi i64 [ 0, %outer.ph ], [ %outer.iv.next, %outer.latch ]
  %gep = getelementptr inbounds i32, ptr %arr, i64 %outer.iv
  %ld = load i32, ptr %gep, align 4
  %cmp = icmp eq i32 %ld, %N1
  br i1 %cmp, label %middle.ph, label %outer.latch

middle.ph:
  br label %middle.body

middle.body:
  %middle.iv = phi i32 [ 0, %middle.ph ], [ %middle.iv.next, %middle.latch ]
  %cmp2 = icmp eq i32 %middle.iv, 5
  br i1 %cmp2, label %middle.early.exit, label %inner.ph

inner.ph:
  br label %inner.body

inner.body:
  %inner.iv = phi i32 [ 0, %inner.ph], [ %inner.iv.next, %inner.latch]
  %cmp3 = icmp eq i32 %inner.iv, 8
  br i1 %cmp3, label %inner.early.exit, label %inner.latch

inner.early.exit:
  br label %middle.latch

inner.latch:
  %inner.iv.next = add nuw nsw i32 %inner.iv, 1
  %exitcond2.not = icmp eq i32 %inner.iv.next, %N2
  br i1 %exitcond2.not, label %inner.exit, label %inner.body

inner.exit:
  br label %middle.latch

middle.latch:
  %middle.iv.next = add nuw nsw i32 %middle.iv, 1
  %exitcond.not = icmp eq i32 %middle.iv.next, %N3
  br i1 %exitcond.not, label %middle.exit, label %middle.body

middle.early.exit:
  br label %outer.latch

middle.exit:
  br label %outer.latch

outer.latch:
  %storemerge.lcssa = phi i32 [ 2, %middle.early.exit ], [ 2, %middle.exit ], [ 0, %outer.body ]
  %conv44 = sitofp i32 %storemerge.lcssa to float
  %outer.iv.next = add nuw nsw i64 %outer.iv, 1
  %exitcond35.not = icmp eq i64 %outer.iv.next, 100
  br i1 %exitcond35.not, label %outer.exit, label %outer.body

outer.exit:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

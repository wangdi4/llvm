; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region -disable-output 2>&1 < %s | FileCheck %s

; In this test case, we store the address of alloca %a in %p and perform load
; and store to it through %p in addition to direct store to %a.

; Verify that store to (%a)[0] is not incorrectly removed during 2nd invocation
; of the DSE pass after we replace store and load of (%p)[0] by %temp.
; This was happening because %temp is a dummy copy inst with undef operand
; which is introduced by HIR to act as a temp. AA recognized it as an undef
; value and concluded noalias between it and %a.

; Incoming HIR-

; CHECK: BEGIN REGION { }
; CHECK: @llvm.lifetime.start.p0(4,  &((%a)[0]));
; CHECK: (%a)[0] = 2;
; CHECK: (%p)[0] = &((%a)[0]);
; CHECK: %ld.ptr = (%p)[0];
; CHECK: %ld.val = (%ld.ptr)[0];
; CHECK: (%ld.ptr)[0] = %ld.val + 5;
; CHECK: (%p)[0] = null;
; CHECK: @llvm.lifetime.end.p0(4,  &((%a)[0]));
; CHECK: ret ;
; CHECK: END REGION

; After first DSE pass replacing load/store to %p by %temp-

; CHECK: BEGIN REGION { modified }
; CHECK: @llvm.lifetime.start.p0(4,  &((%a)[0]));
; CHECK: (%a)[0] = 2;
; CHECK: %temp = &((%a)[0]);
; CHECK: %ld.val = (%temp)[0];
; CHECK: (%temp)[0] = %ld.val + 5;
; CHECK: (%p)[0] = null;
; CHECK: @llvm.lifetime.end.p0(4,  &((%a)[0]));
; CHECK: ret ;
; CHECK: END REGION

; After second DSE pass, we should have the store to (%a)[0] so HIR should be
; unchanged-

; CHECK: BEGIN REGION { modified }
; CHECK: @llvm.lifetime.start.p0(4,  &((%a)[0]));
; CHECK: (%a)[0] = 2;
; CHECK: %temp = &((%a)[0]);
; CHECK: %ld.val = (%temp)[0];
; CHECK: (%temp)[0] = %ld.val + 5;
; CHECK: (%p)[0] = null;
; CHECK: @llvm.lifetime.end.p0(4,  &((%a)[0]));
; CHECK: ret ;

define void @foo(ptr %p) {
entry:
  %a = alloca i32
  br label %bb

bb:
  call void @llvm.lifetime.start.p0i8(i64 4, ptr nonnull %a)
  store i32 2, ptr %a, !tbaa !5
  store ptr %a, ptr %p, !tbaa !7
  %ld.ptr = load ptr, ptr %p,  !tbaa !7
  %ld.val = load i32, ptr %ld.ptr, !tbaa !5
  %add = add i32 %ld.val, 5
  store i32 %add, ptr %ld.ptr, !tbaa !5
  store ptr null, ptr %p, !tbaa !7
  call void @llvm.lifetime.end.p0i8(i64 4, ptr nonnull %a)
  ret void
}

declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #1 = { argmemonly nounwind }

!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"ptr", !3, i64 0}


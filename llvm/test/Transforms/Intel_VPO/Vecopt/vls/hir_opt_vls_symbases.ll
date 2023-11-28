; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -disable-output -hir-details < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%S1 = type { i64, double }

define void @foo(ptr %p) {
; HIR before VPlan:
; CHECK:      DO i64 i1 = 0, 127, 1   <DO_LOOP>
; CHECK:        %ld.i64 = (%p)[i1].0;
; CHECK-NEXT:   <LVAL-REG> NON-LINEAR i64 %ld.i64
; CHECK-NEXT:   <RVAL-REG> {al:8}(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds  !tbaa !1 [[SB_I64:{sb:.*}]]

; CHECK:        %ld.double = (%p)[i1].1;
; CHECK-NEXT:   <LVAL-REG> NON-LINEAR double %ld.double
; CHECK-NEXT:   <RVAL-REG> {al:8}(LINEAR ptr %p)[LINEAR i64 i1].1 inbounds  !tbaa !5 [[SB_DBL:{sb:.*}]]

; CHECK:        (%p)[i1].0 = i1;
; CHECK-NEXT:   <LVAL-REG> {al:8}(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds  !tbaa !1 [[SB_I64]]

; CHECK:        (%p)[i1].1 = %cast;
; CHECK-NEXT:   <LVAL-REG> {al:8}(LINEAR ptr %p)[LINEAR i64 i1].1 inbounds  !tbaa !5 [[SB_DBL]]
; CHECK:      END LOOP

; HIR after VPlan:
; CHECK:      DO i64 i1 = 0, 127, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK:        %.vls.load = (<8 x i64>*)(%p)[i1].0;
; CHECK-NEXT:   <LVAL-REG> NON-LINEAR <8 x i64> %.vls.load
; CHECK:        <RVAL-REG> {al:8}(<8 x i64>*)(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds !tbaa <{{.*}}> [[SB_I64]]
; CHECK-NEXT:      <BLOB> LINEAR ptr %p
; CHECK-NEXT:   <FAKE-RVAL-REG> {al:8}(<8 x i64>*)(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds !tbaa <{{.*}}> [[SB_DBL]]
; CHECK-NEXT:      <BLOB> LINEAR ptr %p

; CHECK:        (<8 x i64>*)(%p)[i1].0 = [[SHUFFLE:%shuffle[0-9]*]];
; CHECK-NEXT:   <LVAL-REG> {al:8}(<8 x i64>*)(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds !tbaa <{{.*}}> [[SB_I64]]
; CHECK-NEXT:      <BLOB> LINEAR ptr %p
; CHECK-NEXT:   <RVAL-REG> NON-LINEAR <8 x i64> [[SHUFFLE]]
; CHECK-NEXT:   <FAKE-LVAL-REG> {al:8}(<8 x i64>*)(LINEAR ptr %p)[LINEAR i64 i1].0 inbounds !tbaa <{{.*}}> [[SB_DBL]]
; CHECK-NEXT:      <BLOB> LINEAR ptr
; CHECK:      END LOOP
entry:
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]

  %p.i64 = getelementptr inbounds %S1, ptr %p, i64 %iv, i32 0
  %p.double = getelementptr inbounds %S1, ptr %p, i64 %iv, i32 1

  %ld.i64 = load i64, ptr %p.i64, !tbaa !4
  %ld.double = load double, ptr %p.double, !tbaa !5

  %cast = sitofp i64 %iv to double

  store i64 %iv, ptr %p.i64, !tbaa !4
  store double %cast, ptr %p.double, !tbaa !5

  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 128
  br i1 %exitcond, label %exit, label %header

exit:
  ret void
}

!0 = !{!"Simple C/C++ TBAA"}
!1 = !{!"omnipotent char", !0, i64 0}
!2 = !{!"long", !1, i64 0}
!3 = !{!"double", !1, i64 0}
!4 = !{!2, !2, i64 0}
!5 = !{!3, !3, i64 0}

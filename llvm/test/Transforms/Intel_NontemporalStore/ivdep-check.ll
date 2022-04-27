; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -aa-pipeline="default" -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check that we use "ivdep loop" metadata to prove that stores don't conflict.
define void @example(<8 x i64>* %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %index.jump = add i64 %index, 20000
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 1
  %splat2 = insertelement <8 x i64> zeroinitializer, i64 %index, i32 2
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  %jump = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.jump
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  store <8 x i64> %splat2, <8 x i64>* %jump, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 1000
  br i1 %cond, label %exit, label %loop, !llvm.loop !1

exit:
  ret void
}

; In this case we check that we still respect potential loop-independent
; conflicts. (For example, in this case j may be zero.)
define void @example2(<8 x i64>* %dest, i64 %j) "target-features"="+avx512f" {
; CHECK-LABEL: @example2(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %index.jump = add i64 %index, %j
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 1
  %splat2 = insertelement <8 x i64> zeroinitializer, i64 %index, i32 2
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  %jump = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.jump
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  store <8 x i64> %splat2, <8 x i64>* %jump, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 1000
  br i1 %cond, label %exit, label %loop, !llvm.loop !2

exit:
  ret void
}

!0 = !{i32 1}
!1 = distinct !{!1, !3}
!2 = distinct !{!2, !3}
!3 = !{!"llvm.loop.vectorize.ivdep_loop"}

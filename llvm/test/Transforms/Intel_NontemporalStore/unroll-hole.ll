; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; This test checks that blocks of stores with a sub-element hole are not
; converted.
define void @example(<8 x i64>* %dest) #0 {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next.3, %loop ]
  %index.next = add nuw nsw i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %index.next.1 = add nuw nsw i64 %index.next, 1
  %splat.1 = insertelement <8 x i64> zeroinitializer, i64 %index.next, i32 0
  %addr.1 = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.next
  store <8 x i64> %splat.1, <8 x i64>* %addr.1, align 16, !nontemporal !0
  %index.next.2 = add nuw nsw i64 %index.next.1, 1
  %index.next.3 = add nuw nsw i64 %index.next.2, 1
  %splat.3 = insertelement <8 x i64> zeroinitializer, i64 %index.next.2, i32 0
  %addr.3 = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.next.2
  store <8 x i64> %splat.3, <8 x i64>* %addr.3, align 16, !nontemporal !0
  %cond.3 = icmp eq i64 %index.next.3, 10000
  br i1 %cond.3, label %exit, label %loop, !llvm.loop !1

exit:                                             ; preds = %loop
  ret void
}

attributes #0 = { "target-features"="+avx512f" }

!0 = !{i32 1}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.unroll.disable"}

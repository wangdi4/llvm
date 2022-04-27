; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; This test checks that non-contiguous blocks of stores are not converted, even
; in sneaky cases where it's not obvious that the block is non-contiguous until
; the last store is reached.

define void @example(<8 x i64>* %dest) #0 {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:                                             ; preds = %loop, %entry
  %index = phi i64 [ 0, %entry ], [ %index.4, %loop ]
  %index.3 = add nuw nsw i64 %index, 3
  %splat.3 = insertelement <8 x i64> zeroinitializer, i64 %index.3, i32 0
  %addr.3 = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.3
  store <8 x i64> %splat.3, <8 x i64>* %addr.3, align 16, !nontemporal !0
  %index.5 = add nuw nsw i64 %index, 5
  %splat.5 = insertelement <8 x i64> zeroinitializer, i64 %index.5, i32 0
  %addr.5 = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.5
  store <8 x i64> %splat.5, <8 x i64>* %addr.5, align 16, !nontemporal !0
  %index.2 = add nuw nsw i64 %index, 2
  %splat.2 = insertelement <8 x i64> zeroinitializer, i64 %index.2, i32 0
  %addr.2 = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index.2
  store <8 x i64> %splat.2, <8 x i64>* %addr.2, align 16, !nontemporal !0
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %index.4 = add nuw nsw i64 %index, 4
  %cond = icmp eq i64 %index.4, 10000
  br i1 %cond, label %exit, label %loop, !llvm.loop !1

exit:                                             ; preds = %loop
  ret void
}

attributes #0 = { "target-features"="+avx512f" }

!0 = !{i32 1}
!1 = distinct !{!1, !2}
!2 = !{!"llvm.loop.unroll.disable"}

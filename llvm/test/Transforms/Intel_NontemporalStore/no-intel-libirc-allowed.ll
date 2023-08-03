; RUN: opt -enable-intel-advanced-opts -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; Check that the transform doesn't apply when -intel-libirc-allowed is not
; passed.

define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = !{i32 1}

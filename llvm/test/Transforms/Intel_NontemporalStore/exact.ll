; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

; Store-store collission
define void @example(ptr %dest) "target-features"="+avx512f" {
; CHECK-LABEL: @example(
; CHECK-NOT: call void @__libirc_nontemporal_store
entry:
  %dest2 = getelementptr inbounds <8 x i64>, ptr %dest, i64 10000
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  %addr2 = getelementptr inbounds <8 x i64>, ptr %dest2, i64 %index
  store volatile <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  store volatile <8 x i64> %splat, ptr %addr2, align 16, !nontemporal !0
  %cond = icmp ult i64 %index, 10000
  br i1 %cond, label %loop, label %exit

exit:
  ret void
}

!0 = !{i32 1}

; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: call void @__libirc_nontemporal_store

; The CFG in this test caused false negatives in our earlier checks for
; conditional stores; this test makes sure the store is correctly identified as
; conditional.

define void @example(ptr %dest) "target-features"="+avx512f" {
entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %latch ]
  %loop.cond = icmp eq i64 %index, 10000
  br i1 %loop.cond, label %exit, label %if

if:
  %if.cond = trunc i64 %index to i1
  br i1 %if.cond, label %then, label %latch

then:
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, ptr %dest, i64 %index
  store <8 x i64> %splat, ptr %addr, align 16, !nontemporal !0
  br label %latch

latch:
  %index.next = add i64 %index, 1
  br label %loop

exit:
  ret void
}

!0 = !{i32 1}

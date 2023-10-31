; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"

define void @test() "target-features"="+avx512f" {
; CHECK-LABEL: @test(
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %next, %loop ]

  %idx1 = add nsw i32 %index, -20000
  %ext1 = sext i32 %idx1 to i64
  %gep1 = getelementptr inbounds [26000000 x double], ptr undef, i64 0, i64 %ext1
  store <16 x double> zeroinitializer, ptr %gep1, align 8, !nontemporal !0

  %idx2 = add nuw i32 %index, 1320000
  %ext2 = sext i32 %idx2 to i64
  %gep2 = getelementptr inbounds [26000000 x double], ptr undef, i64 0, i64 %ext2
  store <16 x double> zeroinitializer, ptr %gep2, align 8, !nontemporal !0

  %next = add nuw nsw i32 %index, 16
  %cond = icmp ult i32 %index, 1339984
  br i1 %cond, label %loop, label %exit

exit:
  ret void
}

!0 = !{i32 1}

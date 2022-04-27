; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' < %s | FileCheck %s
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify < %s | FileCheck %s
target triple = "x86_64-unknown-linux-gnu"

define void @test() "target-features"="+avx512f" {
; CHECK-LABEL: @test(
; CHECK: call void @__libirc_nontemporal_store
entry:
  br label %loop

loop:
  %index = phi i32 [ 0, %entry ], [ %next, %loop ]

  %idx1 = add nsw i32 %index, -20000
  %ext1 = sext i32 %idx1 to i64
  %gep1 = getelementptr inbounds [26000000 x double], [26000000 x double]* undef, i64 0, i64 %ext1
  %cst1 = bitcast double* %gep1 to <16 x double>*
  store <16 x double> zeroinitializer, <16 x double>* %cst1, align 8, !nontemporal !0

  %idx2 = add nuw i32 %index, 1320000
  %ext2 = sext i32 %idx2 to i64
  %gep2 = getelementptr inbounds [26000000 x double], [26000000 x double]* undef, i64 0, i64 %ext2
  %cst2 = bitcast double* %gep2 to <16 x double>*
  store <16 x double> zeroinitializer, <16 x double>* %cst2, align 8, !nontemporal !0

  %next = add nuw nsw i32 %index, 16
  %cond = icmp ult i32 %index, 1339984
  br i1 %cond, label %loop, label %exit

exit:
  ret void
}

!0 = !{i32 1}

; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' -unaligned-nontemporal-buffer-size=1 < %s | FileCheck %s --check-prefixes=ALL,BS1
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify -unaligned-nontemporal-buffer-size=1 < %s | FileCheck %s --check-prefixes=ALL,BS1
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' -unaligned-nontemporal-buffer-size=512 < %s | FileCheck %s --check-prefixes=ALL,BS512
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify -unaligned-nontemporal-buffer-size=512 < %s | FileCheck %s --check-prefixes=ALL,BS512
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' -unaligned-nontemporal-buffer-size=543 < %s | FileCheck %s --check-prefixes=ALL,BS543
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify -unaligned-nontemporal-buffer-size=543 < %s | FileCheck %s --check-prefixes=ALL,BS543
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' -unaligned-nontemporal-buffer-size=544 < %s | FileCheck %s --check-prefixes=ALL,BS544
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify -unaligned-nontemporal-buffer-size=544 < %s | FileCheck %s --check-prefixes=ALL,BS544
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -passes 'unaligned-nontemporal,verify' -unaligned-nontemporal-buffer-size=545 < %s | FileCheck %s --check-prefixes=ALL,BS545
; RUN: opt -enable-intel-advanced-opts -intel-libirc-allowed -S -unaligned-nontemporal -verify -unaligned-nontemporal-buffer-size=545 < %s | FileCheck %s --check-prefixes=ALL,BS545
target triple = "x86_64-unknown-linux-gnu"

define void @example(<8 x i64>* %dest) "target-features"="+avx512f" {
; ALL-LABEL: @example(

; Ensure that the buffer has at least one element.
; BS1:   call void @__libirc_nontemporal_store(%__nontemporal_buffer_data* [[ADDR_NT_STORE_STRUCT:%.*]], i64 64, i32 0)

; Don't round if the buffer size is an exact multiple of the element size.
; BS512: call void @__libirc_nontemporal_store(%__nontemporal_buffer_data* [[ADDR_NT_STORE_STRUCT:%.*]], i64 512, i32 0)

; Round down if the buffer size is less than halfway between two multiples.
; BS543: call void @__libirc_nontemporal_store(%__nontemporal_buffer_data* [[ADDR_NT_STORE_STRUCT:%.*]], i64 512, i32 0)

; Also round down if the buffer size is exactly between two multiples.
; BS544: call void @__libirc_nontemporal_store(%__nontemporal_buffer_data* [[ADDR_NT_STORE_STRUCT:%.*]], i64 512, i32 0)

; Round up if the buffer size is more than halfway between two multiples.
; BS545: call void @__libirc_nontemporal_store(%__nontemporal_buffer_data* [[ADDR_NT_STORE_STRUCT:%.*]], i64 576, i32 0)

entry:
  br label %loop

loop:
  %index = phi i64 [ 0, %entry ], [ %index.next, %loop ]
  %index.next = add i64 %index, 1
  %splat = insertelement <8 x i64> zeroinitializer, i64 %index, i32 0
  %addr = getelementptr inbounds <8 x i64>, <8 x i64>* %dest, i64 %index
  store <8 x i64> %splat, <8 x i64>* %addr, align 16, !nontemporal !0
  %cond = icmp eq i64 %index, 10000
  br i1 %cond, label %exit, label %loop

exit:
  ret void
}

!0 = !{i32 1}

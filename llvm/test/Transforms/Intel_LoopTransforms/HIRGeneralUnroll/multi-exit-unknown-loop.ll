; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-general-unroll,print<hir>" < %s -xmain-opt-level=3 2>&1 | FileCheck %s

; Check that profitable multi-exit unknown loop is unrolled by 2.


; CHECK: Function

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   loop:
; CHECK: |   %t828.out = &((%t828)[0]);
; CHECK: |   %t827.out = &((%t827)[0]);
; CHECK: |   %t830 = (%t827)[0].1;
; CHECK: |   if (%t830 >= %t815)
; CHECK: |   {
; CHECK: |      goto earlyexit;
; CHECK: |   }
; CHECK: |   %t834 = (%t827)[0].0;
; CHECK: |   %t827 = &((%t834)[0]);
; CHECK: |   %t828 = &((%t827.out)[0]);
; CHECK: |   if (&((%t834)[0]) != null)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto loop;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: Function

; CHECK: + UNKNOWN LOOP i1
; CHECK: |   <i1 = 0>
; CHECK: |   loop:
; CHECK: |   %t828.out = &((%t828)[0]);
; CHECK: |   %t827.out = &((%t827)[0]);
; CHECK: |   %t830 = (%t827)[0].1;
; CHECK: |   if (%t830 >= %t815)
; CHECK: |   {
; CHECK: |      goto earlyexit;
; CHECK: |   }
; CHECK: |   %t834 = (%t827)[0].0;
; CHECK: |   %t827 = &((%t834)[0]);
; CHECK: |   %t828 = &((%t827.out)[0]);
; CHECK: |   if (&((%t834)[0]) == null)
; CHECK: |   {
; CHECK: |      goto loopexit[[SUFFIX:.*]];
; CHECK: |   }
; CHECK: |   %t828.out = &((%t828)[0]);
; CHECK: |   %t827.out = &((%t827)[0]);
; CHECK: |   %t830 = (%t827)[0].1;
; CHECK: |   if (%t830 >= %t815)
; CHECK: |   {
; CHECK: |      goto earlyexit;
; CHECK: |   }
; CHECK: |   %t834 = (%t827)[0].0;
; CHECK: |   %t827 = &((%t834)[0]);
; CHECK: |   %t828 = &((%t827.out)[0]);
; CHECK: |   if (&((%t834)[0]) != null)
; CHECK: |   {
; CHECK: |      <i1 = i1 + 1>
; CHECK: |      goto loop;
; CHECK: |   }
; CHECK: + END LOOP
; CHECK: loopexit[[SUFFIX]]:


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.mbuf = type { %struct.m_hdr, %union.anon }
%struct.m_hdr = type { %struct.mbuf*, %struct.mbuf*, i8*, i32, i16, i16 }
%union.anon = type { %struct.anon }
%struct.anon = type { %struct.pkthdr, %union.anon.7 }
%struct.pkthdr = type { %struct.ifnet*, i32, i8*, i32, i32, %struct.mbuf* }
%struct.ifnet = type { i8*, i8*, %struct.anon.0, i32, i16, i16, i16, i16, i32, i8*, i64, i32, i32 (%struct.ifnet*, %struct.mbuf*, %struct.sockaddr*, %struct.rtentry*)*, void (%struct.ifnet*)*, %union.anon.5, i32 (%struct.ifnet*, i64, i8*)*, void (%struct.ifnet*)*, %union.anon.6, i32 (%struct.ifnet*, i32*)*, void (%struct.ifnet*)*, void (%struct.ifnet*, %struct.mbuf*)*, void (i8*)*, i32 (%struct.ifnet*, %struct.sockaddr**, %struct.sockaddr*)*, %struct.ifqueue, %struct.ifqueue* }
%struct.anon.0 = type { %struct.ifnet*, %struct.ifnet** }
%struct.sockaddr = type { i8, i8, [14 x i8] }
%struct.rtentry = type { [2 x %struct.radix_node], %struct.sockaddr*, i64, i64, %struct.ifnet*, %struct.ifaddr*, %struct.sockaddr*, i8*, %struct.rt_metrics, %struct.rtentry*, %struct.rtentry*, i8* }
%struct.radix_node = type { %struct.radix_mask*, %struct.radix_node*, i16, i8, i8, %union.anon.2 }
%struct.radix_mask = type { i16, i8, i8, %struct.radix_mask*, %union.anon.1, i32 }
%union.anon.1 = type { i8* }
%union.anon.2 = type { %struct.anon.3 }
%struct.anon.3 = type { i8*, i8*, %struct.radix_node* }
%struct.ifaddr = type opaque
%struct.rt_metrics = type { i64, i64, i64, i64, i64, i64, i64, i64, i64, i64, [4 x i64] }
%union.anon.5 = type { i32 (%struct.ifnet*)* }
%union.anon.6 = type { i32 (%struct.ifnet*, i32*)* }
%struct.ifqueue = type { %struct.mbuf*, %struct.mbuf*, i32, i32, i32 }
%union.anon.7 = type { %struct.m_ext, [136 x i8] }
%struct.m_ext = type { i8*, void (i8*, i8*)*, i8*, i32, i32*, i32 }
%struct.route = type { %struct.rtentry*, %struct.sockaddr }
%struct.sockaddr_in = type { i8, i8, i16, %struct.in_addr, [8 x i8] }
%struct.in_addr = type { i32 }
%struct.ip_fw = type { %struct.anon.8, i64, i64, i64, %struct.in_addr, %struct.in_addr, %struct.in_addr, %struct.in_addr, i16, i8, i8, %union.anon.9, i32, i16, i16, i8, i8, i8, i8, i8, i8, i8, i8, i8, i8, i16, i64, i64, i64, %union.ip_fw_if, %union.ip_fw_if, %union.anon.11, i8*, i8*, i64, i64, i32, i64, i64, i8, i8, i16 }
%struct.anon.8 = type { %struct.ip_fw*, %struct.ip_fw** }
%union.anon.9 = type { [4 x i32], [4 x i8] }
%union.ip_fw_if = type { %struct.in_addr, [8 x i8] }
%union.anon.11 = type { %struct.sockaddr_in }
%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.TCDef*, %struct.version_number, %struct.version_number, %struct.version_number, i64, i32 (i64, i32, i8**)*, {}*, i32 (i32, i8**)*, void ()* }
%struct.version_number = type { i8, i8, i8, i8 }
%struct.dn_heap = type { i32, i32, i32, %struct.dn_heap_entry* }
%struct.dn_heap_entry = type { i32, i8* }
%struct.dn_pipe = type { %struct.dn_pipe*, i32, i32, i32, %struct.dn_pkt*, %struct.dn_pkt*, %struct.dn_heap, %struct.dn_heap, %struct.dn_heap, i32, i32, i32, i32, [16 x i8], %struct.ifnet*, i32, %struct.dn_flow_set }
%struct.dn_pkt = type { %struct.m_hdr, i32, %struct.ifnet*, %struct.sockaddr_in*, %struct.route, i32 }
%struct.dn_flow_set = type { %struct.dn_flow_set*, i16, i16, %struct.dn_pipe*, i16, i32, i32, i32, %struct.ipfw_flow_id, i32, i32, %struct.dn_flow_queue**, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32*, i32, i32, i32, i32, i32 }
%struct.ipfw_flow_id = type { i64, i64, i16, i16, i8, i8 }
%struct.dn_flow_queue = type { %struct.dn_flow_queue*, %struct.ipfw_flow_id, %struct.dn_pkt*, %struct.dn_pkt*, i32, i32, i64, i64, i64, i64, i32, i32, i32, i32, i64, %struct.dn_flow_set*, i32, i32, i32, i32 }


define void @foo(%struct.dn_pipe* %in, i32 %t815) {
entry:
  br label %loop

loop:                                    ; preds = %latch, %entry
  %t827 = phi %struct.dn_pipe* [ %t834, %latch ], [ %in, %entry ]
  %t828 = phi %struct.dn_pipe* [ %t827, %latch ], [ null, %entry ]
  %t829 = getelementptr inbounds %struct.dn_pipe, %struct.dn_pipe* %t827, i64 0, i32 1
  %t830 = load i32, i32* %t829
  %t831 = icmp slt i32 %t830, %t815
  br i1 %t831, label %latch, label %earlyexit

latch:                                    ; preds = %loop
  %t833 = getelementptr inbounds %struct.dn_pipe, %struct.dn_pipe* %t827, i64 0, i32 0
  %t834 = load %struct.dn_pipe*, %struct.dn_pipe** %t833
  %t835 = icmp eq %struct.dn_pipe* %t834, null
  br i1 %t835, label %exit, label %loop

earlyexit:                                    ; preds = %loop
  %t837 = phi %struct.dn_pipe* [ %t827, %loop ]
  %t838 = phi %struct.dn_pipe* [ %t828, %loop ]
  %t839 = phi i32 [ %t830, %loop ]
  ret void

exit:
  ret void
}


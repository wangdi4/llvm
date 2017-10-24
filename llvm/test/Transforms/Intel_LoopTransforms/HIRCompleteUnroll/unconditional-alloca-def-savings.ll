; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that this loop is deemed profitable. We should be able to deduce that the alloca loads (%heaps)[0][i1] can be optimized away after unrolling.
; CHECK: Dump Before HIR PostVec Complete Unroll

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   %2 = (%heaps)[0][i1];
; CHECK: |   %3 = (%2)[0].1;
; CHECK: |   if (%3 > 0)
; CHECK: |   {
; CHECK: |      %4 = (%2)[0].3;
; CHECK: |      %5 = (%4)[0].0;
; CHECK: |      %6 = (@curr_time)[0];
; CHECK: |      if (%6 >= %5)
; CHECK: |      {
; CHECK: |         %7 = (%4)[0].1;
; CHECK: |         @ip_heap_extract(&((%2)[0]),  null);
; CHECK: |         switch(i1)
; CHECK: |         {
; CHECK: |         case 0:
; CHECK: |            @ready_event(&((%struct.dn_flow_queue*)(%7)[0]));
; CHECK: |            break;
; CHECK: |         case 1:
; CHECK: |            %9 = (%7)[88];
; CHECK: |            if (%9 == 0)
; CHECK: |            {
; CHECK: |               @ready_event_wfq(&((%struct.dn_pipe*)(%7)[0]));
; CHECK: |            }
; CHECK: |            else
; CHECK: |            {
; CHECK: |               %call28 = @th_printf(&((@.str.2)[0][0]),  &((%7)[88]));
; CHECK: |            }
; CHECK: |            break;
; CHECK: |         default:
; CHECK: |            @transmit_event(&((%struct.dn_pipe*)(%7)[0]));
; CHECK: |            break;
; CHECK: |         }
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Dump After HIR PostVec Complete Unroll
; CHECK-NOT: DO i1

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.dn_pipe = type { %struct.dn_pipe*, i32, i32, i32, %struct.dn_pkt*, %struct.dn_pkt*, %struct.dn_heap, %struct.dn_heap, %struct.dn_heap, i32, i32, i32, i32, [16 x i8], %struct.ifnet*, i32, %struct.dn_flow_set }
%struct.dn_pkt = type { %struct.m_hdr, i32, %struct.ifnet*, %struct.sockaddr_in*, %struct.route, i32 }
%struct.m_hdr = type { %struct.mbuf*, %struct.mbuf*, i8*, i32, i16, i16 }
%struct.mbuf = type { %struct.m_hdr, %union.anon.5 }
%union.anon.5 = type { %struct.anon.6 }
%struct.anon.6 = type { %struct.pkthdr, %union.anon.7 }
%struct.pkthdr = type { %struct.ifnet*, i32, i8*, i32, i32, %struct.mbuf* }
%union.anon.7 = type { %struct.m_ext, [188 x i8] }
%struct.m_ext = type { i8*, void (i8*, i8*)*, i8*, i32, i32*, i32 }
%struct.sockaddr_in = type { i8, i8, i16, %struct.in_addr, [8 x i8] }
%struct.in_addr = type { i32 }
%struct.route = type { %struct.rtentry*, %struct.sockaddr }
%struct.rtentry = type { [2 x %struct.radix_node], %struct.sockaddr*, i32, i32, %struct.ifnet*, %struct.ifaddr*, %struct.sockaddr*, i8*, %struct.rt_metrics, %struct.rtentry*, %struct.rtentry*, i8* }
%struct.radix_node = type { %struct.radix_mask*, %struct.radix_node*, i16, i8, i8, %union.anon.0 }
%struct.radix_mask = type { i16, i8, i8, %struct.radix_mask*, %union.anon, i32 }
%union.anon = type { i8* }
%union.anon.0 = type { %struct.anon.1 }
%struct.anon.1 = type { i8*, i8*, %struct.radix_node* }
%struct.ifaddr = type opaque
%struct.rt_metrics = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i32] }
%struct.sockaddr = type { i8, i8, [14 x i8] }
%struct.dn_heap = type { i32, i32, i32, %struct.dn_heap_entry* }
%struct.dn_heap_entry = type { i32, i8* }
%struct.ifnet = type { i8*, i8*, %struct.anon, i32, i16, i16, i16, i16, i32, i8*, i32, i32, i32 (%struct.ifnet*, %struct.mbuf*, %struct.sockaddr*, %struct.rtentry*)*, void (%struct.ifnet*)*, %union.anon.3, i32 (%struct.ifnet*, i32, i8*)*, void (%struct.ifnet*)*, %union.anon.4, i32 (%struct.ifnet*, i32*)*, void (%struct.ifnet*)*, void (%struct.ifnet*, %struct.mbuf*)*, void (i8*)*, i32 (%struct.ifnet*, %struct.sockaddr**, %struct.sockaddr*)*, %struct.ifqueue, %struct.ifqueue* }
%struct.anon = type { %struct.ifnet*, %struct.ifnet** }
%union.anon.3 = type { i32 (%struct.ifnet*)* }
%union.anon.4 = type { i32 (%struct.ifnet*, i32*)* }
%struct.ifqueue = type { %struct.mbuf*, %struct.mbuf*, i32, i32, i32 }
%struct.dn_flow_set = type { %struct.dn_flow_set*, i16, i16, %struct.dn_pipe*, i16, i32, i32, i32, %struct.ipfw_flow_id, i32, i32, %struct.dn_flow_queue**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32*, i32, i32, i32, i32, i32 }
%struct.ipfw_flow_id = type { i32, i32, i16, i16, i8, i8 }
%struct.dn_flow_queue = type { %struct.dn_flow_queue*, %struct.ipfw_flow_id, %struct.dn_pkt*, %struct.dn_pkt*, i32, i32, i32, i64, i64, i32, i32, i32, i32, i32, i32, %struct.dn_flow_set*, i32, i32, i32, i32 }

@curr_time = external local_unnamed_addr global i32, align 4
@ready_heap = external global %struct.dn_heap, align 4
@wfq_ready_heap = external global %struct.dn_heap, align 4
@extract_heap = external global %struct.dn_heap, align 4
@.str.2 = external hidden unnamed_addr constant [37 x i8], align 1

declare i32 @th_printf(i8*, ...)

; Function Attrs: nounwind
define void @dummynet(i8* nocapture readnone %unused) {
entry:
  %heaps = alloca [3 x %struct.dn_heap*], align 4
  %0 = bitcast [3 x %struct.dn_heap*]* %heaps to i8*
  %arrayidx = getelementptr inbounds [3 x %struct.dn_heap*], [3 x %struct.dn_heap*]* %heaps, i32 0, i32 0
  store %struct.dn_heap* @ready_heap, %struct.dn_heap** %arrayidx, align 4, !tbaa !3
  %arrayidx1 = getelementptr inbounds [3 x %struct.dn_heap*], [3 x %struct.dn_heap*]* %heaps, i32 0, i32 1
  store %struct.dn_heap* @wfq_ready_heap, %struct.dn_heap** %arrayidx1, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds [3 x %struct.dn_heap*], [3 x %struct.dn_heap*]* %heaps, i32 0, i32 2
  store %struct.dn_heap* @extract_heap, %struct.dn_heap** %arrayidx2, align 4, !tbaa !3
  %1 = load i32, i32* @curr_time, align 4, !tbaa !7
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* @curr_time, align 4, !tbaa !7
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %i.091 = phi i32 [ 0, %entry ], [ %inc35, %for.inc ]
  %arrayidx3 = getelementptr inbounds [3 x %struct.dn_heap*], [3 x %struct.dn_heap*]* %heaps, i32 0, i32 %i.091
  %2 = load %struct.dn_heap*, %struct.dn_heap** %arrayidx3, align 4, !tbaa !3
  %elements = getelementptr inbounds %struct.dn_heap, %struct.dn_heap* %2, i32 0, i32 1
  %3 = load i32, i32* %elements, align 4, !tbaa !9
  %cmp4 = icmp sgt i32 %3, 0
  br i1 %cmp4, label %land.lhs.true, label %for.inc

land.lhs.true:                                    ; preds = %for.body
  %p5 = getelementptr inbounds %struct.dn_heap, %struct.dn_heap* %2, i32 0, i32 3
  %4 = load %struct.dn_heap_entry*, %struct.dn_heap_entry** %p5, align 4, !tbaa !11
  %key = getelementptr inbounds %struct.dn_heap_entry, %struct.dn_heap_entry* %4, i32 0, i32 0
  %5 = load i32, i32* %key, align 4, !tbaa !12
  %6 = load i32, i32* @curr_time, align 4, !tbaa !7
  %cmp7 = icmp slt i32 %6, %5
  br i1 %cmp7, label %for.inc, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %object = getelementptr inbounds %struct.dn_heap_entry, %struct.dn_heap_entry* %4, i32 0, i32 1
  %7 = load i8*, i8** %object, align 4, !tbaa !15
  tail call void @ip_heap_extract(%struct.dn_heap* nonnull %2, i8* null) #3
  switch i32 %i.091, label %if.else31 [
    i32 0, label %if.then20
    i32 1, label %if.then22
  ]

if.then20:                                        ; preds = %if.then
  %8 = bitcast i8* %7 to %struct.dn_flow_queue*
  tail call void @ready_event(%struct.dn_flow_queue* %8) #3
  br label %for.inc

if.then22:                                        ; preds = %if.then
  %if_name = getelementptr inbounds i8, i8* %7, i32 88
  %9 = load i8, i8* %if_name, align 4, !tbaa !16
  %cmp24 = icmp eq i8 %9, 0
  br i1 %cmp24, label %if.else29, label %if.then26

if.then26:                                        ; preds = %if.then22
  %call28 = tail call i32 (i8*, ...) @th_printf(i8* getelementptr inbounds ([37 x i8], [37 x i8]* @.str.2, i32 0, i32 0), i8* %if_name) #3
  br label %for.inc

if.else29:                                        ; preds = %if.then22
  %10 = bitcast i8* %7 to %struct.dn_pipe*
  tail call void @ready_event_wfq(%struct.dn_pipe* %10)
  br label %for.inc

if.else31:                                        ; preds = %if.then
  %11 = bitcast i8* %7 to %struct.dn_pipe*
  tail call void @transmit_event(%struct.dn_pipe* %11) #3
  br label %for.inc

for.inc:                                          ; preds = %if.else31, %if.else29, %if.then26, %if.then20, %land.lhs.true, %for.body
  %inc35 = add nuw nsw i32 %i.091, 1
  %exitcond = icmp eq i32 %inc35, 3
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

declare void @ip_heap_extract(%struct.dn_heap*, i8*)

declare void @ready_event(%struct.dn_flow_queue*)

; Function Attrs: nounwind
declare void @ready_event_wfq(%struct.dn_pipe*)

declare void @transmit_event(%struct.dn_pipe*)

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 5.0.0 (cfe/trunk)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"unspecified pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
!9 = !{!10, !8, i64 4}
!10 = !{!"struct@dn_heap", !8, i64 0, !8, i64 4, !8, i64 8, !4, i64 12}
!11 = !{!10, !4, i64 12}
!12 = !{!13, !8, i64 0}
!13 = !{!"struct@dn_heap_entry", !8, i64 0, !14, i64 4}
!14 = !{!"pointer@_ZTSPv", !5, i64 0}
!15 = !{!13, !14, i64 4}
!16 = !{!17, !5, i64 88}
!17 = !{!"struct@dn_pipe", !4, i64 0, !8, i64 4, !8, i64 8, !8, i64 12, !4, i64 16, !4, i64 20, !10, i64 24, !10, i64 40, !10, i64 56, !8, i64 72, !8, i64 76, !8, i64 80, !8, i64 84, !18, i64 88, !4, i64 104, !8, i64 108, !19, i64 112}
!18 = !{!"array@_ZTSA16_c", !5, i64 0}
!19 = !{!"struct@dn_flow_set", !4, i64 0, !20, i64 4, !20, i64 6, !4, i64 8, !20, i64 12, !8, i64 16, !8, i64 20, !8, i64 24, !21, i64 28, !8, i64 44, !8, i64 48, !4, i64 52, !22, i64 56, !8, i64 60, !8, i64 64, !8, i64 68, !8, i64 72, !8, i64 76, !8, i64 80, !8, i64 84, !8, i64 88, !8, i64 92, !23, i64 96, !8, i64 100, !8, i64 104, !8, i64 108, !8, i64 112, !8, i64 116}
!20 = !{!"short", !5, i64 0}
!21 = !{!"struct@ipfw_flow_id", !22, i64 0, !22, i64 4, !20, i64 8, !20, i64 10, !5, i64 12, !5, i64 13}
!22 = !{!"long", !5, i64 0}
!23 = !{!"pointer@_ZTSPj", !5, i64 0}
!24 = !{!17, !8, i64 60}
!25 = !{!17, !4, i64 68}
!26 = !{!17, !8, i64 72}
!27 = !{!28, !8, i64 96}
!28 = !{!"struct@dn_flow_queue", !4, i64 0, !21, i64 4, !4, i64 20, !4, i64 24, !8, i64 28, !8, i64 32, !22, i64 36, !29, i64 40, !29, i64 48, !22, i64 56, !8, i64 60, !8, i64 64, !8, i64 68, !8, i64 72, !22, i64 76, !4, i64 80, !8, i64 84, !8, i64 88, !8, i64 92, !8, i64 96}
!29 = !{!"long long", !5, i64 0}
!30 = !{!28, !8, i64 92}
!31 = !{!28, !4, i64 80}
!32 = !{!19, !8, i64 16}
!33 = !{!17, !8, i64 76}

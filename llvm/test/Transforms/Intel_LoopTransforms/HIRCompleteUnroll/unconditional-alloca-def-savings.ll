; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that this loop is deemed profitable. We should be able to deduce that the alloca loads (%heaps)[0][i1] can be optimized away after unrolling.
; CHECK: Function

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   %1 = (%heaps)[0][i1];
; CHECK: |   %2 = (%1)[0].1;
; CHECK: |   if (%2 > 0)
; CHECK: |   {
; CHECK: |      %3 = (%1)[0].3;
; CHECK: |      %4 = (%3)[0];
; CHECK: |      %5 = (@curr_time)[0];
; CHECK: |      if (%5 >= %4)
; CHECK: |      {
; CHECK: |         %6 = (%3)[0].1;
; CHECK: |         @ip_heap_extract(&((%1)[0]),  null);
; CHECK: |         switch(i1)
; CHECK: |         {
; CHECK: |         case 0:
; CHECK: |            @ready_event(&((%6)[0]));
; CHECK: |            break;
; CHECK: |         case 1:
; CHECK: |            %7 = (%6)[88];
; CHECK: |            if (%7 == 0)
; CHECK: |            {
; CHECK: |               @ready_event_wfq(&((%6)[0]));
; CHECK: |            }
; CHECK: |            else
; CHECK: |            {
; CHECK: |               %call28 = @th_printf(&((@.str.2)[0]),  &((%6)[88]));
; CHECK: |            }
; CHECK: |            break;
; CHECK: |         default:
; CHECK: |            @transmit_event(&((%6)[0]));
; CHECK: |            break;
; CHECK: |         }
; CHECK: |      }
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: Function
; CHECK-NOT: DO i1

target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%struct.dn_pipe = type { ptr, i32, i32, i32, ptr, ptr, %struct.dn_heap, %struct.dn_heap, %struct.dn_heap, i32, i32, i32, i32, [16 x i8], ptr, i32, %struct.dn_flow_set }
%struct.dn_pkt = type { %struct.m_hdr, i32, ptr, ptr, %struct.route, i32 }
%struct.m_hdr = type { ptr, ptr, ptr, i32, i16, i16 }
%struct.mbuf = type { %struct.m_hdr, %union.anon.5 }
%union.anon.5 = type { %struct.anon.6 }
%struct.anon.6 = type { %struct.pkthdr, %union.anon.7 }
%struct.pkthdr = type { ptr, i32, ptr, i32, i32, ptr }
%union.anon.7 = type { %struct.m_ext, [188 x i8] }
%struct.m_ext = type { ptr, ptr, ptr, i32, ptr, i32 }
%struct.sockaddr_in = type { i8, i8, i16, %struct.in_addr, [8 x i8] }
%struct.in_addr = type { i32 }
%struct.route = type { ptr, %struct.sockaddr }
%struct.rtentry = type { [2 x %struct.radix_node], ptr, i32, i32, ptr, ptr, ptr, ptr, %struct.rt_metrics, ptr, ptr, ptr }
%struct.radix_node = type { ptr, ptr, i16, i8, i8, %union.anon.0 }
%struct.radix_mask = type { i16, i8, i8, ptr, %union.anon, i32 }
%union.anon = type { ptr }
%union.anon.0 = type { %struct.anon.1 }
%struct.anon.1 = type { ptr, ptr, ptr }
%struct.ifaddr = type opaque
%struct.rt_metrics = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i32] }
%struct.sockaddr = type { i8, i8, [14 x i8] }
%struct.dn_heap = type { i32, i32, i32, ptr }
%struct.dn_heap_entry = type { i32, ptr }
%struct.ifnet = type { ptr, ptr, %struct.anon, i32, i16, i16, i16, i16, i32, ptr, i32, i32, ptr, ptr, %union.anon.3, ptr, ptr, %union.anon.4, ptr, ptr, ptr, ptr, ptr, %struct.ifqueue, ptr }
%struct.anon = type { ptr, ptr }
%union.anon.3 = type { ptr }
%union.anon.4 = type { ptr }
%struct.ifqueue = type { ptr, ptr, i32, i32, i32 }
%struct.dn_flow_set = type { ptr, i16, i16, ptr, i16, i32, i32, i32, %struct.ipfw_flow_id, i32, i32, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32, i32 }
%struct.ipfw_flow_id = type { i32, i32, i16, i16, i8, i8 }
%struct.dn_flow_queue = type { ptr, %struct.ipfw_flow_id, ptr, ptr, i32, i32, i32, i64, i64, i32, i32, i32, i32, i32, i32, ptr, i32, i32, i32, i32 }

@curr_time = external local_unnamed_addr global i32, align 4
@ready_heap = external global %struct.dn_heap, align 4
@wfq_ready_heap = external global %struct.dn_heap, align 4
@extract_heap = external global %struct.dn_heap, align 4
@.str.2 = external hidden unnamed_addr constant [37 x i8], align 1

declare i32 @th_printf(ptr, ...)

; Function Attrs: nounwind
define void @dummynet(ptr nocapture readnone %unused) {
entry:
  %heaps = alloca [3 x ptr], align 4
  store ptr @ready_heap, ptr %heaps, align 4, !tbaa !3
  %arrayidx1 = getelementptr inbounds [3 x ptr], ptr %heaps, i32 0, i32 1
  store ptr @wfq_ready_heap, ptr %arrayidx1, align 4, !tbaa !3
  %arrayidx2 = getelementptr inbounds [3 x ptr], ptr %heaps, i32 0, i32 2
  store ptr @extract_heap, ptr %arrayidx2, align 4, !tbaa !3
  %0 = load i32, ptr @curr_time, align 4, !tbaa !7
  %inc = add nsw i32 %0, 1
  store i32 %inc, ptr @curr_time, align 4, !tbaa !7
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %i.091 = phi i32 [ 0, %entry ], [ %inc35, %for.inc ]
  %arrayidx3 = getelementptr inbounds [3 x ptr], ptr %heaps, i32 0, i32 %i.091
  %1 = load ptr, ptr %arrayidx3, align 4, !tbaa !3
  %elements = getelementptr inbounds %struct.dn_heap, ptr %1, i32 0, i32 1
  %2 = load i32, ptr %elements, align 4, !tbaa !9
  %cmp4 = icmp sgt i32 %2, 0
  br i1 %cmp4, label %land.lhs.true, label %for.inc

land.lhs.true:                                    ; preds = %for.body
  %p5 = getelementptr inbounds %struct.dn_heap, ptr %1, i32 0, i32 3
  %3 = load ptr, ptr %p5, align 4, !tbaa !11
  %4 = load i32, ptr %3, align 4, !tbaa !12
  %5 = load i32, ptr @curr_time, align 4, !tbaa !7
  %cmp7 = icmp slt i32 %5, %4
  br i1 %cmp7, label %for.inc, label %if.then

if.then:                                          ; preds = %land.lhs.true
  %object = getelementptr inbounds %struct.dn_heap_entry, ptr %3, i32 0, i32 1
  %6 = load ptr, ptr %object, align 4, !tbaa !15
  tail call void @ip_heap_extract(ptr nonnull %1, ptr null) #3
  switch i32 %i.091, label %if.else31 [
    i32 0, label %if.then20
    i32 1, label %if.then22
  ]

if.then20:                                        ; preds = %if.then
  tail call void @ready_event(ptr %6) #3
  br label %for.inc

if.then22:                                        ; preds = %if.then
  %if_name = getelementptr inbounds i8, ptr %6, i32 88
  %7 = load i8, ptr %if_name, align 4, !tbaa !16
  %cmp24 = icmp eq i8 %7, 0
  br i1 %cmp24, label %if.else29, label %if.then26

if.then26:                                        ; preds = %if.then22
  %call28 = tail call i32 (ptr, ...) @th_printf(ptr @.str.2, ptr %if_name) #3
  br label %for.inc

if.else29:                                        ; preds = %if.then22
  tail call void @ready_event_wfq(ptr %6)
  br label %for.inc

if.else31:                                        ; preds = %if.then
  tail call void @transmit_event(ptr %6) #3
  br label %for.inc

for.inc:                                          ; preds = %if.else31, %if.else29, %if.then26, %if.then20, %land.lhs.true, %for.body
  %inc35 = add nuw nsw i32 %i.091, 1
  %exitcond = icmp eq i32 %inc35, 3
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

declare void @ip_heap_extract(ptr, ptr)

declare void @ready_event(ptr)

; Function Attrs: nounwind
declare void @ready_event_wfq(ptr)

declare void @transmit_event(ptr)

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

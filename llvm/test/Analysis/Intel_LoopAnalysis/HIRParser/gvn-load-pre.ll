; Verify that with GVN the if condition is parsed in terms of IV. This is due to GVN hoisting the initial %1 load outside the loop so that %inc114 becomes linear.

; RUN: opt < %s -tbaa -gvn -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck -check-prefix=GVN %s

; GVN: |   if (i1 + %.pre + 1 <=u %sub71)

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; CHECK: |   if (%1 + 1 <=u %sub71)


target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"

%struct.internal_state = type { %struct.z_stream_s*, i32, i8*, i32, i8*, i32, i32, %struct.gz_header_s*, i32, i8, i32, i32, i32, i32, i8*, i32, i16*, i16*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [573 x %struct.ct_data_s], [61 x %struct.ct_data_s], [39 x %struct.ct_data_s], %struct.tree_desc_s, %struct.tree_desc_s, %struct.tree_desc_s, [16 x i16], [573 x i32], i32, i32, [573 x i8], i8*, i32, i32, i16*, i32, i32, i32, i32, i16, i32, i32 }
%struct.z_stream_s = type { i8*, i32, i32, i8*, i32, i32, i8*, %struct.internal_state*, i8* (i8*, i32, i32)*, void (i8*, i8*)*, i8*, i32, i32, i32 }
%struct.gz_header_s = type { i32, i32, i32, i32, i8*, i32, i32, i8*, i32, i8*, i32, i32, i32 }
%struct.ct_data_s = type { %union.anon, %union.anon.0 }
%union.anon = type { i16 }
%union.anon.0 = type { i16 }
%struct.tree_desc_s = type { %struct.ct_data_s*, i32, %struct.static_tree_desc_s* }
%struct.static_tree_desc_s = type { i32 }


define i32 @foo(%struct.internal_state* %s, i32 %sub112, i32 %sub71) {
entry:
  %ins_h = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 18
  %hash_shift = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 22
  %window = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 14
  %strstart = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 27
  %hash_mask = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 21
  %head = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 17
  %prev = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 16
  %w_mask = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 13
  %prev_length = getelementptr inbounds %struct.internal_state, %struct.internal_state* %s, i32 0, i32 30
  br label %do.body

do.body:                                          ; preds = %do.cond, %entry
  %0 = phi i32 [ %dec146, %do.cond ], [ %sub112, %entry ]
  %1 = load i32, i32* %strstart, align 4, !tbaa !68
  %inc114 = add i32 %1, 1
  store i32 %inc114, i32* %strstart, align 4, !tbaa !68
  %cmp115 = icmp ugt i32 %inc114, %sub71
  br i1 %cmp115, label %do.cond, label %if.then117

if.then117:                                       ; preds = %do.body
  %2 = load i32, i32* %ins_h, align 4, !tbaa !75
  %3 = load i32, i32* %hash_shift, align 4, !tbaa !34
  %4 = and i32 %3, 31
  %shl120 = shl i32 %2, %4
  %5 = load i8*, i8** %window, align 4, !tbaa !35
  %add123 = add i32 %1, 3
  %arrayidx124 = getelementptr inbounds i8, i8* %5, i32 %add123
  %6 = load i8, i8* %arrayidx124, align 1, !tbaa !1
  %conv125 = zext i8 %6 to i32
  %xor126 = xor i32 %conv125, %shl120
  %7 = load i32, i32* %hash_mask, align 4, !tbaa !33
  %and128 = and i32 %xor126, %7
  store i32 %and128, i32* %ins_h, align 4, !tbaa !75
  %8 = load i16*, i16** %head, align 4, !tbaa !37
  %arrayidx132 = getelementptr inbounds i16, i16* %8, i32 %and128
  %9 = load i16, i16* %arrayidx132, align 2, !tbaa !58
  %10 = load i16*, i16** %prev, align 4, !tbaa !36
  %11 = load i32, i32* %w_mask, align 4, !tbaa !30
  %and136 = and i32 %11, %inc114
  %arrayidx137 = getelementptr inbounds i16, i16* %10, i32 %and136
  store i16 %9, i16* %arrayidx137, align 2, !tbaa !58
  %conv140 = trunc i32 %inc114 to i16
  store i16 %conv140, i16* %arrayidx132, align 2, !tbaa !58
  br label %do.cond

do.cond:                                          ; preds = %do.body, %if.then117
  %dec146 = add i32 %0, -1
  store i32 %dec146, i32* %prev_length, align 4
  %cmp147 = icmp eq i32 %dec146, 0
  br i1 %cmp147, label %do.end, label %do.body

do.end:                                           ; preds = %do.cond
  ret i32 %1
}


!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 21010) (llvm/branches/loopopt 21016)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!6 = !{!"pointer@_ZTSPh", !2, i64 0}
!7 = !{!"int", !2, i64 0}
!8 = !{!"long", !2, i64 0}
!9 = !{!"pointer@_ZTSPc", !2, i64 0}
!10 = !{!"unspecified pointer", !2, i64 0}
!11 = !{!"pointer@_ZTSPFPvS_jjE", !2, i64 0}
!12 = !{!"pointer@_ZTSPFvPvS_E", !2, i64 0}
!13 = !{!"pointer@_ZTSPv", !2, i64 0}
!19 = !{!"struct@internal_state", !10, i64 0, !7, i64 4, !6, i64 8, !8, i64 12, !6, i64 16, !7, i64 20, !7, i64 24, !10, i64 28, !7, i64 32, !2, i64 36, !7, i64 40, !7, i64 44, !7, i64 48, !7, i64 52, !6, i64 56, !8, i64 60, !20, i64 64, !20, i64 68, !7, i64 72, !7, i64 76, !7, i64 80, !7, i64 84, !7, i64 88, !8, i64 92, !7, i64 96, !7, i64 100, !7, i64 104, !7, i64 108, !7, i64 112, !7, i64 116, !7, i64 120, !7, i64 124, !7, i64 128, !7, i64 132, !7, i64 136, !7, i64 140, !7, i64 144, !2, i64 148, !2, i64 2440, !2, i64 2684, !21, i64 2840, !21, i64 2852, !21, i64 2864, !22, i64 2876, !24, i64 2908, !7, i64 5200, !7, i64 5204, !25, i64 5208, !6, i64 5784, !7, i64 5788, !7, i64 5792, !20, i64 5796, !8, i64 5800, !8, i64 5804, !7, i64 5808, !7, i64 5812, !23, i64 5816, !7, i64 5820, !8, i64 5824}
!20 = !{!"pointer@_ZTSPt", !2, i64 0}
!21 = !{!"struct@tree_desc_s", !10, i64 0, !7, i64 4, !10, i64 8}
!22 = !{!"array@_ZTSA16_t", !23, i64 0}
!23 = !{!"short", !2, i64 0}
!24 = !{!"array@_ZTSA573_i", !7, i64 0}
!25 = !{!"array@_ZTSA573_h", !2, i64 0}
!30 = !{!19, !7, i64 52}
!33 = !{!19, !7, i64 84}
!34 = !{!19, !7, i64 88}
!35 = !{!19, !6, i64 56}
!36 = !{!19, !20, i64 64}
!37 = !{!19, !20, i64 68}
!45 = !{!19, !20, i64 5796}
!58 = !{!23, !23, i64 0}
!68 = !{!19, !7, i64 108}
!75 = !{!19, !7, i64 72}

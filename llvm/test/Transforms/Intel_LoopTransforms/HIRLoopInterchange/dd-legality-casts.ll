; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,print<hir>,hir-loop-interchange,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that we are able to interchange i2-i3 loopnest by resolving all DD
; issues in the presence of sext/zext in indices like this-
; (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2 * zext.i32.i64(%i76) + 2]

; CHECK: Function:

; CHECK: |   + DO i2 = 0, %i76 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>
; CHECK: |   |   + DO i3 = 0, ((-1 + sext.i32.i64(%i9)) /u (2 * zext.i32.i64(%i76))), 1   <DO_LOOP>  <MAX_TC_EST = 536870912>
; CHECK: |   |   |   %i115 = (%i74)[2 * i2];
; CHECK: |   |   |   %i116 = (%i74)[2 * i2 + 1];
; CHECK: |   |   |   %i126 = (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2 * zext.i32.i64(%i76) + 2];
; CHECK: |   |   |   %i130 = (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2 * zext.i32.i64(%i76) + 3];
; CHECK: |   |   |   %i131 = %i126  *  %i115;
; CHECK: |   |   |   %i132 = %i130  *  %i116;
; CHECK: |   |   |   %i133 = %i131  -  %i132;
; CHECK: |   |   |   %i134 = %i130  *  %i115;
; CHECK: |   |   |   %i135 = %i126  *  %i116;
; CHECK: |   |   |   %i136 = %i134  +  %i135;
; CHECK: |   |   |   %i139 = (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2];
; CHECK: |   |   |   %i140 = %i139  -  %i133;
; CHECK: |   |   |   (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2 * zext.i32.i64(%i76) + 2] = %i140;
; CHECK: |   |   |   %i144 = (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 3];
; CHECK: |   |   |   %i145 = %i144  -  %i136;
; CHECK: |   |   |   (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2 * zext.i32.i64(%i76) + 3] = %i145;
; CHECK: |   |   |   %i146 = %i133  +  %i139;
; CHECK: |   |   |   (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 2] = %i146;
; CHECK: |   |   |   %i147 = %i144  +  %i136;
; CHECK: |   |   |   (%i5)[2 * i2 + sext.i32.i64((4 * %i76)) * i3 + 3] = %i147;
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP

; CHECK: Function:
; CHECK: modified

; CHECK: |      + DO i2 = 0, ((-1 + sext.i32.i64(%i9)) /u (2 * zext.i32.i64(%i76))), 1   <DO_LOOP>  <MAX_TC_EST = 536870912>
; CHECK: |      |   + DO i3 = 0, %i76 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 536870911>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32, i32, i32, i16, i64, i64, i64, i64, i16, %struct.snr_result_s, [4 x double], ptr }
%struct.version_number = type { i8, i8, i8, i8 }
%struct.snr_result_s = type { double, double, double, i32, i32, i32, double, double, i32, i32, double, i32 }
%struct.ee_connection_s = type { i32, i32, ptr, i32, i32, %union.pthread_mutex_t, %union.pthread_cond_t, ptr, i32 }
%union.pthread_mutex_t = type { %struct.__pthread_mutex_s }
%struct.__pthread_mutex_s = type { i32, i32, i32, i32, i32, i16, i16, %struct.__pthread_internal_list }
%struct.__pthread_internal_list = type { ptr, ptr }
%union.pthread_cond_t = type { %struct.__pthread_cond_s }
%struct.__pthread_cond_s = type { %union.anon, %union.anon, [2 x i32], [2 x i32], i32, i32, [2 x i32] }
%union.anon = type { i64 }
%struct.intparts_s = type { i8, i16, i32, i32 }

; Function Attrs: nofree nounwind uwtable
define hidden void @foo(ptr %arg, ptr nocapture readonly %arg1, ptr noalias %i5, ptr noalias %i8) {
bb:
  %i = getelementptr inbounds i8, ptr %arg1, i64 16
  %i2 = bitcast ptr %i to ptr
  %i3 = load i32, ptr %i2, align 8, !tbaa !6
  %i4 = bitcast ptr %arg1 to ptr
  %i6 = getelementptr inbounds i8, ptr %arg1, i64 8
  %i7 = bitcast ptr %i6 to ptr
  %i9 = sdiv i32 %i3, 2
  %i10 = icmp eq i32 %i9, 1
  br i1 %i10, label %bb165, label %bb11

bb11:                                             ; preds = %bb
  %i12 = icmp sgt i32 %i9, 1
  br i1 %i12, label %bb13, label %bb22

bb13:                                             ; preds = %bb11
  br label %bb14

bb14:                                             ; preds = %bb14, %bb13
  %i15 = phi i32 [ %i18, %bb14 ], [ 0, %bb13 ]
  %i16 = phi i32 [ %i17, %bb14 ], [ 1, %bb13 ]
  %i17 = shl nsw i32 %i16, 1
  %i18 = add nuw nsw i32 %i15, 1
  %i19 = icmp slt i32 %i17, %i9
  br i1 %i19, label %bb14, label %bb20

bb20:                                             ; preds = %bb14
  %i21 = phi i32 [ %i18, %bb14 ]
  br label %bb22

bb22:                                             ; preds = %bb20, %bb11
  %i23 = phi i32 [ 0, %bb11 ], [ %i21, %bb20 ]
  %i24 = shl nuw i32 1, %i23
  %i25 = icmp eq i32 %i24, %i9
  br i1 %i25, label %bb27, label %bb26

bb26:                                             ; preds = %bb22
  unreachable

bb27:                                             ; preds = %bb22
  %i28 = icmp eq i32 %i3, 0
  br i1 %i28, label %bb165, label %bb29

bb29:                                             ; preds = %bb27
  br i1 %i12, label %bb30, label %bb67

bb30:                                             ; preds = %bb29
  %i31 = add nsw i32 %i9, -1
  %i32 = zext i32 %i31 to i64
  br label %bb66

bb66:                                             ; preds = %bb60
  br label %bb67

bb67:                                             ; preds = %bb66, %bb29
  %i68 = icmp sgt i32 %i23, 0
  %i69 = icmp sgt i32 %i9, 0
  %i70 = sext i32 %i9 to i64
  %i71 = select i1 %i68, i1 %i69, i1 false
  br i1 %i71, label %bb72, label %bb165

bb72:                                             ; preds = %bb67
  br label %bb73

bb73:                                             ; preds = %bb160, %bb72
  %i74 = phi ptr [ %i161, %bb160 ], [ %i8, %bb72 ]
  %i75 = phi i32 [ %i162, %bb160 ], [ 0, %bb72 ]
  %i76 = phi i32 [ %i77, %bb160 ], [ 1, %bb72 ]
  %i77 = shl nsw i32 %i76, 1
  %i78 = zext i32 %i76 to i64
  %i79 = shl nuw nsw i64 %i78, 1
  br label %bb80

bb80:                                             ; preds = %bb80, %bb73
  %i81 = phi i64 [ 0, %bb73 ], [ %i104, %bb80 ]
  %i82 = phi i32 [ 0, %bb73 ], [ %i105, %bb80 ]
  %i83 = shl nsw i32 %i82, 1
  %i84 = add nuw nsw i32 %i82, %i76
  %i85 = shl nsw i32 %i84, 1
  %i86 = sext i32 %i85 to i64
  %i87 = getelementptr inbounds double, ptr %i5, i64 %i86
  %i88 = load double, ptr %i87, align 8, !tbaa !20, !alias.scope !17
  %i89 = or i32 %i85, 1
  %i90 = sext i32 %i89 to i64
  %i91 = getelementptr inbounds double, ptr %i5, i64 %i90
  %i92 = load double, ptr %i91, align 8, !tbaa !20, !alias.scope !17
  %i93 = sext i32 %i83 to i64
  %i94 = getelementptr inbounds double, ptr %i5, i64 %i93
  %i95 = load double, ptr %i94, align 8, !tbaa !20, !alias.scope !17
  %i96 = fsub fast double %i95, %i88
  store double %i96, ptr %i87, align 8, !tbaa !20, !alias.scope !17
  %i97 = or i32 %i83, 1
  %i98 = sext i32 %i97 to i64
  %i99 = getelementptr inbounds double, ptr %i5, i64 %i98
  %i100 = load double, ptr %i99, align 8, !tbaa !20, !alias.scope !17
  %i101 = fsub fast double %i100, %i92
  store double %i101, ptr %i91, align 8, !tbaa !20, !alias.scope !17
  %i102 = fadd fast double %i95, %i88
  store double %i102, ptr %i94, align 8, !tbaa !20, !alias.scope !17
  %i103 = fadd fast double %i100, %i92
  store double %i103, ptr %i99, align 8, !tbaa !20, !alias.scope !17
  %i104 = add nuw nsw i64 %i81, %i79
  %i105 = add nuw nsw i32 %i82, %i77
  %i106 = icmp slt i64 %i104, %i70
  br i1 %i106, label %bb80, label %bb107

bb107:                                            ; preds = %bb80
  %i108 = icmp ugt i32 %i76, 1
  br i1 %i108, label %bb109, label %bb160

bb109:                                            ; preds = %bb107
  %i110 = add nsw i32 %i76, -2
  br label %bb111

bb111:                                            ; preds = %bb151, %bb109
  %i112 = phi ptr [ %i152, %bb151 ], [ %i74, %bb109 ]
  %i113 = phi i32 [ %i153, %bb151 ], [ 1, %bb109 ]
  %i114 = getelementptr inbounds double, ptr %i112, i64 1
  %i115 = load double, ptr %i112, align 8, !tbaa !20, !noalias !17
  %i116 = load double, ptr %i114, align 8, !tbaa !20, !noalias !17
  br label %bb117

bb117:                                            ; preds = %bb117, %bb111
  %i118 = phi i64 [ 0, %bb111 ], [ %i148, %bb117 ]
  %i119 = phi i32 [ 0, %bb111 ], [ %i149, %bb117 ]
  %i120 = add nuw nsw i32 %i119, %i113
  %i121 = shl nsw i32 %i120, 1
  %i122 = add nuw nsw i32 %i120, %i76
  %i123 = shl nsw i32 %i122, 1
  %i124 = sext i32 %i123 to i64
  %i125 = getelementptr inbounds double, ptr %i5, i64 %i124
  %i126 = load double, ptr %i125, align 8, !tbaa !20, !alias.scope !17
  %i127 = or i32 %i123, 1
  %i128 = sext i32 %i127 to i64
  %i129 = getelementptr inbounds double, ptr %i5, i64 %i128
  %i130 = load double, ptr %i129, align 8, !tbaa !20, !alias.scope !17
  %i131 = fmul fast double %i126, %i115
  %i132 = fmul fast double %i130, %i116
  %i133 = fsub fast double %i131, %i132
  %i134 = fmul fast double %i130, %i115
  %i135 = fmul fast double %i126, %i116
  %i136 = fadd fast double %i134, %i135
  %i137 = sext i32 %i121 to i64
  %i138 = getelementptr inbounds double, ptr %i5, i64 %i137
  %i139 = load double, ptr %i138, align 8, !tbaa !20, !alias.scope !17
  %i140 = fsub fast double %i139, %i133
  store double %i140, ptr %i125, align 8, !tbaa !20, !alias.scope !17
  %i141 = or i32 %i121, 1
  %i142 = sext i32 %i141 to i64
  %i143 = getelementptr inbounds double, ptr %i5, i64 %i142
  %i144 = load double, ptr %i143, align 8, !tbaa !20, !alias.scope !17
  %i145 = fsub fast double %i144, %i136
  store double %i145, ptr %i129, align 8, !tbaa !20, !alias.scope !17
  %i146 = fadd fast double %i133, %i139
  store double %i146, ptr %i138, align 8, !tbaa !20, !alias.scope !17
  %i147 = fadd fast double %i144, %i136
  store double %i147, ptr %i143, align 8, !tbaa !20, !alias.scope !17
  %i148 = add nuw nsw i64 %i118, %i79
  %i149 = add nuw nsw i32 %i119, %i77
  %i150 = icmp slt i64 %i148, %i70
  br i1 %i150, label %bb117, label %bb151

bb151:                                            ; preds = %bb117
  %i152 = getelementptr inbounds double, ptr %i112, i64 2
  %i153 = add nuw nsw i32 %i113, 1
  %i154 = icmp eq i32 %i153, %i76
  br i1 %i154, label %bb155, label %bb111

bb155:                                            ; preds = %bb151
  %i156 = zext i32 %i110 to i64
  %i157 = shl nuw nsw i64 %i156, 1
  %i158 = getelementptr double, ptr %i74, i64 2
  %i159 = getelementptr double, ptr %i158, i64 %i157
  br label %bb160

bb160:                                            ; preds = %bb155, %bb107
  %i161 = phi ptr [ %i74, %bb107 ], [ %i159, %bb155 ]
  %i162 = add nuw nsw i32 %i75, 1
  %i163 = icmp eq i32 %i162, %i23
  br i1 %i163, label %bb164, label %bb73

bb164:                                            ; preds = %bb160
  br label %bb165

bb165:
  ret void
}


!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !11, i64 16}
!7 = !{!"struct@radix2_params_s", !8, i64 0, !8, i64 8, !11, i64 16, !11, i64 20, !12, i64 24, !13, i64 32, !13, i64 44, !13, i64 56, !11, i64 68, !11, i64 72}
!8 = !{!"pointer@_ZTSPd", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!"int", !9, i64 0}
!12 = !{!"pointer@_ZTSP10intparts_s", !9, i64 0}
!13 = !{!"struct@intparts_s", !9, i64 0, !14, i64 2, !11, i64 4, !11, i64 8}
!14 = !{!"short", !9, i64 0}
!15 = !{!7, !8, i64 0}
!16 = !{!7, !8, i64 8}
!17 = !{!18}
!18 = distinct !{!18, !19, !"FFT_transform_internal: %data"}
!19 = distinct !{!19, !"FFT_transform_internal"}
!20 = !{!21, !21, i64 0}
!21 = !{!"double", !9, i64 0}
!22 = !{!23, !18}
!23 = distinct !{!23, !24, !"FFT_bitreverse: %data"}
!24 = distinct !{!24, !"FFT_bitreverse"}
!25 = distinct !{!25, !26}
!26 = !{!"llvm.loop.mustprogress"}
!27 = !{!7, !11, i64 68}
!28 = !{!29, !14, i64 160}
!29 = !{!"struct@TCDef", !30, i64 0, !30, i64 16, !30, i64 32, !30, i64 48, !31, i64 64, !14, i64 128, !32, i64 130, !32, i64 134, !32, i64 138, !11, i64 144, !11, i64 148, !11, i64 152, !11, i64 156, !14, i64 160, !33, i64 168, !33, i64 176, !33, i64 184, !33, i64 192, !14, i64 200, !34, i64 208, !35, i64 288, !36, i64 320}
!30 = !{!"array@_ZTSA16_c", !9, i64 0}
!31 = !{!"array@_ZTSA64_c", !9, i64 0}
!32 = !{!"struct@", !9, i64 0, !9, i64 1, !9, i64 2, !9, i64 3}
!33 = !{!"long", !9, i64 0}
!34 = !{!"struct@snr_result_s", !21, i64 0, !21, i64 8, !21, i64 16, !11, i64 24, !11, i64 28, !11, i64 32, !21, i64 40, !21, i64 48, !11, i64 56, !11, i64 60, !21, i64 64, !11, i64 72}
!35 = !{!"array@_ZTSA4_d", !21, i64 0}
!36 = !{!"pointer@_ZTSPP15ee_connection_s", !9, i64 0}
!37 = !{!29, !33, i64 168}
!38 = !{!29, !21, i64 288}

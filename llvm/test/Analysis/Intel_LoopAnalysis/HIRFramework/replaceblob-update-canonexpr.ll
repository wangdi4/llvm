; Check that the verification passes after loop reversal. It was previously failing because replaceBlob() did not update CanonExpr src/dest type.

; RUN: opt < %s -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-loop-reversal -hir-loop-reversal 2>&1 | FileCheck %s

; CHECK:             if (%9 == 0)
; CHECK:             {
; CHECK:               + DO i1 = 0, -1 * zext.i32.i64(%5) + %indvars.iv307 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 42>
; CHECK:               |   %11 = (@a1_w)[0][i1 + -1 * %indvars.iv307 + 186];
; CHECK:               |   (@a1_w)[0][i1 + -1 * %indvars.iv307 + 186] = %11 + trunc.i64.i32(%indvars.iv310);
; CHECK:               + END LOOP
; CHECK:             }
; CHECK:             else
; CHECK:             {
; CHECK:               + DO i1 = 0, -1 * zext.i32.i64(%5) + %indvars.iv307 + -1, 1   <DO_LOOP>  <MAX_TC_EST = 42>
; CHECK:                |   %10 = (@a1_w)[0][%indvars.iv310];
; CHECK:                |   %xor83.us = %10  ^  6;
; CHECK:                |   (@a1_w)[0][%indvars.iv310] = %xor83.us;
; CHECK:               + END LOOP
; CHECK:             }

;Module Before HIR; ModuleID = '20160925_050522.test.1238.compfail.cpp'
source_filename = "20160925_050522.test.1238.compfail.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_eqw = local_unnamed_addr global i32 708, align 4
@g_pr = local_unnamed_addr global i16 70, align 2
@g_er = local_unnamed_addr global i32 79, align 4
@a1_w = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_awoie = local_unnamed_addr global [192 x i16] zeroinitializer, align 16
@a2_z = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %v_icp = alloca i32, align 4
  %0 = bitcast i32* %v_icp to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #6
  %1 = load i32, i32* @g_er, align 4, !tbaa !1
  %2 = load i32, i32* @g_eqw, align 4, !tbaa !1
  %or = or i32 %2, %1
  store i32 %or, i32* %v_icp, align 4, !tbaa !1
  store i32 63, i32* @g_eqw, align 4, !tbaa !1
  br label %for.cond2.preheader

for.cond2.preheader:                              ; preds = %entry, %for.end139
  %3 = phi i32 [ 63, %entry ], [ %dec141, %for.end139 ]
  %4 = phi i32 [ %1, %entry ], [ 42, %for.end139 ]
  br label %for.body16

for.body16:                                       ; preds = %if.end128.for.body16_crit_edge, %for.cond2.preheader
  %5 = phi i32 [ %3, %for.cond2.preheader ], [ %.pre315, %if.end128.for.body16_crit_edge ]
  %6 = phi i32 [ %4, %for.cond2.preheader ], [ %.pre, %if.end128.for.body16_crit_edge ]
  %indvars.iv310 = phi i64 [ 41, %for.cond2.preheader ], [ %indvars.iv.next311, %if.end128.for.body16_crit_edge ]
  %indvars.iv307 = phi i64 [ 42, %for.cond2.preheader ], [ %indvars.iv.next308, %if.end128.for.body16_crit_edge ]
  %indvars.iv303 = phi i32 [ 3838, %for.cond2.preheader ], [ %indvars.iv.next304, %if.end128.for.body16_crit_edge ]
  %v_jq.0291 = phi i32 [ %4, %for.cond2.preheader ], [ %or127, %if.end128.for.body16_crit_edge ]
  %add17 = add i32 %5, %6
  %conv21 = zext i32 %add17 to i64
  switch i32 %add17, label %sw.default [
    i32 70, label %if.end128
    i32 58, label %sw.bb
  ]

sw.bb:                                            ; preds = %for.body16
  %7 = mul nuw nsw i64 %indvars.iv310, 3
  %8 = trunc i64 %indvars.iv310 to i32
  %conv38 = zext i32 %5 to i64
  %cmp41284 = icmp ugt i64 %conv38, %indvars.iv310
  br i1 %cmp41284, label %for.end103, label %for.body42.lr.ph

for.body42.lr.ph:                                 ; preds = %sw.bb
  %tobool68 = icmp eq i32 %v_jq.0291, 0
  %arrayidx82 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_w, i64 0, i64 %indvars.iv310
  br i1 %tobool68, label %for.body42.us.preheader, label %for.body42.preheader

for.body42.preheader:                             ; preds = %for.body42.lr.ph
  br label %for.body42

for.body42.us.preheader:                          ; preds = %for.body42.lr.ph
  %9 = load i16, i16* @g_pr, align 2, !tbaa !5
  %tobool74.us = icmp eq i16 %9, 0
  br label %for.body42.us

for.body42.us:                                    ; preds = %for.body42.us.preheader, %if.end99.us
  %v_yo.0285.us = phi i64 [ %inc102.us, %if.end99.us ], [ %conv38, %for.body42.us.preheader ]
  br i1 %tobool74.us, label %if.else.us, label %if.then75.us

if.then75.us:                                     ; preds = %for.body42.us
  %10 = load i32, i32* %arrayidx82, align 4, !tbaa !7
  %xor83.us = xor i32 %10, 6
  store i32 %xor83.us, i32* %arrayidx82, align 4, !tbaa !7
  br label %if.end99.us

if.else.us:                                       ; preds = %for.body42.us
  %sub84.us = sub i64 185, %v_yo.0285.us
  %arrayidx85.us = getelementptr inbounds [192 x i32], [192 x i32]* @a1_w, i64 0, i64 %sub84.us
  %11 = load i32, i32* %arrayidx85.us, align 4, !tbaa !7
  %add86.us = add i32 %11, %8
  store i32 %add86.us, i32* %arrayidx85.us, align 4, !tbaa !7
  br label %if.end99.us

if.end99.us:                                      ; preds = %if.else.us, %if.then75.us
  %inc102.us = add nuw nsw i64 %v_yo.0285.us, 1
  %exitcond309 = icmp eq i64 %inc102.us, %indvars.iv307
  br i1 %exitcond309, label %for.end103.loopexit, label %for.body42.us

for.body42:                                       ; preds = %for.body42.preheader, %if.end99
  %v_yo.0285 = phi i64 [ %inc102, %if.end99 ], [ %conv38, %for.body42.preheader ]
  call void @_Z4swapRjS_(i32* nonnull dereferenceable(4) %v_icp, i32* nonnull dereferenceable(4) %v_icp)
  %12 = load i16, i16* @g_pr, align 2, !tbaa !5
  %tobool74 = icmp eq i16 %12, 0
  br i1 %tobool74, label %if.else, label %if.then75

if.then75:                                        ; preds = %for.body42
  %13 = load i32, i32* %arrayidx82, align 4, !tbaa !7
  %xor83 = xor i32 %13, 6
  store i32 %xor83, i32* %arrayidx82, align 4, !tbaa !7
  br label %if.end99

if.else:                                          ; preds = %for.body42
  %sub84 = sub i64 185, %v_yo.0285
  %arrayidx85 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_w, i64 0, i64 %sub84
  %14 = load i32, i32* %arrayidx85, align 4, !tbaa !7
  %add86 = add i32 %14, %8
  store i32 %add86, i32* %arrayidx85, align 4, !tbaa !7
  br label %if.end99

if.end99:                                         ; preds = %if.then75, %if.else
  %inc102 = add nuw nsw i64 %v_yo.0285, 1
  %exitcond = icmp eq i64 %inc102, %indvars.iv307
  br i1 %exitcond, label %for.end103.loopexit296, label %for.body42

for.end103.loopexit296:                           ; preds = %if.end99
  %15 = mul i32 %5, -90
  %16 = add i32 %indvars.iv303, %15
  %.pre317 = load i32, i32* @g_eqw, align 4, !tbaa !1
  br label %for.end103

for.end103.loopexit:                              ; preds = %if.end99.us
  br label %for.end103

for.end103:                                       ; preds = %for.end103.loopexit, %for.end103.loopexit296, %sw.bb
  %17 = phi i32 [ %5, %sw.bb ], [ %.pre317, %for.end103.loopexit296 ], [ %5, %for.end103.loopexit ]
  %v_yo.0.lcssa = phi i64 [ %conv38, %sw.bb ], [ %indvars.iv307, %for.end103.loopexit296 ], [ %indvars.iv307, %for.end103.loopexit ]
  %v_mcnxqlr.0.lcssa = phi i32 [ 58, %sw.bb ], [ %16, %for.end103.loopexit296 ], [ 58, %for.end103.loopexit ]
  %idxprom107 = zext i32 %17 to i64
  %arrayidx109 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_z, i64 0, i64 %idxprom107, i64 %7
  %18 = load i32, i32* %arrayidx109, align 4, !tbaa !9
  %arrayidx111 = getelementptr inbounds [192 x i16], [192 x i16]* @a1_awoie, i64 0, i64 %idxprom107
  %19 = load i16, i16* %arrayidx111, align 2, !tbaa !11
  %conv112 = zext i16 %19 to i32
  %and113 = and i32 %conv112, %18
  %conv114 = trunc i32 %and113 to i16
  store i16 %conv114, i16* %arrayidx111, align 2, !tbaa !11
  %mul115 = mul i32 %v_mcnxqlr.0.lcssa, 68
  br label %if.end128

sw.default:                                       ; preds = %for.body16
  call void @_Z6printbj(i32 %v_jq.0291)
  %20 = load i32, i32* @g_er, align 4, !tbaa !1
  %21 = trunc i64 %indvars.iv310 to i32
  %or124 = or i32 %20, %21
  store i32 %or124, i32* @g_er, align 4, !tbaa !1
  br label %if.end128

if.end128:                                        ; preds = %for.body16, %sw.default, %for.end103
  %v_yo.3 = phi i64 [ %conv21, %sw.default ], [ %v_yo.0.lcssa, %for.end103 ], [ %conv21, %for.body16 ]
  %v_mcnxqlr.5 = phi i32 [ %add17, %sw.default ], [ %mul115, %for.end103 ], [ 70, %for.body16 ]
  %22 = load i32, i32* %v_icp, align 4, !tbaa !1
  %call = call i32 @_Z4copyj(i32 %22)
  %mul129 = mul i64 %v_yo.3, %v_yo.3
  %add130 = add i64 %mul129, 87
  %23 = shl i64 %indvars.iv310, 1
  %24 = sub nuw nsw i64 145, %23
  %arrayidx134 = getelementptr inbounds [192 x i16], [192 x i16]* @a1_awoie, i64 0, i64 %24
  %25 = load i16, i16* %arrayidx134, align 2, !tbaa !11
  %conv135 = zext i16 %25 to i64
  %and136 = and i64 %conv135, %add130
  %conv137 = trunc i64 %and136 to i16
  store i16 %conv137, i16* %arrayidx134, align 2, !tbaa !11
  call void @_Z6printbj(i32 %v_mcnxqlr.5)
  %indvars.iv.next311 = add nsw i64 %indvars.iv310, -1
  %cmp15 = icmp eq i64 %indvars.iv.next311, 0
  br i1 %cmp15, label %for.end139, label %if.end128.for.body16_crit_edge

if.end128.for.body16_crit_edge:                   ; preds = %if.end128
  %indvars.iv.next308 = add nsw i64 %indvars.iv307, -1
  %indvars.iv.next304 = add nsw i32 %indvars.iv303, -90
  %add126 = add i32 %call, %v_jq.0291
  %or127 = or i32 %add126, 83
  %.pre = load i32, i32* @g_er, align 4, !tbaa !1
  %.pre315 = load i32, i32* @g_eqw, align 4, !tbaa !1
  br label %for.body16

for.end139:                                       ; preds = %if.end128
  store i32 42, i32* @g_er, align 4, !tbaa !1
  %26 = load i32, i32* @g_eqw, align 4, !tbaa !1
  %dec141 = add i32 %26, -1
  store i32 %dec141, i32* @g_eqw, align 4, !tbaa !1
  %cmp = icmp ugt i32 %dec141, 28
  br i1 %cmp, label %for.cond2.preheader, label %for.end142

for.end142:                                       ; preds = %for.end139
  store i32 10, i32* @g_er, align 4, !tbaa !1
  %27 = load i32, i32* %v_icp, align 4, !tbaa !1
  call void @_Z6printbj(i32 %27)
  %28 = load i32, i32* @g_eqw, align 4, !tbaa !1
  call void @_Z6printbj(i32 %28)
  %29 = load i16, i16* @g_pr, align 2, !tbaa !5
  call void @_Z6printbt(i16 zeroext %29)
  %30 = load i32, i32* @g_er, align 4, !tbaa !1
  call void @_Z6printbj(i32 %30)
  br label %for.body146

for.body146:                                      ; preds = %for.end142, %for.body146
  %indvars.iv301 = phi i64 [ 0, %for.end142 ], [ %indvars.iv.next302, %for.body146 ]
  %arrayidx148 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_w, i64 0, i64 %indvars.iv301
  %31 = load i32, i32* %arrayidx148, align 4, !tbaa !7
  call void @_Z6printbj(i32 %31)
  %indvars.iv.next302 = add nuw nsw i64 %indvars.iv301, 1
  %cmp145 = icmp eq i64 %indvars.iv.next302, 192
  br i1 %cmp145, label %for.body157.preheader, label %for.body146

for.body157.preheader:                            ; preds = %for.body146
  br label %for.body157

for.body157:                                      ; preds = %for.body157.preheader, %for.body157
  %indvars.iv299 = phi i64 [ %indvars.iv.next300, %for.body157 ], [ 0, %for.body157.preheader ]
  %arrayidx159 = getelementptr inbounds [192 x i16], [192 x i16]* @a1_awoie, i64 0, i64 %indvars.iv299
  %32 = load i16, i16* %arrayidx159, align 2, !tbaa !11
  call void @_Z6printbt(i16 zeroext %32)
  %indvars.iv.next300 = add nuw nsw i64 %indvars.iv299, 1
  %cmp155 = icmp eq i64 %indvars.iv.next300, 192
  br i1 %cmp155, label %for.cond169.preheader.preheader, label %for.body157

for.cond169.preheader.preheader:                  ; preds = %for.body157
  br label %for.cond169.preheader

for.cond169.preheader:                            ; preds = %for.cond169.preheader.preheader, %for.cond.cleanup171
  %indvars.iv297 = phi i64 [ %indvars.iv.next298, %for.cond.cleanup171 ], [ 0, %for.cond169.preheader.preheader ]
  br label %for.body172

for.cond.cleanup166:                              ; preds = %for.cond.cleanup171
  call void @_Z11flushprintbv()
  call void @llvm.lifetime.end(i64 4, i8* %0) #6
  ret i32 0

for.cond.cleanup171:                              ; preds = %for.body172
  %indvars.iv.next298 = add nuw nsw i64 %indvars.iv297, 1
  %cmp165 = icmp eq i64 %indvars.iv.next298, 192
  br i1 %cmp165, label %for.cond.cleanup166, label %for.cond169.preheader

for.body172:                                      ; preds = %for.cond169.preheader, %for.body172
  %indvars.iv = phi i64 [ 0, %for.cond169.preheader ], [ %indvars.iv.next, %for.body172 ]
  %arrayidx176 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_z, i64 0, i64 %indvars.iv297, i64 %indvars.iv
  %33 = load i32, i32* %arrayidx176, align 4, !tbaa !9
  call void @_Z6printbj(i32 %33)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp170 = icmp eq i64 %indvars.iv.next, 192
  br i1 %cmp170, label %for.cond.cleanup171, label %for.body172
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

declare void @_Z4swapRjS_(i32* dereferenceable(4), i32* dereferenceable(4)) local_unnamed_addr #2

declare void @_Z6printbj(i32) local_unnamed_addr #2

declare i32 @_Z4copyj(i32) local_unnamed_addr #2

declare void @_Z6printbt(i16 zeroext) local_unnamed_addr #2

declare void @_Z11flushprintbv() local_unnamed_addr #2


!0 = !{!"clang version 4.0.0 (trunk 17978) (llvm/branches/loopopt 20388)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"short", !3, i64 0}
!7 = !{!8, !2, i64 0}
!8 = !{!"array@_ZTSA192_j", !2, i64 0}
!9 = !{!10, !2, i64 0}
!10 = !{!"array@_ZTSA192_A192_j", !8, i64 0}
!11 = !{!12, !6, i64 0}
!12 = !{!"array@_ZTSA192_t", !6, i64 0}

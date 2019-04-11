; Check reassociation not happen when the matched latch binary operator (%t246) is in the
; instruction chain.
;
; RUN: opt < %s -loop-carried-cse -S 2>&1 | FileCheck %s
; RUN: opt -passes="loop-carried-cse" -S 2>&1 < %s | FileCheck %s
;
; CHECK: %t593.0 = phi i16 [ %gepload1753, %afterloop.930 ], [ %gepload1756, %loop.664 ]
; CHECK: %t592.0 = phi i16 [ %gepload1750, %afterloop.930 ], [ %t593.0, %loop.664 ]
; CHECK: %t246 = add i16 %t593.0, %gepload1756
; CEHCK: %t247 = add i16 %t246, %t592.0

; Function Attrs: nounwind uwtable
define void @silk_pitch_analysis_core_FLP(float* %frame, i32* nocapture %pitch_out, i16* nocapture %lagIndex, i8* nocapture %contourIndex, float* nocapture %LTPCorr, i32 %prevLag, float %search_thres1, float %search_thres2, i32 %Fs_kHz, i32 %complexity, i32 %nb_subfr, i32 %arch) local_unnamed_addr #0 {
entry:
  %d_comp = alloca [149 x i16], align 16
  br label %afterloop.930

afterloop.930:                                    ; preds = %loop.930
  %arrayIdx1749 = getelementptr inbounds [149 x i16], [149 x i16]* %d_comp, i64 0, i64 18
  %gepload1750 = load i16, i16* %arrayIdx1749, align 4
  %arrayIdx1752 = getelementptr inbounds [149 x i16], [149 x i16]* %d_comp, i64 0, i64 17
  %gepload1753 = load i16, i16* %arrayIdx1752, align 2
  br label %loop.664
loop.664:                                         ; preds = %loop.664, %afterloop.930
  %t593.0 = phi i16 [ %gepload1753, %afterloop.930 ], [ %gepload1756, %loop.664 ]
  %t592.0 = phi i16 [ %gepload1750, %afterloop.930 ], [ %t593.0, %loop.664 ]
  %i1.i641706.1 = phi i64 [ 128, %afterloop.930 ], [ %nextivloop.664, %loop.664 ]
  %t245 = sub nsw i64 144, %i1.i641706.1
  %arrayIdx1755 = getelementptr inbounds [149 x i16], [149 x i16]* %d_comp, i64 0, i64 %t245
  %gepload1756 = load i16, i16* %arrayIdx1755, align 2
  %t246 = add i16 %t593.0, %gepload1756
  %t247 = add i16 %t246, %t592.0
  %t248 = sub nsw i64 146, %i1.i641706.1
  %arrayIdx1761 = getelementptr inbounds [149 x i16], [149 x i16]* %d_comp, i64 0, i64 %t248
  store i16 %t247, i16* %arrayIdx1761, align 2
  %nextivloop.664 = add nuw nsw i64 %i1.i641706.1, 1
  %condloop.664 = icmp ult i64 %nextivloop.664, 131
  br i1 %condloop.664, label %loop.664, label %region.164

region.164:
  ret void
}

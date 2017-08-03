; Expecting PreHdr/PostExit Extracted
;      if (-1 * i1 + 63 <u 33)
;
; REQUIRES: asserts 
; RUN:  opt  < %s  -hir-ssa-deconstruction -hir-loop-distribute-loopnest -debug -hir-loop-interchange -print-after=hir-loop-interchange 2>&1 | FileCheck %s
; CHECK: if (-1 * i1 + 63 <u 33)
; CHECK:  DO i2 = 0, 52, 1  
; CHECK:    DO i3 = 0, i1 + -31, 1 
;
; ModuleID = 'ExtractZTT.cpp'
source_filename = "ExtractZTT.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_krf = global i32 63, align 4
@g_pnyrestwrqei = local_unnamed_addr global i16 49, align 2
@a1_mne = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_aszvb = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_hylg = local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a2_ynbngtbf = local_unnamed_addr global [192 x [192 x i32]] zeroinitializer, align 16
@a2_uf = local_unnamed_addr global [192 x [192 x i8]] zeroinitializer, align 16
@a3_m = local_unnamed_addr global [192 x [192 x [192 x i32]]] zeroinitializer, align 16

; Function Attrs: norecurse uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %v_hwpdr = alloca i64, align 8
  %0 = bitcast i64* %v_hwpdr to i8*
  call void @llvm.lifetime.start(i64 8, i8* %0) #5
  %1 = load i16, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  %conv = zext i16 %1 to i64
  store i64 %conv, i64* %v_hwpdr, align 8, !tbaa !5
  call void @_Z4swapRjRm(i32* nonnull dereferenceable(4) @g_krf, i64* nonnull dereferenceable(8) %v_hwpdr)
  store i64 63, i64* %v_hwpdr, align 8, !tbaa !5
  %2 = load i16, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  br label %for.body

for.body:                                         ; preds = %entry, %for.end131
  %indvars.iv356 = phi i64 [ 63, %entry ], [ %indvars.iv.next357, %for.end131 ]
  %3 = phi i16 [ %2, %entry ], [ %inc130.lcssa335, %for.end131 ]
  %tobool = icmp eq i16 %3, -94
  br i1 %tobool, label %if.else63, label %if.end71

if.else63:                                        ; preds = %for.body
  %arrayidx67 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_mne, i64 0, i64 %indvars.iv356
  %4 = load i32, i32* %arrayidx67, align 4, !tbaa !7
  %and68 = and i32 %4, 69
  store i32 %and68, i32* %arrayidx67, align 4, !tbaa !7
  br label %if.end71

if.end71:                                         ; preds = %for.body, %if.else63
  %conv72 = trunc i64 %indvars.iv356 to i16
  %cmp75329 = icmp ult i16 %conv72, 33
  br i1 %cmp75329, label %for.end97.preheader, label %for.end131

for.end97.preheader:                              ; preds = %if.end71
  %arrayidx78 = getelementptr inbounds [192 x [192 x i8]], [192 x [192 x i8]]* @a2_uf, i64 0, i64 %indvars.iv356, i64 %indvars.iv356
  %arrayidx78.promoted = load i8, i8* %arrayidx78, align 1, !tbaa !9
  br label %for.end97

for.end97:                                        ; preds = %for.end97.preheader, %for.end115
  %5 = phi i8 [ %conv81, %for.end115 ], [ %arrayidx78.promoted, %for.end97.preheader ]
  %indvars.iv358 = phi i64 [ %indvars.iv.next359, %for.end115 ], [ %indvars.iv356, %for.end97.preheader ]
  %conv79 = zext i8 %5 to i64
  %and80 = and i64 %conv79, %indvars.iv356
  %conv81 = trunc i64 %and80 to i8
  br label %for.body103

for.body103:                                      ; preds = %for.body103, %for.end97
  %indvars.iv354 = phi i64 [ 11, %for.end97 ], [ %indvars.iv.next355, %for.body103 ]
  %arrayidx110 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_ynbngtbf, i64 0, i64 %indvars.iv354, i64 %indvars.iv358
  %6 = load i32, i32* %arrayidx110, align 4, !tbaa !7
  %inc111 = add i32 %6, 1
  store i32 %inc111, i32* %arrayidx110, align 4, !tbaa !7
  %indvars.iv.next355 = add nuw nsw i64 %indvars.iv354, 1
  %exitcond = icmp eq i64 %indvars.iv.next355, 64
  br i1 %exitcond, label %for.end115, label %for.body103

for.end115:                                       ; preds = %for.body103
  %indvars.iv.next359 = add nuw nsw i64 %indvars.iv358, 1
  %exitcond360 = icmp eq i64 %indvars.iv.next359, 33
  br i1 %exitcond360, label %for.cond73.for.end131_crit_edge, label %for.end97

for.cond73.for.end131_crit_edge:                  ; preds = %for.end115
  store i8 %conv81, i8* %arrayidx78, align 1, !tbaa !9
  store i32 64, i32* @g_krf, align 4, !tbaa !7
  br label %for.end131

for.end131:                                       ; preds = %for.cond73.for.end131_crit_edge, %if.end71
  %inc130.lcssa335 = phi i16 [ 33, %for.cond73.for.end131_crit_edge ], [ %conv72, %if.end71 ]
  %indvars.iv.next357 = add nsw i64 %indvars.iv356, -1
  %cmp = icmp ugt i64 %indvars.iv.next357, 32
  br i1 %cmp, label %for.body, label %for.end136

for.end136:                                       ; preds = %for.end131
  store i16 32, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  store i64 32, i64* %v_hwpdr, align 8, !tbaa !5
  call void @_Z6printbm(i64 32)
  %7 = load i32, i32* @g_krf, align 4, !tbaa !7
  call void @_Z6printbj(i32 %7)
  %8 = load i16, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  call void @_Z6printbt(i16 zeroext %8)
  br label %for.body143

for.body143:                                      ; preds = %for.end136, %for.body143
  %indvars.iv352 = phi i64 [ 0, %for.end136 ], [ %indvars.iv.next353, %for.body143 ]
  %arrayidx145 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_mne, i64 0, i64 %indvars.iv352
  %9 = load i32, i32* %arrayidx145, align 4, !tbaa !7
  call void @_Z6printbj(i32 %9)
  %indvars.iv.next353 = add nuw nsw i64 %indvars.iv352, 1
  %cmp142 = icmp eq i64 %indvars.iv.next353, 192
  br i1 %cmp142, label %for.body153, label %for.body143

for.body153:                                      ; preds = %for.body143, %for.body153
  %indvars.iv350 = phi i64 [ %indvars.iv.next351, %for.body153 ], [ 0, %for.body143 ]
  %arrayidx155 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_aszvb, i64 0, i64 %indvars.iv350
  %10 = load i32, i32* %arrayidx155, align 4, !tbaa !7
  call void @_Z6printbj(i32 %10)
  %indvars.iv.next351 = add nuw nsw i64 %indvars.iv350, 1
  %cmp151 = icmp eq i64 %indvars.iv.next351, 192
  br i1 %cmp151, label %for.body163, label %for.body153

for.body163:                                      ; preds = %for.body153, %for.body163
  %indvars.iv348 = phi i64 [ %indvars.iv.next349, %for.body163 ], [ 0, %for.body153 ]
  %arrayidx165 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_hylg, i64 0, i64 %indvars.iv348
  %11 = load i32, i32* %arrayidx165, align 4, !tbaa !7
  call void @_Z6printbj(i32 %11)
  %indvars.iv.next349 = add nuw nsw i64 %indvars.iv348, 1
  %cmp161 = icmp eq i64 %indvars.iv.next349, 192
  br i1 %cmp161, label %for.cond174.preheader, label %for.body163

for.cond174.preheader:                            ; preds = %for.body163, %for.cond.cleanup176
  %indvars.iv346 = phi i64 [ %indvars.iv.next347, %for.cond.cleanup176 ], [ 0, %for.body163 ]
  br label %for.body177

for.cond.cleanup176:                              ; preds = %for.body177
  %indvars.iv.next347 = add nuw nsw i64 %indvars.iv346, 1
  %cmp170 = icmp eq i64 %indvars.iv.next347, 192
  br i1 %cmp170, label %for.cond194.preheader, label %for.cond174.preheader

for.body177:                                      ; preds = %for.cond174.preheader, %for.body177
  %indvars.iv344 = phi i64 [ 0, %for.cond174.preheader ], [ %indvars.iv.next345, %for.body177 ]
  %arrayidx181 = getelementptr inbounds [192 x [192 x i32]], [192 x [192 x i32]]* @a2_ynbngtbf, i64 0, i64 %indvars.iv346, i64 %indvars.iv344
  %12 = load i32, i32* %arrayidx181, align 4, !tbaa !7
  call void @_Z6printbj(i32 %12)
  %indvars.iv.next345 = add nuw nsw i64 %indvars.iv344, 1
  %cmp175 = icmp eq i64 %indvars.iv.next345, 192
  br i1 %cmp175, label %for.cond.cleanup176, label %for.body177

for.cond194.preheader:                            ; preds = %for.cond.cleanup176, %for.cond.cleanup196
  %indvars.iv342 = phi i64 [ %indvars.iv.next343, %for.cond.cleanup196 ], [ 0, %for.cond.cleanup176 ]
  br label %for.body197

for.cond.cleanup196:                              ; preds = %for.body197
  %indvars.iv.next343 = add nuw nsw i64 %indvars.iv342, 1
  %cmp190 = icmp eq i64 %indvars.iv.next343, 192
  br i1 %cmp190, label %for.cond213.preheader, label %for.cond194.preheader

for.body197:                                      ; preds = %for.cond194.preheader, %for.body197
  %indvars.iv340 = phi i64 [ 0, %for.cond194.preheader ], [ %indvars.iv.next341, %for.body197 ]
  %arrayidx201 = getelementptr inbounds [192 x [192 x i8]], [192 x [192 x i8]]* @a2_uf, i64 0, i64 %indvars.iv342, i64 %indvars.iv340
  %13 = load i8, i8* %arrayidx201, align 1, !tbaa !9
  call void @_Z6printbh(i8 zeroext %13)
  %indvars.iv.next341 = add nuw nsw i64 %indvars.iv340, 1
  %cmp195 = icmp eq i64 %indvars.iv.next341, 192
  br i1 %cmp195, label %for.cond.cleanup196, label %for.body197

for.cond213.preheader:                            ; preds = %for.cond.cleanup196, %for.cond.cleanup215
  %indvars.iv338 = phi i64 [ %indvars.iv.next339, %for.cond.cleanup215 ], [ 0, %for.cond.cleanup196 ]
  br label %for.cond218.preheader

for.cond.cleanup210:                              ; preds = %for.cond.cleanup215
  call void @_Z11flushprintbv()
  call void @llvm.lifetime.end(i64 8, i8* %0) #5
  ret i32 0

for.cond218.preheader:                            ; preds = %for.cond213.preheader, %for.cond.cleanup220
  %indvars.iv336 = phi i64 [ 0, %for.cond213.preheader ], [ %indvars.iv.next337, %for.cond.cleanup220 ]
  br label %for.body221

for.cond.cleanup215:                              ; preds = %for.cond.cleanup220
  %indvars.iv.next339 = add nuw nsw i64 %indvars.iv338, 1
  %cmp209 = icmp eq i64 %indvars.iv.next339, 192
  br i1 %cmp209, label %for.cond.cleanup210, label %for.cond213.preheader

for.cond.cleanup220:                              ; preds = %for.body221
  %indvars.iv.next337 = add nuw nsw i64 %indvars.iv336, 1
  %cmp214 = icmp eq i64 %indvars.iv.next337, 192
  br i1 %cmp214, label %for.cond.cleanup215, label %for.cond218.preheader

for.body221:                                      ; preds = %for.cond218.preheader, %for.body221
  %indvars.iv = phi i64 [ 0, %for.cond218.preheader ], [ %indvars.iv.next, %for.body221 ]
  %arrayidx227 = getelementptr inbounds [192 x [192 x [192 x i32]]], [192 x [192 x [192 x i32]]]* @a3_m, i64 0, i64 %indvars.iv338, i64 %indvars.iv336, i64 %indvars.iv
  %14 = load i32, i32* %arrayidx227, align 4, !tbaa !7
  call void @_Z6printbj(i32 %14)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp219 = icmp eq i64 %indvars.iv.next, 192
  br i1 %cmp219, label %for.cond.cleanup220, label %for.body221
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @_Z4swapRjRm(i32* dereferenceable(4), i64* dereferenceable(8)) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

declare void @_Z6printbj(i32) local_unnamed_addr #2

declare void @_Z6printbm(i64) local_unnamed_addr #2

declare void @_Z6printbt(i16 zeroext) local_unnamed_addr #2

declare void @_Z6printbh(i8 zeroext) local_unnamed_addr #2

declare void @_Z11flushprintbv() local_unnamed_addr #2

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @_Z7f3_xsoimjj(i64 %v_w, i32 %v_qd, i32 %v_ar) local_unnamed_addr #3 {
entry:
  %0 = load i16, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  %tobool = icmp eq i16 %0, 0
  %mul8 = mul i64 %v_w, 3
  %conv9 = trunc i64 %mul8 to i32
  %1 = select i1 %tobool, i32 76, i32 %conv9
  ret i32 %1
}

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @_Z4f0_qv() local_unnamed_addr #3 {
entry:
  %0 = load i16, i16* @g_pnyrestwrqei, align 2, !tbaa !1
  %tobool = icmp eq i16 %0, 0
  %1 = select i1 %tobool, i32 21, i32 4
  ret i32 %1
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i8 @_Z7f3_xgdqjyj(i32 %v_f, i64 %v_nvf, i32 %v_y) local_unnamed_addr #4 {
entry:
  ret i8 -53
}

; Function Attrs: norecurse nounwind readnone uwtable
define zeroext i8 @_Z4f1_bj(i32 %v_46) local_unnamed_addr #4 {
entry:
  ret i8 71
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z8f2_mbfgamy(i64 %v_77, i64 %v_26) local_unnamed_addr #4 {
entry:
  ret i32 68
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z7f2_dxssjj(i32 %v_68, i32 %v_47) local_unnamed_addr #4 {
entry:
  ret i32 48
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z5f1_naj(i32 %v_12) local_unnamed_addr #4 {
entry:
  ret i32 78
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z5f1_kzj(i32 %v_100) local_unnamed_addr #4 {
entry:
  ret i32 55
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z7f1_fdfzj(i32 %v_19) local_unnamed_addr #4 {
entry:
  ret i32 14
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @_Z10f2_gfqpeftjj(i32 %v_82, i32 %v_42) local_unnamed_addr #4 {
entry:
  ret i32 80
}

attributes #0 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20746) (llvm/branches/loopopt 20753)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"short", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"long", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !3, i64 0}
!9 = !{!3, !3, i64 0}

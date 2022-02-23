; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that we hoist the ztt of the inner loop which contains a blob.

; CHECK: Function

; CHECK: + DO i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |   %1 = (@s)[0][i1 + 4];
; CHECK: |   %cmp744 = i1 + 4 <u 8;
; CHECK: |
; CHECK: |   + DO i2 = 0, i1 + -4, 1   <DO_LOOP>  <MAX_TC_EST = 2>
; CHECK: |   |   %indvars.iv.out = i2 + 8;
; CHECK: |   + END LOOP
; CHECK: |      %2 = (@e)[0][%indvars.iv.out];
; CHECK: |      (@s)[0][i1 + 4] = %2;
; CHECK: + END LOOP

; CHECK: Function

; CHECK: %1 = (@s)[0][4];
; CHECK: %cmp744 = 4 <u 8;
; CHECK: %1 = (@s)[0][5];
; CHECK: %cmp744 = 5 <u 8;
; CHECK: %1 = (@s)[0][6];
; CHECK: %cmp744 = 6 <u 8;
; CHECK: %1 = (@s)[0][7];
; CHECK: %cmp744 = 7 <u 8;
; CHECK: %1 = (@s)[0][8];
; CHECK: %cmp744 = 8 <u 8;

; Ztt for the inner loop

; CHECK: if (umax(%tobool, %cmp744) == 0)
; CHECK: {
; CHECK: %indvars.iv.out = 8;
; CHECK: %2 = (@e)[0][%indvars.iv.out];
; CHECK: (@s)[0][8] = %2;
; CHECK: }
; CHECK: %1 = (@s)[0][9];
; CHECK: %cmp744 = 9 <u 8;

; Ztt for the inner loop

; CHECK: if (umax(%tobool, %cmp744) == 0)
; CHECK: {
; CHECK: %indvars.iv.out = 8;
; CHECK: %indvars.iv.out = 9;
; CHECK: %2 = (@e)[0][%indvars.iv.out];
; CHECK: (@s)[0][9] = %2;
; CHECK: }

;Module Before HIR; ModuleID = '110513-231344.c'
source_filename = "110513-231344.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@z = local_unnamed_addr global i32 0, align 4
@q = local_unnamed_addr global i64 0, align 8
@f = local_unnamed_addr global i64 0, align 8
@b = local_unnamed_addr global i32 0, align 4
@uw = local_unnamed_addr global [20 x i32] [i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0], align 16
@e = local_unnamed_addr global [20 x i16] zeroinitializer, align 16
@s = local_unnamed_addr global [20 x i8] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  call void @llvm.memset.p0i8.i64(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @s, i64 0, i64 0), i8 6, i64 20, i32 16, i1 false)
  store i32 20, i32* @z, align 4, !tbaa !1
  store i32 4, i32* @b, align 4, !tbaa !1
  %0 = load i32, i32* getelementptr inbounds ([20 x i32], [20 x i32]* @uw, i64 0, i64 0), align 16, !tbaa !5
  %tobool = icmp ne i32 %0, 0
  br label %for.body3

for.cond27.preheader:                             ; preds = %for.inc24
  %.lcssa = phi i8 [ %1, %for.inc24 ]
  %conv.le = zext i8 %.lcssa to i64
  store i32 10, i32* @b, align 4, !tbaa !1
  store i64 %conv.le, i64* @q, align 8, !tbaa !7
  store i64 0, i64* @f, align 8, !tbaa !7
  br label %for.body30

for.body3:                                        ; preds = %for.inc24, %entry
  %indvars.iv49 = phi i64 [ 4, %entry ], [ %indvars.iv.next50, %for.inc24 ]
  %arrayidx5 = getelementptr inbounds [20 x i8], [20 x i8]* @s, i64 0, i64 %indvars.iv49
  %1 = load i8, i8* %arrayidx5, align 1, !tbaa !9
  %cmp744 = icmp ult i64 %indvars.iv49, 8
  %or.cond = or i1 %tobool, %cmp744
  br i1 %or.cond, label %for.inc24, label %for.cond10.preheader.preheader

for.cond10.preheader.preheader:                   ; preds = %for.body3
  br label %for.cond10.preheader

for.cond10.preheader:                             ; preds = %for.cond10.preheader.preheader, %for.cond10.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.cond10.preheader ], [ 8, %for.cond10.preheader.preheader ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp7 = icmp ult i64 %indvars.iv, %indvars.iv49
  br i1 %cmp7, label %for.cond10.preheader, label %for.inc24.loopexit

for.inc24.loopexit:                               ; preds = %for.cond10.preheader
  %indvars.iv.lcssa = phi i64 [ %indvars.iv, %for.cond10.preheader ]
  %arrayidx15 = getelementptr inbounds [20 x i16], [20 x i16]* @e, i64 0, i64 %indvars.iv.lcssa
  %2 = load i16, i16* %arrayidx15, align 2, !tbaa !11
  %conv16 = trunc i16 %2 to i8
  store i8 %conv16, i8* %arrayidx5, align 1, !tbaa !9
  br label %for.inc24

for.inc24:                                        ; preds = %for.inc24.loopexit, %for.body3
  %indvars.iv.next50 = add nuw nsw i64 %indvars.iv49, 1
  %exitcond = icmp eq i64 %indvars.iv.next50, 10
  br i1 %exitcond, label %for.cond27.preheader, label %for.body3

for.body30:                                       ; preds = %for.cond27.preheader, %for.body30
  %inc34.sink42 = phi i64 [ 0, %for.cond27.preheader ], [ %inc34, %for.body30 ]
  %arrayidx31 = getelementptr inbounds [20 x i8], [20 x i8]* @s, i64 0, i64 %inc34.sink42
  %3 = load i8, i8* %arrayidx31, align 1, !tbaa !9
  %conv32 = zext i8 %3 to i32
  tail call void @print_int(i32 %conv32) #3
  %4 = load i64, i64* @f, align 8, !tbaa !7
  %inc34 = add i64 %4, 1
  store i64 %inc34, i64* @f, align 8, !tbaa !7
  %cmp28 = icmp ult i64 %inc34, 20
  br i1 %cmp28, label %for.body30, label %for.end35

for.end35:                                        ; preds = %for.body30
  ret i32 0
}

declare void @print_int(i32) local_unnamed_addr #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (trunk 21198) (llvm/branches/loopopt 21242)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA20_j", !2, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"long", !3, i64 0}
!9 = !{!10, !3, i64 0}
!10 = !{!"array@_ZTSA20_h", !3, i64 0}
!11 = !{!12, !13, i64 0}
!12 = !{!"array@_ZTSA20_t", !13, i64 0}
!13 = !{!"short", !3, i64 0}

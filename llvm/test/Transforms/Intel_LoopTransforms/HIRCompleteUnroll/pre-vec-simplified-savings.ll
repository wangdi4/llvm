; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that post vec complete unroll does not assume simplfied savings exposed by pre vec complete unroll (in the form of instructions like %add = 10  +  3) as its own by unrolling the i1-i2 loopnest below.

; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 21, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, i1 + 17, 1   <DO_LOOP>  <MAX_TC_EST = 39>
; CHECK: |   |   %6 = 10;
; CHECK: |   |   if (%2 != 0)
; CHECK: |   |   {
; CHECK: |   |      if (%v_h.022 == 0)
; CHECK: |   |      {
; CHECK: |   |         %add = 10  +  3;
; CHECK: |   |         %add = 13  +  3;
; CHECK: |   |         %add = 16  +  3;
; CHECK: |   |         %add = 19  +  3;
; CHECK: |   |         %add = 22  +  3;
; CHECK: |   |         %add = 25  +  3;
; CHECK: |   |         %add = 28  +  3;
; CHECK: |   |         %add = 31  +  3;
; CHECK: |   |         %add = 34  +  3;
; CHECK: |   |         %add = 37  +  3;
; CHECK: |   |         %add = 40  +  3;
; CHECK: |   |         %add = 43  +  3;
; CHECK: |   |         %add = 46  +  3;
; CHECK: |   |         %add = 49  +  3;
; CHECK: |   |         %add = 52  +  3;
; CHECK: |   |         %add = 55  +  3;
; CHECK: |   |         %add = 58  +  3;
; CHECK: |   |         %add = 61  +  3;
; CHECK: |   |         %6 = %add;
; CHECK: |   |      }
; CHECK: |   |      else
; CHECK: |   |      {
; CHECK: |   |         %v_orvvlzqhjnbsvm.023 = %v_orvvlzqhjnbsvm.023  *  %v_oplf.024;
; CHECK: |   |         %6 = 10;
; CHECK: |   |      }
; CHECK: |   |   }
; CHECK: |   |   %v_h.022 = %v_h.022  -  %v_orvvlzqhjnbsvm.023;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %v_oplf.024 = %v_oplf.024  &  78;
; CHECK: + END LOOP


; CHECK: Function

; CHECK: BEGIN REGION { modified }
; CHECK: + DO i1 = 0, 21, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, i1 + 17, 1   <DO_LOOP>  <MAX_TC_EST = 39>
; CHECK: |   |   %6 = 10;
; CHECK: |   |   if (%2 != 0)
; CHECK: |   |   {
; CHECK: |   |      if (%v_h.022 == 0)
; CHECK: |   |      {
; CHECK: |   |         %add = 10  +  3;
; CHECK: |   |         %add = 13  +  3;
; CHECK: |   |         %add = 16  +  3;
; CHECK: |   |         %add = 19  +  3;
; CHECK: |   |         %add = 22  +  3;
; CHECK: |   |         %add = 25  +  3;
; CHECK: |   |         %add = 28  +  3;
; CHECK: |   |         %add = 31  +  3;
; CHECK: |   |         %add = 34  +  3;
; CHECK: |   |         %add = 37  +  3;
; CHECK: |   |         %add = 40  +  3;
; CHECK: |   |         %add = 43  +  3;
; CHECK: |   |         %add = 46  +  3;
; CHECK: |   |         %add = 49  +  3;
; CHECK: |   |         %add = 52  +  3;
; CHECK: |   |         %add = 55  +  3;
; CHECK: |   |         %add = 58  +  3;
; CHECK: |   |         %add = 61  +  3;
; CHECK: |   |         %6 = %add;
; CHECK: |   |      }
; CHECK: |   |      else
; CHECK: |   |      {
; CHECK: |   |         %v_orvvlzqhjnbsvm.023 = %v_orvvlzqhjnbsvm.023  *  %v_oplf.024;
; CHECK: |   |         %6 = 10;
; CHECK: |   |      }
; CHECK: |   |   }
; CHECK: |   |   %v_h.022 = %v_h.022  -  %v_orvvlzqhjnbsvm.023;
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   %v_oplf.024 = %v_oplf.024  &  78;
; CHECK: + END LOOP


;Module Before HIR; ModuleID = 'atg_cq419100_10.cpp'
source_filename = "atg_cq419100_10.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@g_i = local_unnamed_addr global i16 0, align 2
@g_u = local_unnamed_addr global i32 0, align 4
@g_ejlqld = local_unnamed_addr global i32 0, align 4
@g_pfss = local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
  %1 = load i32, i32* @g_u, align 4, !tbaa !2
  store i32 17, i32* @g_u, align 4, !tbaa !2
  %2 = load i32, i32* @g_ejlqld, align 4
  %tobool14 = icmp eq i32 %2, 0
  br label %3

; <label>:3:                                      ; preds = %7, %0
  %indvars.iv = phi i32 [ 18, %0 ], [ %indvars.iv.next, %7 ]
  %v_oplf.024 = phi i32 [ %1, %0 ], [ %and, %7 ]
  %v_orvvlzqhjnbsvm.023 = phi i32 [ 4, %0 ], [ %v_orvvlzqhjnbsvm.2.lcssa, %7 ]
  %v_h.022 = phi i32 [ 0, %0 ], [ %sub.lcssa, %7 ]
  br label %4

; <label>:4:                                      ; preds = %3, %.loopexit
  %inc.sink20 = phi i32 [ 0, %3 ], [ %inc, %.loopexit ]
  %v_orvvlzqhjnbsvm.118 = phi i32 [ %v_orvvlzqhjnbsvm.023, %3 ], [ %v_orvvlzqhjnbsvm.2, %.loopexit ]
  %v_h.117 = phi i32 [ %v_h.022, %3 ], [ %sub, %.loopexit ]
  br i1 %tobool14, label %.loopexit, label %.lr.ph

.lr.ph:                                           ; preds = %4
  %tobool3 = icmp eq i32 %v_h.117, 0
  br i1 %tobool3, label %.lr.ph.split, label %.split

.lr.ph.split:                                     ; preds = %.lr.ph
  br label %5

; <label>:5:                                      ; preds = %.lr.ph.split, %5
  %conv5.sink16 = phi i16 [ 10, %.lr.ph.split ], [ %add, %5 ]
  %add = add nuw nsw i16 %conv5.sink16, 3
  %cmp2 = icmp ult i16 %add, 64
  br i1 %cmp2, label %5, label %.loopexit.loopexit

.split:                                           ; preds = %.lr.ph
  %mul = mul i32 %v_orvvlzqhjnbsvm.118, %v_oplf.024
  br label %.loopexit

.loopexit.loopexit:                               ; preds = %5
  %add.lcssa = phi i16 [ %add, %5 ]
  br label %.loopexit

.loopexit:                                        ; preds = %.loopexit.loopexit, %4, %.split
  %6 = phi i16 [ 10, %.split ], [ 10, %4 ], [ %add.lcssa, %.loopexit.loopexit ]
  %v_orvvlzqhjnbsvm.2 = phi i32 [ %mul, %.split ], [ %v_orvvlzqhjnbsvm.118, %4 ], [ %v_orvvlzqhjnbsvm.118, %.loopexit.loopexit ]
  %sub = sub i32 %v_h.117, %v_orvvlzqhjnbsvm.2
  %inc = add nuw nsw i32 %inc.sink20, 1
  %exitcond = icmp eq i32 %inc, %indvars.iv
  br i1 %exitcond, label %7, label %4

; <label>:7:                                      ; preds = %.loopexit
  %.lcssa = phi i16 [ %6, %.loopexit ]
  %v_orvvlzqhjnbsvm.2.lcssa = phi i32 [ %v_orvvlzqhjnbsvm.2, %.loopexit ]
  %sub.lcssa = phi i32 [ %sub, %.loopexit ]
  %and = and i32 %v_oplf.024, 78
  %indvars.iv.next = add nuw nsw i32 %indvars.iv, 1
  %exitcond26 = icmp eq i32 %indvars.iv.next, 40
  br i1 %exitcond26, label %8, label %3

; <label>:8:                                      ; preds = %7
  %.lcssa.lcssa = phi i16 [ %.lcssa, %7 ]
  store i16 %.lcssa.lcssa, i16* @g_i, align 2, !tbaa !6
  store i32 39, i32* @g_pfss, align 4, !tbaa !2
  store i32 39, i32* @g_u, align 4, !tbaa !2
  ret i32 0
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 5.0.0 (cfe/trunk)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"short", !4, i64 0}

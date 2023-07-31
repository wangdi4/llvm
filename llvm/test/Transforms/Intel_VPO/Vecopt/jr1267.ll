; Check that the testcase in JR1267 compiles cleanly with LV and VPlan LLVM IR vectorizer
; RUN: opt -S -passes=loop-vectorize %s | FileCheck %s
; RUN: opt -S -vplan-build-vect-candidates=4 -passes=vplan-vec -vplan-force-vf=4 %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'jr1267.c'
source_filename = "jr1267.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ydv = dso_local local_unnamed_addr global i32 0, align 4
@n = dso_local local_unnamed_addr global i32 0, align 4
@ucw = dso_local local_unnamed_addr global i32 0, align 4
@d = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@o = dso_local local_unnamed_addr global [20 x i64] zeroinitializer, align 16
@x = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@l = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@w = dso_local local_unnamed_addr global [20 x i16] zeroinitializer, align 16
@mib = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16
@e = dso_local local_unnamed_addr global [20 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  store i32 0, ptr @ucw, align 4, !tbaa !2
  br label %for.body

for.cond9.preheader:                              ; preds = %for.body
  store i32 1, ptr @ucw, align 4, !tbaa !2
  %0 = load i64, ptr @o, align 16
  %tobool53 = icmp eq i64 %0, 0
  %1 = load i32, ptr @x, align 16
  %tobool56 = icmp eq i32 %1, 0
  %2 = load i32, ptr @mib, align 16, !tbaa !6
  br label %for.body17.lr.ph

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv117 = phi i64 [ 0, %entry ], [ %indvars.iv.next118, %for.body ]
  %arrayidx = getelementptr inbounds [20 x i32], ptr @d, i64 0, i64 %indvars.iv117, !intel-tbaa !6
  store i32 5, ptr %arrayidx, align 4, !tbaa !6
  %arrayidx2 = getelementptr inbounds [20 x i64], ptr @o, i64 0, i64 %indvars.iv117, !intel-tbaa !8
  store i64 4, ptr %arrayidx2, align 8, !tbaa !8
  %arrayidx4 = getelementptr inbounds [20 x i32], ptr @x, i64 0, i64 %indvars.iv117, !intel-tbaa !6
  store i32 3, ptr %arrayidx4, align 4, !tbaa !6
  %arrayidx6 = getelementptr inbounds [20 x i32], ptr @l, i64 0, i64 %indvars.iv117, !intel-tbaa !6
  store i32 7, ptr %arrayidx6, align 4, !tbaa !6
  %arrayidx8 = getelementptr inbounds [20 x i16], ptr @w, i64 0, i64 %indvars.iv117, !intel-tbaa !11
  store i16 1, ptr %arrayidx8, align 2, !tbaa !11
  %indvars.iv.next118 = add nuw nsw i64 %indvars.iv117, 1
  %exitcond119 = icmp eq i64 %indvars.iv.next118, 20
  br i1 %exitcond119, label %for.cond9.preheader, label %for.body

for.body17.lr.ph:                                 ; preds = %for.end77, %for.cond9.preheader
  %indvars.iv113 = phi i64 [ 2, %for.cond9.preheader ], [ %indvars.iv.next114, %for.end77 ]
  %indvars.iv = phi i64 [ 1, %for.cond9.preheader ], [ %indvars.iv.next, %for.end77 ]
  %arrayidx19 = getelementptr inbounds [20 x i32], ptr @x, i64 0, i64 %indvars.iv
  %3 = add nuw nsw i64 %indvars.iv, 7
  %arrayidx23 = getelementptr inbounds [20 x i32], ptr @mib, i64 0, i64 %3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %arrayidx42 = getelementptr inbounds [20 x i16], ptr @w, i64 0, i64 %indvars.iv.next
  %arrayidx55 = getelementptr inbounds [20 x i32], ptr @e, i64 0, i64 %indvars.iv
  %arrayidx59 = getelementptr inbounds [20 x i32], ptr @e, i64 0, i64 %indvars.iv
  %4 = select i1 %tobool56, ptr @d, ptr %arrayidx59
  %5 = load i32, ptr %arrayidx19, align 4, !tbaa !6
  %6 = trunc i64 %indvars.iv to i32
  %7 = add i32 %6, 6
  %add20 = add i32 %7, %5
  %8 = load i32, ptr @e, align 16, !tbaa !6
  %9 = load i32, ptr @d, align 16, !tbaa !6
  %arrayidx23.promoted = load i32, ptr %arrayidx23, align 4, !tbaa !6
  br label %for.body17

for.cond46.preheader:                             ; preds = %for.body17
  store i32 %add24, ptr %arrayidx23, align 4, !tbaa !6
  %arrayidx26.le = getelementptr inbounds [20 x i32], ptr @e, i64 0, i64 %indvars.iv105, !intel-tbaa !6
  %10 = load i32, ptr %arrayidx26.le, align 4, !tbaa !6
  %arrayidx29.le = getelementptr inbounds [20 x i32], ptr @l, i64 0, i64 %indvars.iv105, !intel-tbaa !6
  %11 = load i32, ptr %arrayidx29.le, align 4, !tbaa !6
  %conv33.le = zext i16 %14 to i32
  %add27 = add i32 %10, %2
  %sub = add i32 %add27, %8
  %conv = zext i16 %13 to i32
  %add32 = add i32 %11, %conv
  %add34 = add i32 %add32, %conv33.le
  %add37 = add nuw nsw i32 %conv33.le, 5
  %and = and i32 %add34, %add37
  %mul = mul i32 %and, %9
  %add38 = sub i32 %sub, %mul
  %12 = load i16, ptr @w, align 16, !tbaa !11
  %conv50 = zext i16 %12 to i32
  br label %for.body49

for.body17:                                       ; preds = %for.body17, %for.body17.lr.ph
  %add24125 = phi i32 [ %arrayidx23.promoted, %for.body17.lr.ph ], [ %add24, %for.body17 ]
  %indvars.iv105 = phi i64 [ %indvars.iv, %for.body17.lr.ph ], [ %indvars.iv.next106, %for.body17 ]
  %add24 = add i32 %add20, %add24125
  %arrayidx31 = getelementptr inbounds [20 x i16], ptr @w, i64 0, i64 %indvars.iv105, !intel-tbaa !11
  %13 = load i16, ptr %arrayidx31, align 2, !tbaa !11
  %14 = load i16, ptr @w, align 16, !tbaa !11
  store i16 0, ptr %arrayidx42, align 2, !tbaa !11
  %indvars.iv.next106 = add nuw nsw i64 %indvars.iv105, 1
  %cmp16 = icmp ult i64 %indvars.iv105, %indvars.iv
  br i1 %cmp16, label %for.body17, label %for.cond46.preheader

for.body49:                                       ; preds = %for.cond46.preheader, %cond.end67
  %indvars.iv107 = phi i64 [ 0, %for.cond46.preheader ], [ %indvars.iv.next108, %cond.end67 ]
  %arrayidx52 = getelementptr inbounds [20 x i32], ptr @d, i64 0, i64 %indvars.iv107, !intel-tbaa !6
  store i32 %conv50, ptr %arrayidx52, align 4, !tbaa !6
  %tobool = icmp eq i64 %indvars.iv107, 0
  br i1 %tobool, label %lor.lhs.false, label %cond.true65

lor.lhs.false:                                    ; preds = %for.body49
  br i1 %tobool53, label %cond.false, label %cond.end

cond.false:                                       ; preds = %lor.lhs.false
  %15 = load i32, ptr %arrayidx55, align 4, !tbaa !6
  br label %cond.end

cond.end:                                         ; preds = %lor.lhs.false, %cond.false
  %cond = phi i32 [ %15, %cond.false ], [ 0, %lor.lhs.false ]
  %cond62 = load i32, ptr %4, align 4, !tbaa !6
  %add63 = sub i32 0, %cond62
  %tobool64 = icmp eq i32 %cond, %add63
  br i1 %tobool64, label %cond.end67, label %cond.true65

cond.true65:                                      ; preds = %cond.end, %for.body49
  br label %cond.end67

cond.end67:                                       ; preds = %cond.end, %cond.true65
  %cond68 = phi i32 [ %1, %cond.true65 ], [ 0, %cond.end ]
  %add69 = add i32 %1, %cond68
  %arrayidx71 = getelementptr inbounds [20 x i32], ptr @e, i64 0, i64 %indvars.iv107, !intel-tbaa !6
  store i32 %add69, ptr %arrayidx71, align 4, !tbaa !6
  %indvars.iv.next108 = add nuw nsw i64 %indvars.iv107, 1
  %exitcond = icmp eq i64 %indvars.iv.next108, %indvars.iv113
  br i1 %exitcond, label %for.end77, label %for.body49

for.end77:                                        ; preds = %cond.end67
  %cmp10 = icmp ult i64 %indvars.iv.next, 10
  %indvars.iv.next114 = add nuw nsw i64 %indvars.iv113, 3
  br i1 %cmp10, label %for.body17.lr.ph, label %for.end80

for.end80:                                        ; preds = %for.end77
  store i32 %add38, ptr @ydv, align 4, !tbaa !2
  %16 = trunc i64 %indvars.iv.next108 to i32
  store i32 %16, ptr @n, align 4, !tbaa !2
  store i32 10, ptr @ucw, align 4, !tbaa !2
  ret i32 0
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA20_j", !3, i64 0}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA20_m", !10, i64 0}
!10 = !{!"long", !4, i64 0}
!11 = !{!12, !13, i64 0}
!12 = !{!"array@_ZTSA20_t", !13, i64 0}
!13 = !{!"short", !4, i64 0}

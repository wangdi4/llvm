; RUN: llc -x86-ciscization-helper-bb-inst-number-threshold=1 < %s -O3 -enable-intel-advanced-opts | FileCheck %s --check-prefixes=CHECK

; Tests for x86 ciscization helper

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1 x [9 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @fun(i32 %l1, i32 %l2, i32 %l3, i32 %u1, i32 %u2, i32 %u3) local_unnamed_addr #0 {
; CHECK-LABEL: fun:
; CHECK:       # %bb.0: # %entry
; Test for no block between BB1 and BB3 and between BB2 and BB3
; CHECK:    addl $-11, {{.*}}({{.*}})
; CHECK:    addl $-12, {{.*}}({{.*}})
; CHECK:    addl $12, {{.*}}({{.*}})
; Test for other blocks between BB1 and BB3
; CHECK:    addl $11, {{.*}}({{.*}})
entry:
  %cmp114 = icmp slt i32 %l1, %u1
  br i1 %cmp114, label %for.body.lr.ph, label %for.end78

for.body.lr.ph:                                   ; preds = %entry
  %cmp9112 = icmp sge i32 %l2, %u2
  %cmp24110 = icmp slt i32 %l3, %u3
  %0 = sext i32 %l2 to i64
  %1 = sext i32 %l3 to i64
  %2 = sext i32 %l1 to i64
  %wide.trip.count133 = sext i32 %u1 to i64
  %wide.trip.count = sext i32 %u2 to i64
  %wide.trip.count129 = sext i32 %u2 to i64
  %wide.trip.count125 = sext i32 %u3 to i64
  br label %for.body

for.body:                                         ; preds = %for.inc76, %for.body.lr.ph
  %indvars.iv131 = phi i64 [ %2, %for.body.lr.ph ], [ %indvars.iv.next132, %for.inc76 ]
  %arrayidx = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 0, i64 0, i64 %indvars.iv131
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp slt i32 %3, 1
  br i1 %cmp1, label %for.inc76, label %if.end

if.end:                                           ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 1, i64 0, i64 %indvars.iv131
  %4 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %sub = add nsw i32 %4, -10
  store i32 %sub, i32* %arrayidx3, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 7, i64 0, i64 %indvars.iv131
  %5 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %cmp6 = icmp sgt i32 %5, 0
  %brmerge = or i1 %cmp6, %cmp9112
  br i1 %brmerge, label %if.end72, label %for.body10.lr.ph

for.body10.lr.ph:                                 ; preds = %if.end
  br i1 %cmp24110, label %for.body10.us, label %for.body10

for.body10.us:                                    ; preds = %for.body10.lr.ph, %for.inc69.us
  %indvars.iv127 = phi i64 [ %indvars.iv.next128, %for.inc69.us ], [ %0, %for.body10.lr.ph ]
  %arrayidx12.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 2, i64 0, i64 %indvars.iv127
  %6 = load i32, i32* %arrayidx12.us, align 4, !tbaa !2
  %cmp13.us = icmp slt i32 %6, 1
  br i1 %cmp13.us, label %for.inc69.us, label %if.end15.us

if.end15.us:                                      ; preds = %for.body10.us
  %arrayidx17.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 3, i64 0, i64 %indvars.iv127
  %7 = load i32, i32* %arrayidx17.us, align 4, !tbaa !2
  %sub18.us = add nsw i32 %7, -11
  store i32 %sub18.us, i32* %arrayidx17.us, align 4, !tbaa !2
  %arrayidx20.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 6, i64 0, i64 %indvars.iv127
  %8 = load i32, i32* %arrayidx20.us, align 4, !tbaa !2
  %cmp21.us = icmp slt i32 %8, 1
  br i1 %cmp21.us, label %for.body25.us, label %if.else.us

if.else.us:                                       ; preds = %if.end15.us
  %arrayidx63.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 11, i64 0, i64 %indvars.iv127
  %9 = load i32, i32* %arrayidx63.us, align 4, !tbaa !2
  %inc64.us = add nsw i32 %9, 1
  store i32 %inc64.us, i32* %arrayidx63.us, align 4, !tbaa !2
  br label %if.end65.us

if.end65.us:                                      ; preds = %for.cond23.for.end_crit_edge.us, %if.else.us
  %10 = phi i32 [ %.pre135, %for.cond23.for.end_crit_edge.us ], [ %sub18.us, %if.else.us ]
  %add68.us = add nsw i32 %10, 11
  store i32 %add68.us, i32* %arrayidx17.us, align 4, !tbaa !2
  br label %for.inc69.us

for.body25.us:                                    ; preds = %if.end15.us, %for.inc.us
  %indvars.iv121 = phi i64 [ %indvars.iv.next122, %for.inc.us ], [ %1, %if.end15.us ]
  %arrayidx27.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 4, i64 0, i64 %indvars.iv121
  %11 = load i32, i32* %arrayidx27.us, align 4, !tbaa !2
  %cmp28.us = icmp slt i32 %11, 1
  br i1 %cmp28.us, label %for.inc.us, label %if.end30.us

if.end30.us:                                      ; preds = %for.body25.us
  %arrayidx32.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 5, i64 0, i64 %indvars.iv121
  %12 = load i32, i32* %arrayidx32.us, align 4, !tbaa !2
  %sub33.us = add nsw i32 %12, -12
  store i32 %sub33.us, i32* %arrayidx32.us, align 4, !tbaa !2
  %13 = sub nsw i64 %indvars.iv131, %indvars.iv121
  %14 = sub nsw i64 %indvars.iv127, %indvars.iv121
  %arrayidx39.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 0, i64 %13, i64 %14
  %15 = load i32, i32* %arrayidx39.us, align 4, !tbaa !2
  %tobool.us = icmp eq i32 %15, 0
  br i1 %tobool.us, label %if.end47.us, label %if.then40.us

if.then40.us:                                     ; preds = %if.end30.us
  %inc.us = add nsw i32 %15, 1
  store i32 %inc.us, i32* %arrayidx39.us, align 4, !tbaa !2
  %.pre = load i32, i32* %arrayidx32.us, align 4, !tbaa !2
  br label %if.end47.us

if.end47.us:                                      ; preds = %if.then40.us, %if.end30.us
  %16 = phi i32 [ %.pre, %if.then40.us ], [ %sub33.us, %if.end30.us ]
  %add.us = add nsw i32 %16, 12
  store i32 %add.us, i32* %arrayidx32.us, align 4, !tbaa !2
  %arrayidx51.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 8, i64 0, i64 %indvars.iv121
  %17 = load i32, i32* %arrayidx51.us, align 4, !tbaa !2
  %cmp52.us = icmp slt i32 %17, 1
  br i1 %cmp52.us, label %if.then53.us, label %for.inc.us

if.then53.us:                                     ; preds = %if.end47.us
  %inc56.us = add nsw i32 %17, 1
  store i32 %inc56.us, i32* %arrayidx51.us, align 4, !tbaa !2
  br label %for.inc.us

for.inc.us:                                       ; preds = %if.then53.us, %if.end47.us, %for.body25.us
  %indvars.iv.next122 = add nsw i64 %indvars.iv121, 1
  %exitcond126 = icmp eq i64 %indvars.iv.next122, %wide.trip.count125
  br i1 %exitcond126, label %for.cond23.for.end_crit_edge.us, label %for.body25.us

for.inc69.us:                                     ; preds = %if.end65.us, %for.body10.us
  %indvars.iv.next128 = add nsw i64 %indvars.iv127, 1
  %exitcond130 = icmp eq i64 %indvars.iv.next128, %wide.trip.count129
  br i1 %exitcond130, label %if.end72, label %for.body10.us

for.cond23.for.end_crit_edge.us:                  ; preds = %for.inc.us
  %arrayidx60.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 9, i64 0, i64 %indvars.iv127
  %18 = load i32, i32* %arrayidx60.us, align 4, !tbaa !2
  %inc61.us = add nsw i32 %18, 1
  store i32 %inc61.us, i32* %arrayidx60.us, align 4, !tbaa !2
  %.pre135 = load i32, i32* %arrayidx17.us, align 4, !tbaa !2
  br label %if.end65.us

for.body10:                                       ; preds = %for.body10.lr.ph, %for.inc69
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc69 ], [ %0, %for.body10.lr.ph ]
  %arrayidx12 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 2, i64 0, i64 %indvars.iv
  %19 = load i32, i32* %arrayidx12, align 4, !tbaa !2
  %cmp13 = icmp slt i32 %19, 1
  br i1 %cmp13, label %for.inc69, label %if.end15

if.end15:                                         ; preds = %for.body10
  %arrayidx17 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 3, i64 0, i64 %indvars.iv
  %20 = load i32, i32* %arrayidx17, align 4, !tbaa !2
  %sub18 = add nsw i32 %20, -11
  store i32 %sub18, i32* %arrayidx17, align 4, !tbaa !2
  %arrayidx20 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 6, i64 0, i64 %indvars.iv
  %21 = load i32, i32* %arrayidx20, align 4, !tbaa !2
  %cmp21 = icmp slt i32 %21, 1
  %arrayidx60 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 9, i64 0, i64 %indvars.iv
  %arrayidx63 = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 11, i64 0, i64 %indvars.iv
  %22 = select i1 %cmp21, i32* %arrayidx60, i32* %arrayidx63
  %23 = select i1 %cmp21, i32* %arrayidx60, i32* %arrayidx63
  %24 = load i32, i32* %22, align 4, !tbaa !2
  %inc64 = add nsw i32 %24, 1
  store i32 %inc64, i32* %23, align 4, !tbaa !2
  store i32 %20, i32* %arrayidx17, align 4, !tbaa !2
  br label %for.inc69

for.inc69:                                        ; preds = %for.body10, %if.end15
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %if.end72, label %for.body10

if.end72:                                         ; preds = %for.inc69, %for.inc69.us, %if.end
  %25 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %add75 = add nsw i32 %25, 10
  store i32 %add75, i32* %arrayidx3, align 4, !tbaa !2
  br label %for.inc76

for.inc76:                                        ; preds = %for.body, %if.end72
  %indvars.iv.next132 = add nsw i64 %indvars.iv131, 1
  %exitcond134 = icmp eq i64 %indvars.iv.next132, %wide.trip.count133
  br i1 %exitcond134, label %for.end78, label %for.body

for.end78:                                        ; preds = %for.inc76, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" "contains-rec-pro-clone" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

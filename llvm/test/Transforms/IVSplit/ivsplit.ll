; RUN: opt < %s -iv-split -iv-split-loop-depth-threshold=2 -S | FileCheck %s
;
; This test case checks IV: %indvars.iv45 of outer Loop: for.body.us is being
; spilled before inner Loop: for.body6.us and reloaded at inner Loop for use:
; if.end11.us and exit BB for following use: for.cond4.for.end_crit_edge.us.
; It also checks merging orignal and reloade IV.
;
; Source code for the test
; int a[][9];
; void ivsplit_test(int l1, int l2, int u1, int u2) {
;  int i1, i2;
;  for(i1 = l1; i1 < u1; i1++) {
;    if(a[0][i1] <= 0)
;      continue;
;    a[1][i1] -= 10;
;    for(i2 = l2; i2 < u2; i2++) {
;      if(a[4][i2] <= 0)
;        continue;
;      a[5][i2] -= 12;
;      a[6][i1-i2] -= 13;
;      a[5][i2] += 12;
;    }
;    a[1][i1] += 10;
;  }
; }


target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local local_unnamed_addr global [1 x [9 x i32]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @ivsplit_test(i32 %l1, i32 %l2, i32 %u1, i32 %u2) local_unnamed_addr #0 {
; CHECK-LABEL: @ivsplit_test(
; CHECK:entry:
; CHECK-NEXT: %iv-split-var = alloca i64
; IV is being spilled before inner Loop
; CHECK: if.end.us:
; CHECK-NEXT: store i64 %indvars.iv45, i64* %iv-split-var
; IV is being reloaded at inner Loop for use
; CHECK: if.end11.us:
; CHECK: %iv-inner-reload-var = load i64, i64* %iv-split-var
; CHECK: sub nsw i64 %iv-inner-reload-var, %indvars.iv
; IV is being reloaded at exit BB for following use
; CHECK: for.cond4.for.end_crit_edge.us.split:
; CHECK-NEXT: %iv-reload-var = load i64, i64* %iv-split-var
; Merge orignal and reloaded IV for IV increment
; CHECK: for.inc24.us:
; CHECK-NEXT: %iv-split-phi = phi i64 [ %iv-reload-var, %for.cond4.for.end_crit_edge.us.split ], [ %indvars.iv45, %for.body.us ]
; CHECK: %indvars.iv.next46 = add nsw i64 %iv-split-phi, 1
entry:
  %cmp41 = icmp slt i32 %l1, %u1
  %cmp539 = icmp slt i32 %l2, %u2
  %or.cond = and i1 %cmp41, %cmp539
  br i1 %or.cond, label %for.body.us.preheader, label %for.end26

for.body.us.preheader:                            ; preds = %entry
  %0 = sext i32 %l2 to i64
  %1 = sext i32 %l1 to i64
  %wide.trip.count = sext i32 %u2 to i64
  %wide.trip.count47 = sext i32 %u1 to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.inc24.us, %for.body.us.preheader
  %indvars.iv45 = phi i64 [ %1, %for.body.us.preheader ], [ %indvars.iv.next46, %for.inc24.us ]
  %arrayidx.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 0, i64 0, i64 %indvars.iv45
  %2 = load i32, i32* %arrayidx.us, align 4, !tbaa !2
  %cmp1.us = icmp slt i32 %2, 1
  br i1 %cmp1.us, label %for.inc24.us, label %if.end.us

if.end.us:                                        ; preds = %for.body.us
  %arrayidx3.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 1, i64 0, i64 %indvars.iv45
  %3 = load i32, i32* %arrayidx3.us, align 4, !tbaa !2
  %sub.us = add nsw i32 %3, -10
  store i32 %sub.us, i32* %arrayidx3.us, align 4, !tbaa !2
  br label %for.body6.us

for.body6.us:                                     ; preds = %for.inc.us, %if.end.us
  %indvars.iv = phi i64 [ %0, %if.end.us ], [ %indvars.iv.next, %for.inc.us ]
  %arrayidx8.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 4, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx8.us, align 4, !tbaa !2
  %cmp9.us = icmp slt i32 %4, 1
  br i1 %cmp9.us, label %for.inc.us, label %if.end11.us

if.end11.us:                                      ; preds = %for.body6.us
  %arrayidx13.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 5, i64 0, i64 %indvars.iv
  %5 = load i32, i32* %arrayidx13.us, align 4, !tbaa !2
  %sub14.us = add nsw i32 %5, -12
  store i32 %sub14.us, i32* %arrayidx13.us, align 4, !tbaa !2
  %6 = sub nsw i64 %indvars.iv45, %indvars.iv
  %arrayidx17.us = getelementptr inbounds [1 x [9 x i32]], [1 x [9 x i32]]* @a, i64 6, i64 0, i64 %6
  %7 = load i32, i32* %arrayidx17.us, align 4, !tbaa !2
  %sub18.us = add nsw i32 %7, -13
  store i32 %sub18.us, i32* %arrayidx17.us, align 4, !tbaa !2
  %8 = load i32, i32* %arrayidx13.us, align 4, !tbaa !2
  %add.us = add nsw i32 %8, 12
  store i32 %add.us, i32* %arrayidx13.us, align 4, !tbaa !2
  br label %for.inc.us

for.inc.us:                                       ; preds = %if.end11.us, %for.body6.us
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond4.for.end_crit_edge.us, label %for.body6.us

for.cond4.for.end_crit_edge.us:                   ; preds = %for.inc.us
  %9 = load i32, i32* %arrayidx3.us, align 4, !tbaa !2
  %add23.us = add nsw i32 %9, 10
  store i32 %add23.us, i32* %arrayidx3.us, align 4, !tbaa !2
  br label %for.inc24.us

for.inc24.us:                                     ; preds = %for.cond4.for.end_crit_edge.us, %for.body.us
  %indvars.iv.next46 = add nsw i64 %indvars.iv45, 1
  %exitcond48 = icmp eq i64 %indvars.iv.next46, %wide.trip.count47
  br i1 %exitcond48, label %for.end26, label %for.body.us

for.end26:                                        ; preds = %for.inc24.us, %entry
  ret void
}

attributes #0 = { norecurse nounwind uwtable "contains-rec-pro-clone" "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

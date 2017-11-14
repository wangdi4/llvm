;
;   for (j=0; j <  n; j++) {
;        for (i=0; i <  n; i++) {
;            B[j][i]  = i + j; }]
;
;    for (j=0; j <  n; j++) {
;        for (i=0; i <  n; i++) {
;            B[j][i]  += i * j;   }} 
;
; RUN:  opt < %s -hir-ssa-deconstruction -hir-create-function-level-region |  opt -hir-create-function-level-region  -hir-dd-test-assume-loop-fusion -hir-dd-analysis  -hir-dd-analysis-verify=Region -analyze  | FileCheck %s 
; CHECK-DAG:   (@B)[0][i1][i2] --> (@B)[0][i1][i2] FLOW (= =)

; ModuleID = 'fuse4.c'
source_filename = "fuse4.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [1000 x [1000 x float]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub1(i32 %n) local_unnamed_addr #0 {
entry:
  %cmp55 = icmp sgt i32 %n, 0
  br i1 %cmp55, label %for.body3.lr.ph.preheader, label %for.end28

for.body3.lr.ph.preheader:                        ; preds = %entry
  %wide.trip.count66 = sext i32 %n to i64
  %wide.trip.count70 = sext i32 %n to i64
  br label %for.body3.lr.ph

for.body3.lr.ph:                                  ; preds = %for.body3.lr.ph.preheader, %for.inc6
  %indvars.iv68 = phi i64 [ %indvars.iv.next69, %for.inc6 ], [ 0, %for.body3.lr.ph.preheader ]
  br label %for.body3

for.cond9.preheader:                              ; preds = %for.inc6
  %cmp1050 = icmp sgt i32 %n, 0
  br i1 %cmp1050, label %for.body16.lr.ph.preheader, label %for.end28

for.body16.lr.ph.preheader:                       ; preds = %for.cond9.preheader
  %wide.trip.count = sext i32 %n to i64
  %wide.trip.count61 = sext i32 %n to i64
  br label %for.body16.lr.ph

for.body3:                                        ; preds = %for.body3, %for.body3.lr.ph
  %indvars.iv63 = phi i64 [ 0, %for.body3.lr.ph ], [ %indvars.iv.next64, %for.body3 ]
  %0 = add nuw nsw i64 %indvars.iv63, %indvars.iv68
  %1 = trunc i64 %0 to i32
  %conv = sitofp i32 %1 to float
  %arrayidx5 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %indvars.iv68, i64 %indvars.iv63
  store float %conv, float* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next64 = add nuw nsw i64 %indvars.iv63, 1
  %exitcond67 = icmp eq i64 %indvars.iv.next64, %wide.trip.count66
  br i1 %exitcond67, label %for.inc6, label %for.body3

for.inc6:                                         ; preds = %for.body3
  %indvars.iv.next69 = add nuw nsw i64 %indvars.iv68, 1
  %exitcond71 = icmp eq i64 %indvars.iv.next69, %wide.trip.count70
  br i1 %exitcond71, label %for.cond9.preheader, label %for.body3.lr.ph

for.body16.lr.ph:                                 ; preds = %for.body16.lr.ph.preheader, %for.inc26
  %indvars.iv59 = phi i64 [ %indvars.iv.next60, %for.inc26 ], [ 0, %for.body16.lr.ph.preheader ]
  br label %for.body16

for.body16:                                       ; preds = %for.body16, %for.body16.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body16.lr.ph ], [ %indvars.iv.next, %for.body16 ]
  %2 = mul nuw nsw i64 %indvars.iv, %indvars.iv59
  %3 = trunc i64 %2 to i32
  %conv17 = sitofp i32 %3 to float
  %arrayidx21 = getelementptr inbounds [1000 x [1000 x float]], [1000 x [1000 x float]]* @B, i64 0, i64 %indvars.iv59, i64 %indvars.iv
  %4 = load float, float* %arrayidx21, align 4, !tbaa !1
  %add22 = fadd float %4, %conv17
  store float %add22, float* %arrayidx21, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.inc26, label %for.body16

for.inc26:                                        ; preds = %for.body16
  %indvars.iv.next60 = add nuw nsw i64 %indvars.iv59, 1
  %exitcond62 = icmp eq i64 %indvars.iv.next60, %wide.trip.count61
  br i1 %exitcond62, label %for.end28.loopexit, label %for.body16.lr.ph

for.end28.loopexit:                               ; preds = %for.inc26
  br label %for.end28

for.end28:                                        ; preds = %for.end28.loopexit, %entry, %for.cond9.preheader
  ret void
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

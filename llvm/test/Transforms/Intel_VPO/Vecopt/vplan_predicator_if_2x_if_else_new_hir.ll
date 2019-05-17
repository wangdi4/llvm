; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -disable-vplan-codegen -debug -S 2>&1 | FileCheck %s
; REQUIRES: asserts

; Verify New VPlan predicator: if with two nested if-else statements.
; Copy of vplan_predicator_if_2x_if_else_new.ll modified for HIR.

; CHECK: VPlan IR for: Predicator: After predication
; CHECK:   i32* [[BAddr:%.*]] = getelementptr inbounds i32* %b
; CHECK:   i32 [[BVal:%.*]] = load i32* [[BAddr]]
; CHECK:   i1 [[OuterCmp:%.*]] = icmp i32 [[BVal]] i32 0
; CHECK:   i1 [[BPred1:%.*]] = block-predicate i1 [[OuterCmp]]
; CHECK:   i32* [[AAddr:%.*]] = getelementptr inbounds i32* %a
; CHECK:   i32 [[AVal:%.*]] = load i32* [[AAddr]]
; CHECK:   i1 [[InnerCmp1:%.*]] = icmp i32 [[AVal]] i32 0
; TODO - RPO traversal is traversing the else successor before the then successor
; this needs to be fixed.
; CHECK:   i1 [[InnerCmp1Not:%.*]] = not i1 [[InnerCmp1]]
; CHECK:   i1 [[ElseAnd1:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp1Not]]
; CHECK:   i1 [[Else1Pred:%.*]] = block-predicate i1 [[ElseAnd1]]
; CHECK:   i32 [[Add1:%.*]] = add i32 [[AVal]] i32 5
; CHECK:   i32* [[AAddr2:%.*]] = getelementptr inbounds i32* %a
; CHECK:   store i32 [[Add1]] i32* [[AAddr2]]
; CHECK:   i1 [[IfAnd1:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp1]]
; CHECK:   i1 [[Then1Pred:%.*]] = block-predicate i1 [[IfAnd1]]
; CHECK:   i32 [[Mul1:%.*]] = mul i32 [[BVal]] i32 5
; CHECK:   i32* [[BAddr2:%.*]] = getelementptr inbounds i32* %b
; CHECK:   store i32 [[Mul1]] i32* [[BAddr2]]
; CHECK:   i1 [[BPred2:%.*]] = block-predicate i1 [[OuterCmp]]
; CHECK:   i32* [[CAddr:%.*]] = getelementptr inbounds i32* %c
; CHECK:   i32 [[CVal:%.*]] = load i32* [[CAddr]]
; CHECK:   i32 [[Mul2:%.*]] = mul i32 [[CVal]] i32 %N
; CHECK:   i32* [[CAddr2:%.*]] = getelementptr inbounds i32* %c
; CHECK:   store i32 [[Mul2]] i32* [[CAddr2]]
; CHECK:   i32 [[Mul2_1:%.*]] = mul i32 [[CVal]] i32 %N
; CHECK:   i1 [[InnerCmp2:%.*]] = icmp i32 [[Mul2_1]] i32 0
; CHECK:   i1 [[InnerCmp2Not:%.*]] = not i1 [[InnerCmp2]]
; CHECK:   i1 [[ElseAnd2:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp2Not]]
; CHECK:   i1 [[Else2Pred:%.*]] = block-predicate i1 [[ElseAnd2]]
; CHECK:   i32 [[Mul3_1:%.*]] = mul i32 {{.*}}
; CHECK:   i32 [[Mul3_2:%.*]] = mul i32 {{.*}}
; CHECK:   i32* [[BAddr3:%.*]] = getelementptr inbounds i32* %b
; CHECK:   store i32 [[Mul3_2]] i32* [[BAddr3]]
; CHECK:   i1 [[ThenAnd2:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp2]]
; CHECK:   i1 [[Then2Pred:%.*]] = block-predicate i1 [[ThenAnd2]]
; CHECK:   i32 [[MulNeg:%.*]] = mul i32 {{.*}} i32 -1
; CHECK:   i32 [[Sub1:%.*]] = add i32 {{.*}} [[MulNeg]]
; CHECK:   i32* [[AAddr3:%.*]] = getelementptr inbounds i32* %a
; CHECK:   store i32 [[Sub1]] i32* [[AAddr3]]

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
;   for (i = 0; i < 300; i++) {
;
;     if (b[i] > 0) {
;       if (a[i] > 0)
;         b[i] = b[i] * 5;
;       else
;         a[i] = a[i] + 5;
;
;       c[i] = c[i] * N;
;
;       if (c[i] > 0)
;         a[i] = c[i] - a[i];
;       else
;         b[i] = a[i] * c[i];
;     }
;   }
; }
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind uwtable
define dso_local void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  %cmp4 = icmp sgt i32 %1, 0
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4, !tbaa !2
  br label %if.end

if.else:                                          ; preds = %if.then
  %add = add nsw i32 %1, 5
  store i32 %add, i32* %arrayidx3, align 4, !tbaa !2
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  %arrayidx15 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx15, align 4, !tbaa !2
  %mul16 = mul nsw i32 %2, %N
  store i32 %mul16, i32* %arrayidx15, align 4, !tbaa !2
  %cmp21 = icmp sgt i32 %mul16, 0
  %3 = load i32, i32* %arrayidx3, align 4, !tbaa !2
  br i1 %cmp21, label %if.then22, label %if.else29

if.then22:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul16, %3
  store i32 %sub, i32* %arrayidx3, align 4, !tbaa !2
  br label %for.inc

if.else29:                                        ; preds = %if.end
  %mul34 = mul nsw i32 %3, %mul16
  store i32 %mul34, i32* %arrayidx, align 4, !tbaa !2
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.else29, %if.then22
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) dev.8.x.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

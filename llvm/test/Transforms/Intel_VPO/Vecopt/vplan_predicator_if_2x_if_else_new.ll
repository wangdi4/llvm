; RUN: opt %s -VPlanDriver -enable-new-vplan-predicator -disable-vplan-codegen -debug -S 2>&1 | FileCheck %s
; REQUIRES: asserts

; Verify New VPlan predicator: if with two nested if-else statements.
; 1. icx test.c -o test_noopt.ll -fopenmp -Qoption,c,-fintel-openmp -O0 -restrict -S -emit-llvm
; 2. opt test_noopt.ll -O2 -debug-pass=Arguments
; 3. opt test_noopt.ll -S -o pred_if_else.ll -loopopt=false (+ all the flags from -O2 from #2, but -VPODirectiveCleanup and -loop-unroll)

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
; CHECK:   store i32 [[Add1]] i32* [[AAddr]]
; CHECK:   i1 [[IfAnd1:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp1]]
; CHECK:   i1 [[Then1Pred:%.*]] = block-predicate i1 [[IfAnd1]]
; CHECK:   i32 [[Mul1:%.*]] = mul i32 [[BVal]] i32 5
; CHECK:   store i32 [[Mul1]] i32* [[BAddr]]
; CHECK:   i1 [[BPred2:%.*]] = block-predicate i1 [[OuterCmp]]
; CHECK:   i32* [[CAddr:%.*]] = getelementptr inbounds i32* %c
; CHECK:   i32 [[CVal:%.*]] = load i32* [[CAddr]]
; CHECK:   i32 [[Mul2:%.*]] = mul i32 [[CVal]] i32 %N
; CHECK:   store i32 [[Mul2]] i32* [[CAddr]]
; CHECK:   i1 [[InnerCmp2:%.*]] = icmp i32 [[Mul2]] i32 0
; CHECK:   i1 [[InnerCmp2Not:%.*]] = not i1 [[InnerCmp2]]
; CHECK:   i1 [[ElseAnd2:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp2Not]]
; CHECK:   i1 [[Else2Pred:%.*]] = block-predicate i1 [[ElseAnd2]]
; CHECK:   i32 [[Mul3:%.*]] = mul i32 {{.*}}
; CHECK:   store i32 [[Mul3]] i32* [[BAddr]]
; CHECK:   i1 [[ThenAnd2:%.*]] = and i1 [[OuterCmp]] i1 [[InnerCmp2]]
; CHECK:   i1 [[Then2Pred:%.*]] = block-predicate i1 [[ThenAnd2]]
; CHECK:   i32 [[Sub1:%.*]] = sub i32 {{.*}}
; CHECK:   store i32 [[Sub1]] i32* [[AAddr]]

; void foo(int * restrict a, int * restrict b, int * restrict c, int N)
; {
;   int i;
; #pragma omp simd
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


; ModuleID = 'pred_if_2x_if_else_noopt.ll'
source_filename = "pred_if_2x_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture %c, i32 %N) local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %0, 0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %1, 0
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %0, 5
  store i32 %mul, i32* %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %if.then
  %add = add nsw i32 %1, 5
  store i32 %add, i32* %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  %2 = phi i32 [ %add, %if.else ], [ %1, %if.then5 ]
  %arrayidx15 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx15, align 4
  %mul16 = mul nsw i32 %3, %N
  store i32 %mul16, i32* %arrayidx15, align 4
  %cmp21 = icmp sgt i32 %mul16, 0
  br i1 %cmp21, label %if.then22, label %if.else29

if.then22:                                        ; preds = %if.end
  %sub = sub nsw i32 %mul16, %2
  store i32 %sub, i32* %arrayidx3, align 4
  br label %for.inc

if.else29:                                        ; preds = %if.end
  %mul34 = mul nsw i32 %2, %mul16
  store i32 %mul34, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.else29, %if.then22
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret void
}

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

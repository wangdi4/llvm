; This test represents invoking VLS-grouping analysis from the VPO vectorizer

; Original C/C++ source
;
; int a[300];
; void foo(int N) {
;   int i;
;   for (i=0;i<300;i+=3){
;     a[i]++;
;     a[i+1]++;
;     a[i+2]++;
;   }
; }

; TODO: Only runs in debug mode
; TODO: Not use O2 for testing.
; REQUIRES: asserts
; RUN: opt < %s -O2 -S -loopopt -disable-hir-complete-unroll -debug -enable-vect-vls=1 -vplan-driver-hir=0 2>&1 | FileCheck %s
; XFAIL: *
; TO-DO : The test case fails upon removal of AVR Code. Analyze and fix it so that it works for VPlanDriverHIR
;
; CHECK: Printing Groups- Total Groups 2
; CHECK-DAG: AccessMask(per byte, R to L): 111111111111
; CHECK-DAG: AccessMask(per byte, R to L): 111111111111

; ModuleID = 'vectvls2.c'
source_filename = "vectvls2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [300 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %N) #0 {
entry:
  %N.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 300
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [300 x i32], [300 x i32]* @a, i64 0, i64 %idxprom
  %2 = load i32, i32* %arrayidx, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* %arrayidx, align 4
  %3 = load i32, i32* %i, align 4
  %add = add nsw i32 %3, 1
  %idxprom1 = sext i32 %add to i64
  %arrayidx2 = getelementptr inbounds [300 x i32], [300 x i32]* @a, i64 0, i64 %idxprom1
  %4 = load i32, i32* %arrayidx2, align 4
  %inc3 = add nsw i32 %4, 1
  store i32 %inc3, i32* %arrayidx2, align 4
  %5 = load i32, i32* %i, align 4
  %add4 = add nsw i32 %5, 2
  %idxprom5 = sext i32 %add4 to i64
  %arrayidx6 = getelementptr inbounds [300 x i32], [300 x i32]* @a, i64 0, i64 %idxprom5
  %6 = load i32, i32* %arrayidx6, align 4
  %inc7 = add nsw i32 %6, 1
  store i32 %inc7, i32* %arrayidx6, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, i32* %i, align 4
  %add8 = add nsw i32 %7, 3
  store i32 %add8, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (branches/vpo 10155)"}

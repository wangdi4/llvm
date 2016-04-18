; Check that after ztt extraction the CE definition levels are fine. The HIR verifier would assert on this.
; REQUIRES: asserts

; RUN: opt < %s -hir-ssa-deconstruction -hir-general-unroll -print-after=hir-general-unroll -hir-details 2>&1 | FileCheck %s 

; CHECK: BEGIN REGION { modified }

; CHECK: %call = @foo1(i1);
; CHECK-NEXT: <LVAL-REG> NON-LINEAR i{{[0-9]+}} %call

; CHECK: if (0 < %call)
; CHECK-NEXT: <RVAL-REG> NON-LINEAR i{{[0-9]+}} %call

; CHECK: %tgu = (%call)/u8;
; CHECK-NEXT: <LVAL-REG> NON-LINEAR i{{[0-9]+}} %tgu
; CHECK-NEXT: <RVAL-REG> NON-LINEAR i{{[0-9]+}} (%call)/u{{[0-9]+}}
; CHECK-NEXT: <BLOB> NON-LINEAR i{{[0-9]+}} %call

; CHECK: DO i{{[0-9]+}} i2 = 0, %tgu + -1, 1   <DO_LOOP>
; CHECK-NEXT: <RVAL-REG> LINEAR i{{[0-9]+}} %tgu + -1{def@1}
; CHECK-NEXT: <BLOB> LINEAR i{{[0-9]+}} %tgu{def@1}
; CHECK-NEXT: <ZTT-REG> LINEAR i{{[0-9]+}} %tgu{def@1}

; int a[10];
; extern int foo1(int);
;
; int foo() {
; int i,j,n;
; for (i=0;i<10;++i) {
;   n = foo1(i);
;   for (j=0;j<n;++j) {
;     a[i] = n + j;
;   }
; }
; 
; return a[0];
; }

; ModuleID = 'extract_ztt_def_level.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [10 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @foo() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc4
  %i.03 = phi i32 [ 0, %entry ], [ %inc5, %for.inc4 ]
  %call = call i32 @foo1(i32 %i.03)
  %cmp21 = icmp slt i32 0, %call
  br i1 %cmp21, label %for.body3.lr.ph, label %for.end

for.body3.lr.ph:                                  ; preds = %for.body
  br label %for.body3

for.body3:                                        ; preds = %for.body3.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body3.lr.ph ], [ %inc, %for.inc ]
  %add = add nsw i32 %call, %j.02
  %idxprom = sext i32 %i.03 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* @a, i64 0, i64 %idxprom
  store i32 %add, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %call
  br i1 %cmp2, label %for.body3, label %for.cond1.for.end_crit_edge

for.cond1.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond1.for.end_crit_edge, %for.body
  br label %for.inc4

for.inc4:                                         ; preds = %for.end
  %inc5 = add nsw i32 %i.03, 1
  %cmp = icmp slt i32 %inc5, 10
  br i1 %cmp, label %for.body, label %for.end6

for.end6:                                         ; preds = %for.inc4
  %0 = load i32, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @a, i64 0, i64 0), align 16
  ret i32 %0
}

declare i32 @foo1(i32) #1



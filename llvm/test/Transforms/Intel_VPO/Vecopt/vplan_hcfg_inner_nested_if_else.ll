; RUN: opt %s -VPlanDriver -disable-vplan-predicator -disable-vplan-codegen -debug -debug-only=VPlanDriver 2>&1 | FileCheck %s
; REQUIRES: asserts

; Verify the HCFG construction of an inner loop with a nested if-else statement.

; CHECK: Vectorization Plan\nVD: Initial VPlan for VF=
; CHECK: subgraph cluster_region
; CHECK: subgraph cluster_loop
; CHECK: subgraph cluster_region

; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: }{{[[:space:]]}}

; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop
; CHECK: }{{[[:space:]]}}
; CHECK-NOT: subgraph cluster_region
; CHECK-NOT: subgraph cluster_loop


; void foo(int * restrict a, int * restrict b, int * restrict c, int N, int M, int K)
; {
;   int i;
; #pragma omp simd
;   for (i = 0; i < 300; i++) {
;     if (a[i] == 0) 
;       if (b[i] > 5)
;         b[i] = b[i] * 5;
;       else
;         b[i] = c[i] + b[i];
;     else
;       if (a[i] > 100)
;         a[i] = a[i] - b[i];
;       else
;         a[i] = a[i] * c[i];
;   }
; }


; ModuleID = 'pred_nested_if_else_noopt.ll'
source_filename = "pred_nested_if_else.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define void @foo(i32* noalias nocapture %a, i32* noalias nocapture %b, i32* noalias nocapture readonly %c, i32 %N, i32 %M, i32 %K) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp eq i32 %0, 0
  br i1 %cmp1, label %if.then, label %if.else16

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx3, align 4
  %cmp4 = icmp sgt i32 %1, 5
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %mul = mul nsw i32 %1, 5
  store i32 %mul, i32* %arrayidx3, align 4
  br label %for.inc

if.else:                                          ; preds = %if.then
  %arrayidx11 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx11, align 4
  %add = add nsw i32 %2, %1
  store i32 %add, i32* %arrayidx3, align 4
  br label %for.inc

if.else16:                                        ; preds = %for.body
  %cmp19 = icmp sgt i32 %0, 100
  br i1 %cmp19, label %if.then20, label %if.else27

if.then20:                                        ; preds = %if.else16
  %arrayidx24 = getelementptr inbounds i32, i32* %b, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx24, align 4
  %sub = sub nsw i32 %0, %3
  store i32 %sub, i32* %arrayidx, align 4
  br label %for.inc

if.else27:                                        ; preds = %if.else16
  %arrayidx31 = getelementptr inbounds i32, i32* %c, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx31, align 4
  %mul32 = mul nsw i32 %4, %0
  store i32 %mul32, i32* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then5, %if.else27, %if.then20
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 300
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { noinline nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }


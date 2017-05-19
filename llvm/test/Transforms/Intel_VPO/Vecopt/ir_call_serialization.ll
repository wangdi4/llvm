; RUN: opt -S -VPlanDriver -disable-vplan-subregions < %s | FileCheck %s
;void baz(int i);
;
;void foo(int *arr) {
;#pragma omp simd
;  for (int i = 0; i < 100; i++)
;    if (arr[i])
;      baz(i);
;}

; Predicated call serialization
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: foo
; CHECK:  %[[Cond0:.*]] = icmp eq i1 %[[Pred0:.*]], true
; CHECK:  br i1 %[[Cond0]]
; CHECK:  %[[Cond1:.*]] = icmp eq i1 %[[Pred1:.*]], true
; CHECK:  br i1 %[[Cond1]]

define void @foo(i32* nocapture readonly %arr) {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %arr, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %1 = trunc i64 %indvars.iv to i32
  tail call void @baz(i32 %1) #3
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end
  ret void
}

declare void @llvm.intel.directive(metadata)
declare void @baz(i32 )

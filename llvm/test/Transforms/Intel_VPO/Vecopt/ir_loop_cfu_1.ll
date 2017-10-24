; REQUIRES: asserts
; RUN: opt -VPlanDriver -vplan-loop-cfu -disable-vplan-predicator -disable-vplan-subregions -disable-vplan-codegen -debug -S < %s 2>&1 | FileCheck %s

; Test the transformation of the innermost loop non-uniform control flow to uniform control flow.

;long A[100][100];
;void foo(long N, long *lb, long *ub) {
;  long i, j;
;#pragma omp simd
;  for(i=0;i<N;i++) {
;    for (j=lb[i]; j<ub[i]; j++) { <-- non-uniform trip count
;      A[j][i] = i+j;
;    }
;  }
;}


; CHECK: [[BB_2:BB[0-9]+]] [label = "{[[BB_2]] | Vectorize VPInstIR:\n  %i.022 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]\n  %arrayidx = getelementptr inbounds i64, i64* %lb, i64 %i.022\n  %0 = load i64, i64* %arrayidx, align 8\n  %arrayidx2 = getelementptr inbounds i64, i64* %ub, i64 %i.022\n  %1 = load i64, i64* %arrayidx2, align 8 | IfNotAllZero: \n  %cmp319 = icmp slt i64 %0, %1}"]
; CHECK: [[BB_2]] -> [[BB_17:BB[0-9]+]] [ label=""]
; CHECK: [[BB_17]] [label = "{[[BB_17]]}"]
; CHECK: [[BB_17]] -> [[BB_4:BB[0-9]+]] [ label="Branch If Not All Zero Recipe" lhead=[[cluster_loop_15:cluster_loop[0-9]+]]]
; CHECK: [[BB_17]] -> [[BB_5:BB[0-9]+]] [ label="!Branch If Not All Zero Recipe"]
; CHECK: [[BB_4]] [label = "{[[BB_4]]}"]
; CHECK: [[BB_4]] -> [[BB_6:BB[0-9]+]] [ label=""]
; CHECK: [[BB_6:BB[0-9]+]] [label = "{[[BB_6]] | Vectorize VPInstIR:\n  %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ] | MaskGeneration: \n  %cmp319 = icmp slt i64 %0, %1 &   %cmp3 = icmp slt i64 %inc, %2 | Non-uniform branch condition: MaskGeneration: \n  %cmp319 = icmp slt i64 %0, %1 &   %cmp3 = icmp slt i64 %inc, %2}"]
; CHECK: [[BB_6:BB[0-9]+]] -> [[BB_19:BB[0-9]+]] [ label=""]
; CHECK: [[BB_19]] [label = "{[[BB_19]]}"]
; CHECK: [[BB_19]] -> [[BB_14:BB[0-9]+]] [ label="Non-Uniform Cond Bit Recipe"]
; CHECK: [[BB_19]] -> [[BB_16:BB[0-9]+]] [ label="!Non-Uniform Cond Bit Recipe"]
; CHECK: [[BB_14]] [label = "{[[BB_14]] | Vectorize VPInstIR:\n  %add = add nsw i64 %j.020, %i.022\n  %arrayidx6 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @A, i64 0, i64 %j.020, i64 %i.022\n  store i64 %add, i64* %arrayidx6, align 8}"]
; CHECK: [[BB_14]] -> [[BB_16:BB[0-9]+]] [ label=""]
; CHECK: [[BB_16]] [label = "{[[BB_16]] | Vectorize VPInstIR:\n  %inc = add nsw i64 %j.020, 1\n  %2 = load i64, i64* %arrayidx2, align 8 | IfNotAllZero: \n  %cmp3 = icmp slt i64 %inc, %2}"]
; CHECK: [[BB_16]] -> [[BB_20:BB[0-9]+]] [ label=""]
; CHECK: [[BB_20]] [label = "{[[BB_20]]}"]
; CHECK: [[BB_20]] -> [[BB_6:BB[0-9]+]] [ label="Branch If Not All Zero Recipe"]
; CHECK: [[BB_20]] -> [[BB_8:BB[0-9]+]] [ label="!Branch If Not All Zero Recipe"]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common local_unnamed_addr global [100 x [100 x i64]] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @foo(i64 %N, i64* nocapture readonly %lb, i64* nocapture readonly %ub) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp21 = icmp sgt i64 %N, 0
  br i1 %cmp21, label %for.body.preheader, label %for.end9

for.body.preheader:                               ; preds = %DIR.QUAL.LIST.END.2
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc7
  %i.022 = phi i64 [ %inc8, %for.inc7 ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64, i64* %lb, i64 %i.022
  %0 = load i64, i64* %arrayidx, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %ub, i64 %i.022
  %1 = load i64, i64* %arrayidx2, align 8
  %cmp319 = icmp slt i64 %0, %1
  br i1 %cmp319, label %for.body4.preheader, label %for.inc7

for.body4.preheader:                              ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %for.body4.preheader, %for.body4
  %j.020 = phi i64 [ %inc, %for.body4 ], [ %0, %for.body4.preheader ]
  %add = add nsw i64 %j.020, %i.022
  %arrayidx6 = getelementptr inbounds [100 x [100 x i64]], [100 x [100 x i64]]* @A, i64 0, i64 %j.020, i64 %i.022
  store i64 %add, i64* %arrayidx6, align 8
  %inc = add nsw i64 %j.020, 1
  %2 = load i64, i64* %arrayidx2, align 8
  %cmp3 = icmp slt i64 %inc, %2
  br i1 %cmp3, label %for.body4, label %for.inc7.loopexit

for.inc7.loopexit:                                ; preds = %for.body4
  br label %for.inc7

for.inc7:                                         ; preds = %for.inc7.loopexit, %for.body
  %inc8 = add nuw nsw i64 %i.022, 1
  %exitcond = icmp eq i64 %inc8, %N
  br i1 %exitcond, label %for.end9.loopexit, label %for.body

for.end9.loopexit:                                ; preds = %for.inc7
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  br label %for.end9

for.end9:                                         ; preds = %DIR.QUAL.LIST.END.3, %DIR.QUAL.LIST.END.2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

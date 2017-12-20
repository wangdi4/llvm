; REQUIRES: asserts
; RUN: opt -VPlanDriver -vplan-loop-cfu -disable-vplan-predicator -disable-vplan-subregions -disable-vplan-codegen -debug -S < %s 2>&1 | FileCheck %s

; Test the transformation of the innermost loop non-uniform control flow to uniform control flow.

;void foo(float **a, int m, int n, int k) {
;  int i, j;
;  for(i=0; i<m; i++) {
;    j = k;
;    while (j<=n && i%2 == 0) { <--- non-uniform i expr with uniform upper bound check
;      if (a[i][j] == 0.0f)
;        a[i][j] = i + j;
;      else
;        a[j][i] = i * j;
;      j = j + 1;
;    }
;  }
;}


; CHECK: [[BB_2:BB[0-9]+]] [label = "{[[BB_2]] | Vectorize VPInstIR:\n  %indvars.iv41 = phi i64 [ 0, %while.cond.preheader.lr.ph ], [ %indvars.iv.next42, %for.inc ]\n  %2 = trunc i64 %indvars.iv41 to i32\n  %rem32 = and i32 %2, 1\n  %cmp2 = icmp eq i32 %rem32, 0 | IfNotAllZero: \n  %or.cond34 = and i1 %cmp2, %cmp133}"]
; CHECK: [[BB_2]] -> [[BB_21:BB[0-9]+]] [ label=""]
; CHECK: [[BB_21]] [label = "{[[BB_21]]}"]
; CHECK: [[BB_21]] -> [[BB_4:BB[0-9]+]] [ label="Branch If Not All Zero Recipe" l
; CHECK: [[BB_21]] -> [[BB_5:BB[0-9]+]] [ label="!Branch If Not All Zero Recipe"]
; CHECK: [[BB_6:BB[0-9]+]] [label = "{[[BB_6]] | Vectorize VPInstIR:\n  %indvars.iv = phi i64 [ %0, %while.body.lr.ph ], [ %indvars.iv.next, %if.end ]\n  %j.035 = phi i32 [ %k, %while.body.lr.ph ], [ %add15, %if.end ] | MaskGeneration: \n  %or.cond34 = and i1 %cmp2, %cmp133 &   %cmp1 = icmp slt i64 %indvars.iv, %1 | Non-uniform branch condition: MaskGeneration: \n  %or.cond34 = and i1 %cmp2, %cmp133 &   %cmp1 = icmp slt i64 %indvars.iv, %1}"]
; CHECK: [[BB_6]] -> [[BB_23:BB[0-9]+]] [ label=""]
; CHECK: [[BB_23]] [label = "{[[BB_23]]}"]
; CHECK: [[BB_23]] -> [[BB_18:BB[0-9]+]] [ label="Non-Uniform Cond Bit Recipe"]
; CHECK: [[BB_23]] -> [[BB_20:BB[0-9]+]] [ label="!Non-Uniform Cond Bit Recipe"]
; CHECK: [[BB_18]] [label = "{[[BB_18]] | Vectorize VPInstIR:\n  %arrayidx4 = getelementptr inbounds float, float* %3, i64 %indvars.iv\n  %4 = load float, float* %arrayidx4, align 4 | [[UniformCBR_4:UniformCBR[0-9]+]]: %cmp5 = fcmp oeq float %4, 0.000000e+00}"]
; CHECK: [[BB_18]] -> [[BB_8:BB[0-9]+]] [ label="[[UniformCBR_4]]"]
; CHECK: [[BB_18]] -> [[BB_9:BB[0-9]+]] [ label="![[UniformCBR_4]]"]
; CHECK: [[BB_8]] [label = "{[[BB_8]] | Vectorize VPInstIR:\n  %5 = add nsw i64 %indvars.iv, %indvars.iv41\n  %6 = trunc i64 %5 to i32}"]
; CHECK: [[BB_8]] -> [[BB_10:BB[0-9]+]] [ label=""]
; CHECK: [[BB_10]] [label = "{[[BB_10]] | Vectorize VPInstIR:\n  %.sink16 = phi i64 [ %indvars.iv41, %if.else ], [ %indvars.iv, %if.then ]\n  %.sink = phi float* [ %7, %if.else ], [ %3, %if.then ]\n  %conv10.sink.in = phi i32 [ %mul, %if.else ], [ %6, %if.then ]\n  %conv10.sink = sitofp i32 %conv10.sink.in to float\n  %sext = shl i64 %.sink16, 32\n  %idxprom13 = ashr exact i64 %sext, 32\n  %arrayidx14 = getelementptr inbounds float, float* %.sink, i64 %idxprom13\n  store float %conv10.sink, float* %arrayidx14, align 4}"]
; CHECK: [[BB_10]] -> [[BB_20]] [ label=""]
; CHECK: [[BB_20]] [label = "{[[BB_20]] | Vectorize VPInstIR:\n  %indvars.iv.next = add nsw i64 %indvars.iv, 1\n  %add15 = add nsw i32 %j.035, 1 | IfNotAllZero: \n  %cmp1 = icmp slt i64 %indvars.iv, %1}"]
; CHECK: [[BB_20]] -> [[BB_24:BB[0-9]+]] [ label=""]
; CHECK: [[BB_24]] [label = "{[[BB_24]]}"]
; CHECK: [[BB_24]] -> [[BB_6:BB[0-9]+]] [ label="Branch If Not All Zero Recipe"]
; CHECK: [[BB_24]] -> [[BB_12:BB[0-9]+]] [ label="!Branch If Not All Zero Recipe"]
; CHECK: [[BB_12]] [label = "{[[BB_12]]}"]
; CHECK: [[BB_9]] [label = "{[[BB_9]] | Vectorize VPInstIR:\n  %mul = mul nsw i32 %j.035, %2\n  %arrayidx12 = getelementptr inbounds float*, float** %a, i64 %indvars.iv\n  %7 = load float*, float** %arrayidx12, align 8}"]
; CHECK: [[BB_9]] -> [[BB_10]] [ label=""]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(float** nocapture readonly %a, i32 %m, i32 %n, i32 %k) local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp36 = icmp sgt i32 %m, 0
  br i1 %cmp36, label %while.cond.preheader.lr.ph, label %for.end

while.cond.preheader.lr.ph:                       ; preds = %DIR.QUAL.LIST.END.2
  %cmp133 = icmp sle i32 %k, %n
  %0 = sext i32 %k to i64
  %1 = sext i32 %n to i64
  %wide.trip.count = zext i32 %m to i64
  br label %while.cond.preheader

while.cond.preheader:                             ; preds = %for.inc, %while.cond.preheader.lr.ph
  %indvars.iv41 = phi i64 [ 0, %while.cond.preheader.lr.ph ], [ %indvars.iv.next42, %for.inc ]
  %2 = trunc i64 %indvars.iv41 to i32
  %rem32 = and i32 %2, 1
  %cmp2 = icmp eq i32 %rem32, 0
  %or.cond34 = and i1 %cmp2, %cmp133
  br i1 %or.cond34, label %while.body.lr.ph, label %for.inc

while.body.lr.ph:                                 ; preds = %while.cond.preheader
  %arrayidx = getelementptr inbounds float*, float** %a, i64 %indvars.iv41
  %3 = load float*, float** %arrayidx, align 8
  br label %while.body

while.body:                                       ; preds = %while.body.lr.ph, %if.end
  %indvars.iv = phi i64 [ %0, %while.body.lr.ph ], [ %indvars.iv.next, %if.end ]
  %j.035 = phi i32 [ %k, %while.body.lr.ph ], [ %add15, %if.end ]
  %arrayidx4 = getelementptr inbounds float, float* %3, i64 %indvars.iv
  %4 = load float, float* %arrayidx4, align 4
  %cmp5 = fcmp oeq float %4, 0.000000e+00
  br i1 %cmp5, label %if.then, label %if.else

if.then:                                          ; preds = %while.body
  %5 = add nsw i64 %indvars.iv, %indvars.iv41
  %6 = trunc i64 %5 to i32
  br label %if.end

if.else:                                          ; preds = %while.body
  %mul = mul nsw i32 %j.035, %2
  %arrayidx12 = getelementptr inbounds float*, float** %a, i64 %indvars.iv
  %7 = load float*, float** %arrayidx12, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %.sink16 = phi i64 [ %indvars.iv41, %if.else ], [ %indvars.iv, %if.then ]
  %.sink = phi float* [ %7, %if.else ], [ %3, %if.then ]
  %conv10.sink.in = phi i32 [ %mul, %if.else ], [ %6, %if.then ]
  %conv10.sink = sitofp i32 %conv10.sink.in to float
  %sext = shl i64 %.sink16, 32
  %idxprom13 = ashr exact i64 %sext, 32
  %arrayidx14 = getelementptr inbounds float, float* %.sink, i64 %idxprom13
  store float %conv10.sink, float* %arrayidx14, align 4
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %add15 = add nsw i32 %j.035, 1
  %cmp1 = icmp slt i64 %indvars.iv, %1
  br i1 %cmp1, label %while.body, label %for.inc.loopexit

for.inc.loopexit:                                 ; preds = %if.end
  br label %for.inc

for.inc:                                          ; preds = %for.inc.loopexit, %while.cond.preheader
  %indvars.iv.next42 = add nuw nsw i64 %indvars.iv41, 1
  %exitcond = icmp eq i64 %indvars.iv.next42, %wide.trip.count
  br i1 %exitcond, label %for.end.loopexit, label %while.cond.preheader

for.end.loopexit:                                 ; preds = %for.inc
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %DIR.QUAL.LIST.END.2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

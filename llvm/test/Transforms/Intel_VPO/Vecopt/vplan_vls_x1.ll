; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -vplan-use-da-unit-stride-accesses=false -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 1) {
;    ary[i + 0] += 7;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 4
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 4
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #2 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_PHI:%.*]]
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast i32* [[MM_VECTORGEP_0]] to <4 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <4 x i32>, <4 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[TMP0:%.*]] = add nsw <4 x i32> [[GROUPLOAD]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[MM_VECTORGEP_01:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR2:%.*]] = bitcast i32* [[MM_VECTORGEP_01]] to <4 x i32>*
; CHECK-NEXT:    store <4 x i32> [[TMP0]], <4 x i32>* [[GROUPPTR2]], align 4
; CHECK:         br i1 %{{.*}}, label %{{.*}}, label %vector.body
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add7 = add nsw i32 %0, 7
  store i32 %add7, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

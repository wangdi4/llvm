; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 1) {
;    t0 = ary[i + 0] + 7;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #2 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i64 [ 0, [[VECTOR_PH:%.*]] ], [ [[INDEX_NEXT:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[VEC_IND:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, [[VECTOR_PH]] ], [ [[VEC_IND_NEXT:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_IND]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP0:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP0]], <4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    [[TMP1:%.*]] = add nuw nsw <4 x i64> [[VEC_IND]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[TMP2:%.*]] = icmp ult <4 x i64> [[TMP1]], <i64 1024, i64 1024, i64 1024, i64 1024>
; CHECK-NEXT:    [[TMP3:%.*]] = extractelement <4 x i1> [[TMP2]], i32 0
; CHECK-NEXT:    [[INDEX_NEXT]] = add i64 [[INDEX]], 4
; CHECK-NEXT:    [[TMP4:%.*]] = icmp eq i64 [[INDEX_NEXT]], 1024
; CHECK-NEXT:    [[VEC_IND_NEXT]] = add <4 x i64> [[VEC_IND]], <i64 4, i64 4, i64 4, i64 4>
; CHECK-NEXT:    br i1 [[TMP4]], label [[VPLANNEDBB:%.*]], label [[VECTOR_BODY]]
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

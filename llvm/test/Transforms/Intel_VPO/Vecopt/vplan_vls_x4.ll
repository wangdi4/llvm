; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 4) {
;    t0 = ary[i + 0] + 7;
;    t1 = ary[i + 1] + 11;
;    t2 = ary[i + 2] + 12;
;    t3 = ary[i + 3] + 61;
;    ary[i + 0] = t0;
;    ary[i + 1] = t1;
;    ary[i + 2] = t2;
;    ary[i + 3] = t3;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #5 <4 x 32> SStore
; CHECK-NEXT:   #6 <4 x 32> SStore
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_PHI:%.*]]
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast i32* [[MM_VECTORGEP_0]] to <16 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <16 x i32>, <16 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
; CHECK-NEXT:    [[TMP0:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP1]]
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <4 x i32> <i32 1, i32 5, i32 9, i32 13>
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE2]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP3:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP3]]
; CHECK-NEXT:    [[GROUPSHUFFLE4:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <4 x i32> <i32 2, i32 6, i32 10, i32 14>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE4]], <i32 12, i32 12, i32 12, i32 12>
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 3, i64 3, i64 3, i64 3>
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP5]]
; CHECK-NEXT:    [[GROUPSHUFFLE6:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <4 x i32> <i32 3, i32 7, i32 11, i32 15>
; CHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE6]], <i32 61, i32 61, i32 61, i32 61>
; CHECK-NEXT:    [[TMP7:%.*]] = shufflevector <4 x i32> [[TMP0]], <4 x i32> [[TMP2]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[TMP8:%.*]] = shufflevector <4 x i32> [[TMP4]], <4 x i32> [[TMP6]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[TMP9:%.*]] = shufflevector <8 x i32> [[TMP7]], <8 x i32> [[TMP8]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[GROUPSHUFFLE7:%.*]] = shufflevector <16 x i32> [[TMP9]], <16 x i32> undef, <16 x i32> <i32 0, i32 4, i32 8, i32 12, i32 1, i32 5, i32 9, i32 13, i32 2, i32 6, i32 10, i32 14, i32 3, i32 7, i32 11, i32 15>
; CHECK-NEXT:    [[MM_VECTORGEP_08:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR9:%.*]] = bitcast i32* [[MM_VECTORGEP_08]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[GROUPSHUFFLE7]], <16 x i32>* [[GROUPPTR9]], align 4
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
  %1 = add nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %ary, i64 %1
  %2 = load i32, i32* %arrayidx4, align 4
  %add11 = add nsw i32 %2, 11
  %3 = add nsw i64 %indvars.iv, 2
  %arrayidx8 = getelementptr inbounds i32, i32* %ary, i64 %3
  %4 = load i32, i32* %arrayidx8, align 4
  %add12 = add nsw i32 %4, 12
  %5 = add nsw i64 %indvars.iv, 3
  %arrayidx12 = getelementptr inbounds i32, i32* %ary, i64 %5
  %6 = load i32, i32* %arrayidx12, align 4
  %add61 = add nsw i32 %6, 61
  store i32 %add7, i32* %arrayidx, align 4
  store i32 %add11, i32* %arrayidx4, align 4
  store i32 %add12, i32* %arrayidx8, align 4
  store i32 %add61, i32* %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

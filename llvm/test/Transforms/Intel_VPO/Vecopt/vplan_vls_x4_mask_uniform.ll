; RUN: opt -S -VPlanDriver -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary, i32 %param) {
;  for (i = 0; i < 1024; i += 4) {
;    if (param & 1) {
;      t0 = ary[i + 0] + 7;
;      t1 = ary[i + 1] + 11;
;      t2 = ary[i + 2] + 12;
;      t3 = ary[i + 3] + 61;
;      ary[i + 0] = t0;
;      ary[i + 1] = t1;
;      ary[i + 2] = t2;
;      ary[i + 3] = t3;
;    }
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:      Vector Length(in bytes): 64
; CHECK-NEXT:      AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:      AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:      Vector Length(in bytes): 64
; CHECK-NEXT:      AccType: SStore, Stride (in bytes): 16
; CHECK-NEXT:      AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #5 <4 x 32> SStore
; CHECK-NEXT:   #6 <4 x 32> SStore
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:         br i1 [[TMP1:%.*]], label [[VPLANNEDBB40:%.*]], label [[VPLANNEDBB50:%.*]]
; CHECK:       VPlannedBB5:
; CHECK-NEXT:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT0:%.*]], <4 x i64> [[VEC_PHI0:%.*]]
; CHECK-NEXT:    [[MM_VECTORGEP_00:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP0]], i64 0
; CHECK-NEXT:    [[GROUPPTR0:%.*]] = bitcast i32* [[MM_VECTORGEP_00]] to <16 x i32>*
; CHECK-NEXT:    [[GROUPLOAD0:%.*]] = load <16 x i32>, <16 x i32>* [[GROUPPTR0]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE0:%.*]] = shufflevector <16 x i32> [[GROUPLOAD0]], <16 x i32> undef, <4 x i32> <i32 0, i32 4, i32 8, i32 12>
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE0]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP3:%.*]] = or <4 x i64> [[VEC_PHI0]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP60:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT0]], <4 x i64> [[TMP3]]
; CHECK-NEXT:    [[GROUPSHUFFLE70:%.*]] = shufflevector <16 x i32> [[GROUPLOAD0]], <16 x i32> undef, <4 x i32> <i32 1, i32 5, i32 9, i32 13>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE70]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP5:%.*]] = or <4 x i64> [[VEC_PHI0]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP80:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT0]], <4 x i64> [[TMP5]]
; CHECK-NEXT:    [[GROUPSHUFFLE90:%.*]] = shufflevector <16 x i32> [[GROUPLOAD0]], <16 x i32> undef, <4 x i32> <i32 2, i32 6, i32 10, i32 14>
; CHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE90]], <i32 12, i32 12, i32 12, i32 12>
; CHECK-NEXT:    [[TMP7:%.*]] = or <4 x i64> [[VEC_PHI0]], <i64 3, i64 3, i64 3, i64 3>
; CHECK-NEXT:    [[MM_VECTORGEP100:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT0]], <4 x i64> [[TMP7]]
; CHECK-NEXT:    [[GROUPSHUFFLE110:%.*]] = shufflevector <16 x i32> [[GROUPLOAD0]], <16 x i32> undef, <4 x i32> <i32 3, i32 7, i32 11, i32 15>
; CHECK-NEXT:    [[TMP8:%.*]] = add nsw <4 x i32> [[GROUPSHUFFLE110]], <i32 61, i32 61, i32 61, i32 61>
; CHECK-NEXT:    [[TMP9:%.*]] = shufflevector <4 x i32> [[TMP2]], <4 x i32> [[TMP4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[TMP10:%.*]] = shufflevector <4 x i32> [[TMP6]], <4 x i32> [[TMP8]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[TMP11:%.*]] = shufflevector <8 x i32> [[TMP9]], <8 x i32> [[TMP10]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[GROUPSHUFFLE120:%.*]] = shufflevector <16 x i32> [[TMP11]], <16 x i32> undef, <16 x i32> <i32 0, i32 4, i32 8, i32 12, i32 1, i32 5, i32 9, i32 13, i32 2, i32 6, i32 10, i32 14, i32 3, i32 7, i32 11, i32 15>
; CHECK-NEXT:    [[MM_VECTORGEP_0130:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP0]], i64 0
; CHECK-NEXT:    [[GROUPPTR140:%.*]] = bitcast i32* [[MM_VECTORGEP_0130]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[GROUPSHUFFLE120]], <16 x i32>* [[GROUPPTR140]], align 4
; CHECK-NEXT:    br label [[VPLANNEDBB40]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB4:
; CHECK:         br i1 [[TMP15:%.*]], label [[VECTOR_BODY0:%.*]], label [[VPLANNEDBB150:%.*]]
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %and = and i32 %param, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add7 = add nsw i32 %0, 7
  %1 = or i64 %indvars.iv, 1
  %arrayidx6 = getelementptr inbounds i32, i32* %ary, i64 %1
  %2 = load i32, i32* %arrayidx6, align 4
  %add11 = add nsw i32 %2, 11
  %3 = or i64 %indvars.iv, 2
  %arrayidx10 = getelementptr inbounds i32, i32* %ary, i64 %3
  %4 = load i32, i32* %arrayidx10, align 4
  %add12 = add nsw i32 %4, 12
  %5 = or i64 %indvars.iv, 3
  %arrayidx14 = getelementptr inbounds i32, i32* %ary, i64 %5
  %6 = load i32, i32* %arrayidx14, align 4
  %add61 = add nsw i32 %6, 61
  store i32 %add7, i32* %arrayidx, align 4
  store i32 %add11, i32* %arrayidx6, align 4
  store i32 %add12, i32* %arrayidx10, align 4
  store i32 %add61, i32* %arrayidx14, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

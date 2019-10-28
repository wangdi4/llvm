; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(<2 x i32>* nocapture %src, <2 x i32>* nocapture %dst) {
;  typedef int32_t v2i32 __attribute__((vector_size(8)));
;  v2i32 *ary, t0, t1, t2;
;  for (i = 0; i < 1024; i += 3) {
;    if (dst[i][0] & 1) {
;      t0 = src[i + 0] + 7;
;      t1 = src[i + 1] + 11;
;      t2 = src[i + 2] + 12;
;      dst[i + 0] = t0;
;      dst[i + 1] = t1;
;      dst[i + 2] = t2;
;    }
;  }
;
; CHECK:       Printing Groups- Total Groups 3
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 24
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 24
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111
; CHECK-NEXT:   #2 <4 x 64> SLoad
; CHECK-NEXT:   #3 <4 x 64> SLoad
; CHECK-NEXT:   #4 <4 x 64> SLoad
; CHECK-NEXT:  Group#3
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 24
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111
; CHECK-NEXT:   #5 <4 x 64> SStore
; CHECK-NEXT:   #6 <4 x 64> SStore
; CHECK-NEXT:   #7 <4 x 64> SStore
;
; CHECK:       vector.body:
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_PHI:%.*]]
; CHECK-NEXT:    [[TMP0:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP]] to <4 x i32*>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[TMP0]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP1:%.*]] = and <4 x i32> [[WIDE_MASKED_GATHER]], <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[TMP2:%.*]] = icmp eq <4 x i32> [[TMP1]], zeroinitializer
; CHECK-NEXT:    [[TMP3:%.*]] = xor <4 x i1> [[TMP2]], <i1 true, i1 true, i1 true, i1 true>
; CHECK-NEXT:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT2:%.*]], <4 x i64> [[VEC_PHI]]
; CHECK-NEXT:    [[REPLICATEDMASKELTS_:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[TMP4:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP3]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_:%.*]] = shufflevector <4 x i32*> [[TMP4]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER4:%.*]] = call <8 x i32> @llvm.masked.gather.v8i32.v8p0i32(<8 x i32*> [[ELEMBASEPTR_]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_]], <8 x i32> undef)
; CHECK-NEXT:    [[TMP5:%.*]] = add <8 x i32> [[WIDE_MASKED_GATHER4]], <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP6:%.*]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT2]], <4 x i64> [[TMP6]]
; CHECK-NEXT:    [[REPLICATEDMASKELTS_6:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[TMP7:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP5]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_7:%.*]] = shufflevector <4 x i32*> [[TMP7]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_8:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_7]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER9:%.*]] = call <8 x i32> @llvm.masked.gather.v8i32.v8p0i32(<8 x i32*> [[ELEMBASEPTR_8]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_6]], <8 x i32> undef)
; CHECK-NEXT:    [[TMP8:%.*]] = add <8 x i32> [[WIDE_MASKED_GATHER9]], <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP9:%.*]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP10:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT2]], <4 x i64> [[TMP9]]
; CHECK-NEXT:    [[REPLICATEDMASKELTS_11:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[TMP10:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP10]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_12:%.*]] = shufflevector <4 x i32*> [[TMP10]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_13:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_12]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER14:%.*]] = call <8 x i32> @llvm.masked.gather.v8i32.v8p0i32(<8 x i32*> [[ELEMBASEPTR_13]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_11]], <8 x i32> undef)
; CHECK-NEXT:    [[TMP11:%.*]] = add <8 x i32> [[WIDE_MASKED_GATHER14]], <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
; CHECK-NEXT:    [[TMP12:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_15:%.*]] = shufflevector <4 x i32*> [[TMP12]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_16:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_15]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[REPLICATEDMASKELTS_17:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v8i32.v8p0i32(<8 x i32> [[TMP5]], <8 x i32*> [[ELEMBASEPTR_16]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_17]])
; CHECK-NEXT:    [[MM_VECTORGEP18:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP6]]
; CHECK-NEXT:    [[TMP13:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP18]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_19:%.*]] = shufflevector <4 x i32*> [[TMP13]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_20:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_19]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[REPLICATEDMASKELTS_21:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v8i32.v8p0i32(<8 x i32> [[TMP8]], <8 x i32*> [[ELEMBASEPTR_20]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_21]])
; CHECK-NEXT:    [[MM_VECTORGEP22:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP9]]
; CHECK-NEXT:    [[TMP14:%.*]] = bitcast <4 x <2 x i32>*> [[MM_VECTORGEP22]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_23:%.*]] = shufflevector <4 x i32*> [[TMP14]], <4 x i32*> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_24:%.*]] = getelementptr i32, <8 x i32*> [[VECBASEPTR_23]], <8 x i64> <i64 0, i64 1, i64 0, i64 1, i64 0, i64 1, i64 0, i64 1>
; CHECK-NEXT:    [[REPLICATEDMASKELTS_25:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v8i32.v8p0i32(<8 x i32> [[TMP11]], <8 x i32*> [[ELEMBASEPTR_24]], i32 4, <8 x i1> [[REPLICATEDMASKELTS_25]])
; CHECK:         br i1 %{{.*}}, label %{{.*}}, label %vector.body
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]

  ; if (dst[i][0] & 1) {
  %dst_0 = getelementptr inbounds <2 x i32>, <2 x i32>* %dst, i64 %indvars.iv
  %dst_0_0 = bitcast <2 x i32>* %dst_0 to i32*
  %dst_0_0.val = load i32, i32* %dst_0_0, align 4
  %and = and i32 %dst_0_0.val, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  ;   t0 = src[i + 0] + 7;
  %src_0 = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %indvars.iv
  %src_0.val = load <2 x i32>, <2 x i32>* %src_0, align 4
  %t0 = add <2 x i32> %src_0.val, <i32 7, i32 7>

  ;   t1 = src[i + 0] + 11;
  %i1 = add nsw nuw i64 %indvars.iv, 1
  %src_1 = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %i1
  %src_1.val = load <2 x i32>, <2 x i32>* %src_1, align 4
  %t1 = add <2 x i32> %src_1.val, <i32 11, i32 11>

  ;   t2 = src[i + 0] + 12;
  %i2 = add nsw nuw i64 %indvars.iv, 2
  %src_2 = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %i2
  %src_2.val = load <2 x i32>, <2 x i32>* %src_2, align 4
  %t2 = add <2 x i32> %src_2.val, <i32 12, i32 12>

  ;   dst[i + 0] = t0;
  store <2 x i32> %t0, <2 x i32>* %dst_0, align 4

  ;   dst[i + 1] = t1;
  %dst_1 = getelementptr inbounds <2 x i32>, <2 x i32>* %dst, i64 %i1
  store <2 x i32> %t1, <2 x i32>* %dst_1, align 4

  ;   dst[i + 2] = t2;
  %dst_2 = getelementptr inbounds <2 x i32>, <2 x i32>* %dst, i64 %i2
  store <2 x i32> %t2, <2 x i32>* %dst_2, align 4

  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

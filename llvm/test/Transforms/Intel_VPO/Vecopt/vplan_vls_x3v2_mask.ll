; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; The test is supposed to check that OptVLS works correctly in masked basic
; blocks. It is checked that a masked group-wide load is generated with the
; correct access mask.

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
; CHECK-NEXT:    [[MM_VECTORGEP3_0:%.*]] = extractelement <4 x <2 x i32>*> [[MM_VECTORGEP3]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast <2 x i32>* [[MM_VECTORGEP3_0]] to <24 x i32>*
; CHECK-NEXT:    [[GROUPLOADMASK:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <24 x i32> <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = call <24 x i32> @llvm.masked.load.v24i32.p0v24i32(<24 x i32>* [[GROUPPTR]], i32 4, <24 x i1> [[GROUPLOADMASK]], <24 x i32> undef)
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <24 x i32> [[GROUPLOAD]], <24 x i32> undef, <8 x i32> <i32 0, i32 1, i32 6, i32 7, i32 12, i32 13, i32 18, i32 19>
; CHECK-NEXT:    [[TMP4:%.*]] = add <8 x i32> [[GROUPSHUFFLE]], <i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP5:%.*]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP4:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT2]], <4 x i64> [[TMP5]]
; CHECK-NEXT:    [[GROUPSHUFFLE5:%.*]] = shufflevector <24 x i32> [[GROUPLOAD]], <24 x i32> undef, <8 x i32> <i32 2, i32 3, i32 8, i32 9, i32 14, i32 15, i32 20, i32 21>
; CHECK-NEXT:    [[TMP6:%.*]] = add <8 x i32> [[GROUPSHUFFLE5]], <i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP7:%.*]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT2]], <4 x i64> [[TMP7]]
; CHECK-NEXT:    [[GROUPSHUFFLE7:%.*]] = shufflevector <24 x i32> [[GROUPLOAD]], <24 x i32> undef, <8 x i32> <i32 4, i32 5, i32 10, i32 11, i32 16, i32 17, i32 22, i32 23>
; CHECK-NEXT:    [[TMP8:%.*]] = add <8 x i32> [[GROUPSHUFFLE7]], <i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12, i32 12>
; CHECK-NEXT:    [[MM_VECTORGEP8:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP5]]
; CHECK-NEXT:    [[MM_VECTORGEP9:%.*]] = getelementptr inbounds <2 x i32>, <4 x <2 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP7]]
; CHECK-NEXT:    [[TMP9:%.*]] = shufflevector <8 x i32> [[TMP4]], <8 x i32> [[TMP6]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[TMP10:%.*]] = shufflevector <8 x i32> [[TMP8]], <8 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP11:%.*]] = shufflevector <16 x i32> [[TMP9]], <16 x i32> [[TMP10]], <24 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23>
; CHECK-NEXT:    [[GROUPSHUFFLE10:%.*]] = shufflevector <24 x i32> [[TMP11]], <24 x i32> undef, <24 x i32> <i32 0, i32 1, i32 8, i32 9, i32 16, i32 17, i32 2, i32 3, i32 10, i32 11, i32 18, i32 19, i32 4, i32 5, i32 12, i32 13, i32 20, i32 21, i32 6, i32 7, i32 14, i32 15, i32 22, i32 23>
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x <2 x i32>*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR11:%.*]] = bitcast <2 x i32>* [[MM_VECTORGEP_0]] to <24 x i32>*
; CHECK-NEXT:    [[GROUPSTOREMASK:%.*]] = shufflevector <4 x i1> [[TMP3]], <4 x i1> undef, <24 x i32> <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    call void @llvm.masked.store.v24i32.p0v24i32(<24 x i32> [[GROUPSHUFFLE10]], <24 x i32>* [[GROUPPTR11]], i32 4, <24 x i1> [[GROUPSTOREMASK]])
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

  ;   t1 = src[i + 1] + 11;
  %i1 = add nsw nuw i64 %indvars.iv, 1
  %src_1 = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %i1
  %src_1.val = load <2 x i32>, <2 x i32>* %src_1, align 4
  %t1 = add <2 x i32> %src_1.val, <i32 11, i32 11>

  ;   t2 = src[i + 2] + 12;
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

; RUN: opt -S -vplan-vec -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 4) {
;    t0 = ary[i + 3] + 11;
;    t1 = ary[i + 0] + 22;
;    t2 = ary[i + 2] + 33;
;    t3 = ary[i + 0] + 44;
;    t4 = ary[i + 1] + 55;
;    t5 = ary[i + 2] + 66;
;    ary[i + 1] = t0;
;    ary[i + 2] = t1;
;    ary[i + 3] = t2;
;    ary[i + 0] = t3;
;    ary[i + 2] = t4;
;    ary[i + 1] = t5;
;  }
;
; CHECK:       Printing Groups- Total Groups 4
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #5 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 111100001111
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:   #6 <4 x 32> SLoad
; CHECK-NEXT:  Group#3
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #10 <4 x 32> SStore
; CHECK-NEXT:   #12 <4 x 32> SStore
; CHECK-NEXT:   #11 <4 x 32> SStore
; CHECK-NEXT:   #9 <4 x 32> SStore
; CHECK-NEXT:  Group#4
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 11111111
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
;
; CHECK-LABEL: @foo(
; CHECK:         [[TMP0:%.*]] = add nsw <4 x i64> [[VEC_PHI:%.*]], <i64 3, i64 3, i64 3, i64 3>
; CHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i64> [[TMP0]], i32 0
; CHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds i32, i32* [[ARY:%.*]], i64 [[DOTEXTRACT_0_]]
; CHECK-NEXT:    [[SCALAR_GEP4:%.*]] = getelementptr i32, i32* [[SCALAR_GEP]], i64 -3
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast i32* [[SCALAR_GEP4]] to <16 x i32>*
; CHECK-NEXT:    [[VLS_LOAD:%.*]] = load <16 x i32>, <16 x i32>* [[TMP1]], align 4
; CHECK-NEXT:    [[VP_L1:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 0, i32 4, i32 8, i32 12>
; CHECK-NEXT:    [[VP_L4:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 1, i32 5, i32 9, i32 13>
; CHECK-NEXT:    [[VP_L2:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 2, i32 6, i32 10, i32 14>
; CHECK-NEXT:    [[VP_L0:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 3, i32 7, i32 11, i32 15>
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <4 x i32> [[VP_L0]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[SCALAR_GEP5:%.*]] = getelementptr inbounds i32, i32* [[ARY]], i64 [[UNI_PHI3:%.*]]
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_PHI]]
; CHECK-NEXT:    [[TMP3:%.*]] = add nsw <4 x i32> [[VP_L1]], <i32 22, i32 22, i32 22, i32 22>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP4]]
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i32> [[VP_L2]], <i32 33, i32 33, i32 33, i32 33>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> poison)
; CHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER]], <i32 44, i32 44, i32 44, i32 44>
; CHECK-NEXT:    [[TMP7:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP7:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP7]]
; CHECK-NEXT:    [[TMP8:%.*]] = add nsw <4 x i32> [[VP_L4]], <i32 55, i32 55, i32 55, i32 55>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER8:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP6]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> poison)
; CHECK-NEXT:    [[TMP9:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER8]], <i32 66, i32 66, i32 66, i32 66>
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP2]], <4 x i32*> [[MM_VECTORGEP7]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP3]], <4 x i32*> [[MM_VECTORGEP6]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    [[EXTENDED_:%.*]] = shufflevector <4 x i32> [[TMP6]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP10:%.*]] = shufflevector <16 x i32> undef, <16 x i32> [[EXTENDED_]], <16 x i32> <i32 16, i32 1, i32 2, i32 3, i32 17, i32 5, i32 6, i32 7, i32 18, i32 9, i32 10, i32 11, i32 19, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[EXTENDED_9:%.*]] = shufflevector <4 x i32> [[TMP9]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP11:%.*]] = shufflevector <16 x i32> [[TMP10]], <16 x i32> [[EXTENDED_9]], <16 x i32> <i32 0, i32 16, i32 2, i32 3, i32 4, i32 17, i32 6, i32 7, i32 8, i32 18, i32 10, i32 11, i32 12, i32 19, i32 14, i32 15>
; CHECK-NEXT:    [[EXTENDED_10:%.*]] = shufflevector <4 x i32> [[TMP8]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP12:%.*]] = shufflevector <16 x i32> [[TMP11]], <16 x i32> [[EXTENDED_10]], <16 x i32> <i32 0, i32 1, i32 16, i32 3, i32 4, i32 5, i32 17, i32 7, i32 8, i32 9, i32 18, i32 11, i32 12, i32 13, i32 19, i32 15>
; CHECK-NEXT:    [[EXTENDED_11:%.*]] = shufflevector <4 x i32> [[TMP5]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP13:%.*]] = shufflevector <16 x i32> [[TMP12]], <16 x i32> [[EXTENDED_11]], <16 x i32> <i32 0, i32 1, i32 2, i32 16, i32 4, i32 5, i32 6, i32 17, i32 8, i32 9, i32 10, i32 18, i32 12, i32 13, i32 14, i32 19>
; CHECK-NEXT:    [[TMP14:%.*]] = bitcast i32* [[SCALAR_GEP5]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[TMP13]], <16 x i32>* [[TMP14]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  ; ld: stmt 0 (ary + 3)
  %i0 = add nsw i64 %indvars.iv, 3
  %p0 = getelementptr inbounds i32, i32* %ary, i64 %i0
  %l0 = load i32, i32* %p0, align 4
  %t0 = add nsw i32 %l0, 11

  ; ld: stmt 1 (ary + 0)
  %p1 = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %l1 = load i32, i32* %p1, align 4
  %t1 = add nsw i32 %l1, 22

  ; ld: stmt 2 (ary + 2)
  %i2 = add nsw i64 %indvars.iv, 2
  %p2 = getelementptr inbounds i32, i32* %ary, i64 %i2
  %l2 = load i32, i32* %p2, align 4
  %t2 = add nsw i32 %l2, 33

  ; ld: stmt 3 (ary + 0)
  %l3 = load i32, i32* %p1, align 4
  %t3 = add nsw i32 %l3, 44

  ; ld: stmt 4 (ary + 1)
  %i4 = add nsw i64 %indvars.iv, 1
  %p4 = getelementptr inbounds i32, i32* %ary, i64 %i4
  %l4 = load i32, i32* %p4, align 4
  %t4 = add nsw i32 %l4, 55

  ; ld: stmt 5 (ary + 2)
  %l5 = load i32, i32* %p2, align 4
  %t5 = add nsw i32 %l5, 66

  ; st: stmt 0 (ary + 1)
  store i32 %t0, i32* %p4, align 4

  ; st: stmt 1 (ary + 2)
  store i32 %t1, i32* %p2, align 4

  ; st: stmt 2 (ary + 3)
  store i32 %t2, i32* %p0, align 4

  ; st: stmt 3 (ary + 0)
  store i32 %t3, i32* %p1, align 4

  ; st: stmt 4 (ary + 2)
  store i32 %t4, i32* %p2, align 4

  ; st: stmt 5 (ary + 1)
  store i32 %t5, i32* %p4, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

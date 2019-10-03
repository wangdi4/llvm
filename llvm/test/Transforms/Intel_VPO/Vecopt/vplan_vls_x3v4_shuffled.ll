; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(<4 x i32>* nocapture %ary) {
;  typedef int32_t v4i32 __attribute__((vector_size(16)));
;  v4i32 *ary, t0, t1, t2;
;  for (i = 0; i < 1024; i += 3) {
;    l0 = ary[i + 1];
;    l1 = ary[i + 2];
;    l2 = ary[i + 0];
;    ary[i + 0] = l0 + <10, 11, 12, 13>;
;    ary[i + 2] = l1 + <20, 21, 22, 23>;
;    ary[i + 1] = l2 + <30, 31, 32, 33>;
;  }
;
; CHECK:       Printing Groups- Total Groups 5
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #3 <4 x 128> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 11111111111111111111111111111111
; CHECK-NEXT:   #1 <4 x 128> SLoad
; CHECK-NEXT:   #2 <4 x 128> SLoad
; CHECK-NEXT:  Group#3
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #4 <4 x 128> SStore
; CHECK-NEXT:  Group#4
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #6 <4 x 128> SStore
; CHECK-NEXT:  Group#5
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #5 <4 x 128> SStore
;
; CHECK:       vector.body:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i64 [ 0, [[VECTOR_PH:%.*]] ], [ [[INDEX_NEXT:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VECTOR_PH]] ], [ [[TMP12:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 0, i64 3, i64 6, i64 9>, [[VECTOR_PH]] ], [ [[TMP11:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[TMP0:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[TMP0]]
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_:%.*]] = shufflevector <4 x i32*> [[TMP1]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32(<16 x i32*> [[ELEMBASEPTR_]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> undef)
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP2]]
; CHECK-NEXT:    [[TMP3:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP1]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_2:%.*]] = shufflevector <4 x i32*> [[TMP3]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_3:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_2]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER4:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32(<16 x i32*> [[ELEMBASEPTR_3]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> undef)
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]]
; CHECK-NEXT:    [[TMP4:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP5]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_6:%.*]] = shufflevector <4 x i32*> [[TMP4]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_7:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_6]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER8:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32(<16 x i32*> [[ELEMBASEPTR_7]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x i32> undef)
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <16 x i32> [[WIDE_MASKED_GATHER]], <i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13>
; CHECK-NEXT:    [[TMP6:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP5]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_9:%.*]] = shufflevector <4 x i32*> [[TMP6]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_10:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_9]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v16i32.v16p0i32(<16 x i32> [[TMP5]], <16 x i32*> [[ELEMBASEPTR_10]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    [[TMP7:%.*]] = add nsw <16 x i32> [[WIDE_MASKED_GATHER4]], <i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23>
; CHECK-NEXT:    [[TMP8:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP1]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_11:%.*]] = shufflevector <4 x i32*> [[TMP8]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_12:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_11]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v16i32.v16p0i32(<16 x i32> [[TMP7]], <16 x i32*> [[ELEMBASEPTR_12]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    [[TMP9:%.*]] = add nsw <16 x i32> [[WIDE_MASKED_GATHER8]], <i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33>
; CHECK-NEXT:    [[TMP10:%.*]] = bitcast <4 x <4 x i32>*> [[MM_VECTORGEP]] to <4 x i32*>
; CHECK-NEXT:    [[VECBASEPTR_13:%.*]] = shufflevector <4 x i32*> [[TMP10]], <4 x i32*> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1, i32 2, i32 2, i32 2, i32 2, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ELEMBASEPTR_14:%.*]] = getelementptr i32, <16 x i32*> [[VECBASEPTR_13]], <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:    call void @llvm.masked.scatter.v16i32.v16p0i32(<16 x i32> [[TMP9]], <16 x i32*> [[ELEMBASEPTR_14]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    [[TMP11]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 12, i64 12, i64 12, i64 12>
; CHECK-NEXT:    [[TMP12]] = add nuw nsw i64 [[UNI_PHI]], 12
; CHECK-NEXT:    [[TMP13:%.*]] = icmp ult <4 x i64> [[TMP11]], <i64 1024, i64 1024, i64 1024, i64 1024>
; CHECK-NEXT:    [[TMP14:%.*]] = extractelement <4 x i1> [[TMP13]], i32 0
; CHECK-NEXT:    [[INDEX_NEXT]] = add i64 [[INDEX]], 4
; CHECK-NEXT:    [[TMP15:%.*]] = icmp eq i64 [[INDEX_NEXT]], 340
; CHECK-NEXT:    br i1 [[TMP15]], label [[VPLANNEDBB:%.*]], label [[VECTOR_BODY]]
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  ; l0 = ary[i + 1];
  %i0 = add nsw i64 %indvars.iv, 1
  %p0 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %i0
  %l0 = load <4 x i32>, <4 x i32>* %p0, align 4

  ; l1 = ary[i + 2];
  %i1 = add nsw i64 %indvars.iv, 2
  %p1 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %i1
  %l1 = load <4 x i32>, <4 x i32>* %p1, align 4

  ; l2 = ary[i + 0];
  %p2 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %indvars.iv
  %l2 = load <4 x i32>, <4 x i32>* %p2, align 4

  ; ary[i + 0] = l0 + 11;
  %t0 = add nsw <4 x i32> %l0, <i32 10, i32 11, i32 12, i32 13>
  store <4 x i32> %t0, <4 x i32>* %p2, align 4

  ; ary[i + 2] = l1 + 22;
  %t1 = add nsw <4 x i32> %l1, <i32 20, i32 21, i32 22, i32 23>
  store <4 x i32> %t1, <4 x i32>* %p1, align 4

  ; ary[i + 1] = l2 + 33;
  %t2 = add nsw <4 x i32> %l2, <i32 30, i32 31, i32 32, i32 33>
  store <4 x i32> %t2, <4 x i32>* %p0, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

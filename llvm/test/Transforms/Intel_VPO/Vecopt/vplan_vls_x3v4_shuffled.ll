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
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 48
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111111111111111111111111111
; CHECK-NEXT:   #3 <4 x 128> SLoad
; CHECK-NEXT:   #1 <4 x 128> SLoad
; CHECK-NEXT:   #2 <4 x 128> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 48
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111111111111111111111111111
; CHECK-NEXT:   #4 <4 x 128> SStore
; CHECK-NEXT:   #6 <4 x 128> SStore
; CHECK-NEXT:   #5 <4 x 128> SStore
;
; CHECK:       vector.body:
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[TMP0:%.*]]
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x <4 x i32>*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPSTART:%.*]] = getelementptr inbounds <4 x i32>, <4 x i32>* [[MM_VECTORGEP_0]], i64 -1
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast <4 x i32>* [[GROUPSTART]] to <48 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <48 x i32>, <48 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <48 x i32> [[GROUPLOAD]], <48 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 16, i32 17, i32 18, i32 19, i32 28, i32 29, i32 30, i32 31, i32 40, i32 41, i32 42, i32 43>
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw <4 x i64> [[VEC_PHI:%.*]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP1]]
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <48 x i32> [[GROUPLOAD]], <48 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 20, i32 21, i32 22, i32 23, i32 32, i32 33, i32 34, i32 35, i32 44, i32 45, i32 46, i32 47>
; CHECK-NEXT:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds <4 x i32>, <4 x <4 x i32>*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]]
; CHECK-NEXT:    [[GROUPSHUFFLE4:%.*]] = shufflevector <48 x i32> [[GROUPLOAD]], <48 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 12, i32 13, i32 14, i32 15, i32 24, i32 25, i32 26, i32 27, i32 36, i32 37, i32 38, i32 39>
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <16 x i32> [[GROUPSHUFFLE]], <i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13, i32 10, i32 11, i32 12, i32 13>
; CHECK-NEXT:    [[TMP3:%.*]] = add nsw <16 x i32> [[GROUPSHUFFLE2]], <i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23, i32 20, i32 21, i32 22, i32 23>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <16 x i32> [[GROUPSHUFFLE4]], <i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33, i32 30, i32 31, i32 32, i32 33>
; CHECK-NEXT:    [[TMP5:%.*]] = shufflevector <16 x i32> [[TMP2]], <16 x i32> [[TMP4]], <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:    [[TMP6:%.*]] = shufflevector <16 x i32> [[TMP3]], <16 x i32> undef, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP7:%.*]] = shufflevector <32 x i32> [[TMP5]], <32 x i32> [[TMP6]], <48 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 32, i32 33, i32 34, i32 35, i32 36, i32 37, i32 38, i32 39, i32 40, i32 41, i32 42, i32 43, i32 44, i32 45, i32 46, i32 47>
; CHECK-NEXT:    [[GROUPSHUFFLE5:%.*]] = shufflevector <48 x i32> [[TMP7]], <48 x i32> undef, <48 x i32> <i32 0, i32 1, i32 2, i32 3, i32 16, i32 17, i32 18, i32 19, i32 32, i32 33, i32 34, i32 35, i32 4, i32 5, i32 6, i32 7, i32 20, i32 21, i32 22, i32 23, i32 36, i32 37, i32 38, i32 39, i32 8, i32 9, i32 10, i32 11, i32 24, i32 25, i32 26, i32 27, i32 40, i32 41, i32 42, i32 43, i32 12, i32 13, i32 14, i32 15, i32 28, i32 29, i32 30, i32 31, i32 44, i32 45, i32 46, i32 47>
; CHECK-NEXT:    [[MM_VECTORGEP3_0:%.*]] = extractelement <4 x <4 x i32>*> [[MM_VECTORGEP3]], i64 0
; CHECK-NEXT:    [[GROUPPTR6:%.*]] = bitcast <4 x i32>* [[MM_VECTORGEP3_0]] to <48 x i32>*
; CHECK-NEXT:    store <48 x i32> [[GROUPSHUFFLE5]], <48 x i32>* [[GROUPPTR6]], align 4
; CHECK:         br i1 %{{.*}}, label %{{.*}}, label %vector.body
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

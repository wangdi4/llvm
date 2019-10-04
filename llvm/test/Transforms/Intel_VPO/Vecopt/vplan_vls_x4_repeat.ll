; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
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
; CHECK:       Printing Groups- Total Groups 8
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #5 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#3
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:  Group#4
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #6 <4 x 32> SLoad
; CHECK-NEXT:  Group#5
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #10 <4 x 32> SStore
; CHECK-NEXT:  Group#6
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
; CHECK-NEXT:   #9 <4 x 32> SStore
; CHECK-NEXT:  Group#7
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #11 <4 x 32> SStore
; CHECK-NEXT:  Group#8
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #12 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:    [[TMP0:%.*]] = add nsw <4 x i64> [[VEC_IND:%.*]], <i64 3, i64 3, i64 3, i64 3>
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[TMP0]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_IND]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER2:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP1]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP2:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER2]], <i32 22, i32 22, i32 22, i32 22>
; CHECK-NEXT:    [[TMP3:%.*]] = add nsw <4 x i64> [[VEC_IND]], <i64 2, i64 2, i64 2, i64 2>
; CHECK-NEXT:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP3]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER4:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER4]], <i32 33, i32 33, i32 33, i32 33>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER5:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP1]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER5]], <i32 44, i32 44, i32 44, i32 44>
; CHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i64> [[VEC_IND]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> [[TMP6]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER7:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP6]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP7:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER7]], <i32 55, i32 55, i32 55, i32 55>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER8:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP8:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER8]], <i32 66, i32 66, i32 66, i32 66>
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP1]], <4 x i32*> [[MM_VECTORGEP6]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP2]], <4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP4]], <4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP5]], <4 x i32*> [[MM_VECTORGEP1]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP7]], <4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP8]], <4 x i32*> [[MM_VECTORGEP6]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
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

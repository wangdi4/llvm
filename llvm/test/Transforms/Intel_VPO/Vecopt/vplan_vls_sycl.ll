; RUN: opt -S -VPlanDriver -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; The test checks if OptVLS recognizes complex address computation patterns
; typical for SYCL applications. For example,
;    %src.p.1 = getelementptr inbounds i32, i32* %src, i64 %src.i.1
;    -->  {(4 + (12 * %x) + %src),+,8}<nsw><%for.body>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %dst, i32* nocapture %src, i64 %x) {
;  for (i = 0; i < 2048; i += 2) {
;    t0 = src[3*x + i] + 7;
;    t1 = src[3*x + i + 1] + 11;
;    dst[4*x + i] = t0;
;    dst[4*x + i + 1] = t1;
;  }
;
; CHECK:       Printing Groups- Total Groups 4
; CHECK-NEXT:  Group#1:
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 8
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2:
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 8
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:  Group#3:
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 8
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #4 <4 x 32> SStore
; CHECK-NEXT:  Group#4:
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 8
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #3 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:         [[TMP0:%.*]] = add nsw <4 x i64> [[BROADCAST_SPLAT:%.*]], [[VEC_PHI:%.*]]
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw <4 x i64> [[TMP0]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT2:%.*]], <4 x i64> [[TMP0]]
; CHECK-NEXT:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT2]], <4 x i64> [[TMP1]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[WIDE_MASKED_GATHER4:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP2:%.*]] = add <4 x i32> [[WIDE_MASKED_GATHER]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP3:%.*]] = add <4 x i32> [[WIDE_MASKED_GATHER4]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i64> [[BROADCAST_SPLAT6:%.*]], [[VEC_PHI]]
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i64> [[TMP4]], <i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[MM_VECTORGEP9:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT8:%.*]], <4 x i64> [[TMP4]]
; CHECK-NEXT:    [[MM_VECTORGEP10:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT8]], <4 x i64> [[TMP5]]
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP2]], <4 x i32*> [[MM_VECTORGEP9]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP3]], <4 x i32*> [[MM_VECTORGEP10]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>)
; CHECK:         br i1 %{{.*}}, label %{{.*}}, label %vector.body
;
entry:
  %x3 = mul nsw i64 %x, 3
  %x4 = mul nsw i64 %x, 4
  br label %region

region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %region ], [ %indvars.iv.next, %for.body ]

  %src.i.0 = add nsw i64 %x3, %indvars.iv
  %src.i.1 = add nsw i64 %src.i.0, 1

  %src.p.0 = getelementptr inbounds i32, i32* %src, i64 %src.i.0
  %src.p.1 = getelementptr inbounds i32, i32* %src, i64 %src.i.1

  %src.0 = load i32, i32* %src.p.0, align 4
  %src.1 = load i32, i32* %src.p.1, align 4

  %add.0 = add i32 %src.0, 7
  %add.1 = add i32 %src.1, 11

  %dst.i.0 = add nsw i64 %x4, %indvars.iv
  %dst.i.1 = add nsw i64 %dst.i.0, 1

  %dst.p.0 = getelementptr inbounds i32, i32* %dst, i64 %dst.i.0
  %dst.p.1 = getelementptr inbounds i32, i32* %dst, i64 %dst.i.1

  store i32 %add.0, i32* %dst.p.0, align 4
  store i32 %add.1, i32* %dst.p.1, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 2048
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

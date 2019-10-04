; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 4) {
;    if (ary[i] & 1) {
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
; CHECK:       Printing Groups- Total Groups 3
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:  Group#3
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 16
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #5 <4 x 32> SStore
; CHECK-NEXT:   #6 <4 x 32> SStore
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
;
; CHECK:       vector.body:
; CHECK:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT:%.*]], <4 x i64> {{%.*}}
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i32> undef)
; CHECK-NEXT:    [[TMP0:%.*]] = and <4 x i32> [[WIDE_MASKED_GATHER]], <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[TMP1:%.*]] = icmp eq <4 x i32> [[TMP0]], zeroinitializer
; CHECK-NEXT:    [[TMP2:%.*]] = xor <4 x i1> [[TMP1]], <i1 true, i1 true, i1 true, i1 true>
; CHECK-NEXT:    [[TMP3:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER]], <i32 7, i32 7, i32 7, i32 7>
; CHECK:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> {{%.*}}
; CHECK-NEXT:    [[WIDE_MASKED_GATHER2:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP1]], i32 4, <4 x i1> [[TMP2]], <4 x i32> undef)
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER2]], <i32 11, i32 11, i32 11, i32 11>
; CHECK:    [[MM_VECTORGEP3:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> {{%.*}}
; CHECK-NEXT:    [[WIDE_MASKED_GATHER4:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> [[TMP2]], <4 x i32> undef)
; CHECK-NEXT:    [[TMP7:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER4]], <i32 12, i32 12, i32 12, i32 12>
; CHECK:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds i32, <4 x i32*> [[BROADCAST_SPLAT]], <4 x i64> {{%.*}}
; CHECK-NEXT:    [[WIDE_MASKED_GATHER6:%.*]] = call <4 x i32> @llvm.masked.gather.v4i32.v4p0i32(<4 x i32*> [[MM_VECTORGEP5]], i32 4, <4 x i1> [[TMP2]], <4 x i32> undef)
; CHECK-NEXT:    [[TMP9:%.*]] = add nsw <4 x i32> [[WIDE_MASKED_GATHER6]], <i32 61, i32 61, i32 61, i32 61>
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP3]], <4 x i32*> [[MM_VECTORGEP]], i32 4, <4 x i1> [[TMP2]])
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP5]], <4 x i32*> [[MM_VECTORGEP1]], i32 4, <4 x i1> [[TMP2]])
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP7]], <4 x i32*> [[MM_VECTORGEP3]], i32 4, <4 x i1> [[TMP2]])
; CHECK-NEXT:    call void @llvm.masked.scatter.v4i32.v4p0i32(<4 x i32> [[TMP9]], <4 x i32*> [[MM_VECTORGEP5]], i32 4, <4 x i1> [[TMP2]])
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %and = and i32 %0, 1
  %tobool = icmp eq i32 %and, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
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

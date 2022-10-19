; RUN: opt -S -vplan-vec -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 3072; i += 3) {
;    t0 = ary[i + 0] + 7;
;    t1 = ary[i + 1] + 11;
;    t2 = ary[i + 2] + 12;
;    ary[i + 0] = t0;
;    ary[i + 1] = t1;
;    ary[i + 2] = t2;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 12
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 12
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111
; CHECK-NEXT:   #4 <4 x 32> SStore
; CHECK-NEXT:   #5 <4 x 32> SStore
; CHECK-NEXT:   #6 <4 x 32> SStore
;
; CHECK-LABEL: @foo(
; CHECK:         [[SCALAR_GEP:%.*]] = getelementptr inbounds i32, i32* [[ARY:%.*]], i64 [[UNI_PHI:%.*]]
; CHECK-NEXT:    [[TMP0:%.*]] = bitcast i32* [[SCALAR_GEP]] to <16 x i32>*
; CHECK-NEXT:    [[VLS_LOAD:%.*]] = call <16 x i32> @llvm.masked.load.v16i32.p0v16i32(<16 x i32>* [[TMP0]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false>, <16 x i32> poison)
; CHECK-NEXT:    [[TMP1:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 0, i32 3, i32 6, i32 9>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 1, i32 4, i32 7, i32 10>
; CHECK-NEXT:    [[TMP3:%.*]] = shufflevector <16 x i32> [[VLS_LOAD]], <16 x i32> [[VLS_LOAD]], <4 x i32> <i32 2, i32 5, i32 8, i32 11>
; CHECK-NEXT:    [[TMP4:%.*]] = add nsw <4 x i32> [[TMP1]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i32> [[TMP2]], <i32 11, i32 11, i32 11, i32 11>
; CHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i32> [[TMP3]], <i32 12, i32 12, i32 12, i32 12>
; CHECK-NEXT:    [[TMP7:%.*]] = shufflevector <4 x i32> [[TMP4]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP8:%.*]] = shufflevector <16 x i32> undef, <16 x i32> [[TMP7]], <16 x i32> <i32 16, i32 1, i32 2, i32 17, i32 4, i32 5, i32 18, i32 7, i32 8, i32 19, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[TMP9:%.*]] = shufflevector <4 x i32> [[TMP5]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP10:%.*]] = shufflevector <16 x i32> [[TMP8]], <16 x i32> [[TMP9]], <16 x i32> <i32 0, i32 16, i32 2, i32 3, i32 17, i32 5, i32 6, i32 18, i32 8, i32 9, i32 19, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[TMP11:%.*]] = shufflevector <4 x i32> [[TMP6]], <4 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
; CHECK-NEXT:    [[TMP12:%.*]] = shufflevector <16 x i32> [[TMP10]], <16 x i32> [[TMP11]], <16 x i32> <i32 0, i32 1, i32 16, i32 3, i32 4, i32 17, i32 6, i32 7, i32 18, i32 9, i32 10, i32 19, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[TMP13:%.*]] = bitcast i32* [[SCALAR_GEP]] to <16 x i32>*
; CHECK-NEXT:    call void @llvm.masked.store.v16i32.p0v16i32(<16 x i32> [[TMP12]], <16 x i32>* [[TMP13]], i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false>)
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
  store i32 %add7, i32* %arrayidx, align 4
  store i32 %add11, i32* %arrayidx4, align 4
  store i32 %add12, i32* %arrayidx8, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 3072
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

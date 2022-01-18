; RUN: opt -S -vplan-vec < %s | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 1) {
;    ary[i + 0] += 7;
;  }
;
; CHECK-LABEL: @foo(
; CHECK:         [[SCALAR_GEP:%.*]] = getelementptr inbounds i32, i32* [[ARY:%.*]], i64 [[UNI_PHI:%.*]]
; CHECK-NEXT:    [[TMP0:%.*]] = bitcast i32* [[SCALAR_GEP]] to <4 x i32>*
; CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <4 x i32>, <4 x i32>* [[TMP0]], align 4
; CHECK-NEXT:    [[TMP1:%.*]] = add nsw <4 x i32> [[WIDE_LOAD]], <i32 7, i32 7, i32 7, i32 7>
; CHECK-NEXT:    [[TMP2:%.*]] = bitcast i32* [[SCALAR_GEP]] to <4 x i32>*
; CHECK-NEXT:    store <4 x i32> [[TMP1]], <4 x i32>* [[TMP2]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add7 = add nsw i32 %0, 7
  store i32 %add7, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

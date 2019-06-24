; RUN: opt -VPlanDriver -vplan-force-vf=4 -enable-vplan-vls-cg -debug-only=ovls -disable-output < %s 2>&1  | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* nocapture %ary) {
;  for (i = 0; i < 1024; i += 4) {
;    t0 = ary[i + 0] + 7;
;    t1 = ary[i + 1] + 11;
;    t2 = ary[i + 2] + 12;
;    t3 = ary[i + 3] + 61;
;    ary[i + 0] = t0;
;    ary[i + 1] = t1;
;    ary[i + 2] = t2;
;    ary[i + 3] = t3;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #1 <4 x 32> SLoad
; CHECK-NEXT:   #2 <4 x 32> SLoad
; CHECK-NEXT:   #3 <4 x 32> SLoad
; CHECK-NEXT:   #4 <4 x 32> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore
; CHECK-NEXT:    AccessMask(per byte, R to L): 1111111111111111
; CHECK-NEXT:   #5 <4 x 32> SStore
; CHECK-NEXT:   #6 <4 x 32> SStore
; CHECK-NEXT:   #7 <4 x 32> SStore
; CHECK-NEXT:   #8 <4 x 32> SStore
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %ary, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %add1 = add nsw i32 %0, 7
  %1 = add nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %ary, i64 %1
  %2 = load i32, i32* %arrayidx4, align 4
  %add5 = add nsw i32 %2, 11
  %3 = add nsw i64 %indvars.iv, 2
  %arrayidx8 = getelementptr inbounds i32, i32* %ary, i64 %3
  %4 = load i32, i32* %arrayidx8, align 4
  %add9 = add nsw i32 %4, 12
  %5 = add nsw i64 %indvars.iv, 3
  %arrayidx12 = getelementptr inbounds i32, i32* %ary, i64 %5
  %6 = load i32, i32* %arrayidx12, align 4
  %add13 = add nsw i32 %6, 61
  store i32 %add1, i32* %arrayidx, align 4
  store i32 %add5, i32* %arrayidx4, align 4
  store i32 %add9, i32* %arrayidx8, align 4
  store i32 %add13, i32* %arrayidx12, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 4
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

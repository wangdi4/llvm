; CMPLRLLVM-23003

; RUN: opt -S -passes=vplan-vec -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %ary) {
;  long double *ary, t0, t1, t2;
;  for (i = 0; i < 2048; i += 2) {
;    t0 = ary[i + 0] + 7;
;    t1 = ary[i + 1] + 11;
;    ary[i + 0] = t0;
;    ary[i + 1] = t1;
;  }
;
; CHECK: Received a vector of memrefs (0):
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 2) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds x86_fp80, ptr %ary, i64 %indvars.iv
  %0 = load x86_fp80, ptr %arrayidx, align 16
  %1 = add nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds x86_fp80, ptr %ary, i64 %1
  %2 = load x86_fp80, ptr %arrayidx4, align 16
  store x86_fp80 %0, ptr %arrayidx, align 16
  store x86_fp80 %2, ptr %arrayidx4, align 16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 2048
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

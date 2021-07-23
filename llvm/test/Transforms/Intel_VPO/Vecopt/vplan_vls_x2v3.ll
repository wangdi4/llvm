; CMPLRLLVM-23003

; RUN: opt -S -vplan-vec -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(<3 x i32>* nocapture %ary) {
;  typedef int32_t v4i32 __attribute__((vector_size(16)));
;  v3i32 *ary, t0, t1, t2;
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
  %arrayidx = getelementptr inbounds <3 x i32>, <3 x i32>* %ary, i64 %indvars.iv
  %0 = load <3 x i32>, <3 x i32>* %arrayidx, align 16
  %add7 = add nsw <3 x i32> %0, <i32 7, i32 7, i32 7>
  %1 = add nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds <3 x i32>, <3 x i32>* %ary, i64 %1
  %2 = load <3 x i32>, <3 x i32>* %arrayidx4, align 16
  %add11 = add nsw <3 x i32> %2, <i32 11, i32 11, i32 11>
  store <3 x i32> %add7, <3 x i32>* %arrayidx, align 16
  store <3 x i32> %add11, <3 x i32>* %arrayidx4, align 16
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 2048
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

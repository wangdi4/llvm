; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; typedef int32_t huge_t __attribute__((vector_size(64)));
; v4i32 *ary, t0, t1, t2;
; for (i = 0; i < 2048; i += 2) {
;   t0 = ary[i + 0];
;   t1 = ary[i + 1];
;   ary[i + 0] = t0 + t1;
;   ary[i + 1] = t0 - t1;
; }
;
; Check that no memrefs are created.
; CHECK:  Recieved a vector of memrefs (0)
;
define void @foo(<64 x i32>* nocapture %ary) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  ; t0 = ary[i + 0];
  %p0 = getelementptr inbounds <64 x i32>, <64 x i32>* %ary, i64 %indvars.iv
  %t0 = load <64 x i32>, <64 x i32>* %p0, align 4

  ; t1 = ary[i + 1];
  %i1 = add nsw i64 %indvars.iv, 1
  %p1 = getelementptr inbounds <64 x i32>, <64 x i32>* %ary, i64 %i1
  %t1 = load <64 x i32>, <64 x i32>* %p1, align 4

  ; ary[i + 0] = t0 + t1;
  %tadd = add <64 x i32> %t0, %t1
  store <64 x i32> %tadd, <64 x i32>* %p0, align 4

  ; ary[i + 1] = t0 - t1;
  %tsub = sub <64 x i32> %t0, %t1
  store <64 x i32> %tsub, <64 x i32>* %p1, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 3
  %cmp = icmp ult i64 %indvars.iv.next, 3072
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

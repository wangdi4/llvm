; RUN: opt -S -VPlanDriver -enable-vp-value-codegen -debug-only=ovls < %s 2>&1 | FileCheck %s
; REQUIRES: asserts

; This is a test for a group with stride greater than group size. When looking
; at the group mask only, it may look like an access without gaps. However, in
; fact there are gaps (no access to ary[i + 4]) which must be handled properly.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(<4 x i32>* nocapture %ary) {
;  typedef int32_t v4i32 __attribute__((vector_size(16)));
;  v4i32 *ary, l0, l1, l2;
;  for (i = 0; i < 1024; i += 7) {
;    l0 = ary[i + 0];
;    l1 = ary[i + 1];
;    l2 = ary[i + 2];
;    ary[i + 0] = l0 + <10, 11, 12, 13>;
;    ary[i + 1] = l1 + <20, 21, 22, 23>;
;    ary[i + 2] = l2 + <30, 31, 32, 33>;
;  }
;
; CHECK:       Printing Groups- Total Groups 2
; CHECK-NEXT:  Group#1
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SLoad, Stride (in bytes): 112
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111111111111111111111111111
; CHECK-NEXT:   #1 <4 x 128> SLoad
; CHECK-NEXT:   #2 <4 x 128> SLoad
; CHECK-NEXT:   #3 <4 x 128> SLoad
; CHECK-NEXT:  Group#2
; CHECK-NEXT:    Vector Length(in bytes): 64
; CHECK-NEXT:    AccType: SStore, Stride (in bytes): 112
; CHECK-NEXT:    AccessMask(per byte, R to L): 111111111111111111111111111111111111111111111111
; CHECK-NEXT:   #4 <4 x 128> SStore
; CHECK-NEXT:   #5 <4 x 128> SStore
; CHECK-NEXT:   #6 <4 x 128> SStore
;
; Check that OptVLS is not triggered on this example. We cannot handle gaps yet.
; CHECK-LABEL: vector.body
; CHECK:        call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32
; CHECK:        call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32
; CHECK:        call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32
; CHECK-NOT:    call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32
; CHECK:        call void @llvm.masked.scatter.v16i32.v16p0i32
; CHECK:        call void @llvm.masked.scatter.v16i32.v16p0i32
; CHECK:        call void @llvm.masked.scatter.v16i32.v16p0i32
; CHECK-NOT:    call void @llvm.masked.scatter.v16i32.v16p0i32
; CHECK-LABEL: VPlannedBB
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  ; l0 = ary[i + 0];
  %p0 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %indvars.iv
  %l0 = load <4 x i32>, <4 x i32>* %p0, align 4

  ; l1 = ary[i + 1];
  %i1 = add nsw i64 %indvars.iv, 1
  %p1 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %i1
  %l1 = load <4 x i32>, <4 x i32>* %p1, align 4

  ; l2 = ary[i + 2];
  %i2 = add nsw i64 %indvars.iv, 2
  %p2 = getelementptr inbounds <4 x i32>, <4 x i32>* %ary, i64 %i2
  %l2 = load <4 x i32>, <4 x i32>* %p2, align 4

  ; ary[i + 0] = l0 + <...>;
  %t0 = add nsw <4 x i32> %l0, <i32 10, i32 11, i32 12, i32 13>
  store <4 x i32> %t0, <4 x i32>* %p0, align 4

  ; ary[i + 1] = l1 + <...>;
  %t1 = add nsw <4 x i32> %l1, <i32 20, i32 21, i32 22, i32 23>
  store <4 x i32> %t1, <4 x i32>* %p1, align 4

  ; ary[i + 2] = l2 + <...>;
  %t2 = add nsw <4 x i32> %l2, <i32 30, i32 31, i32 32, i32 33>
  store <4 x i32> %t2, <4 x i32>* %p2, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 7
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec' \
; RUN:     -vplan-force-vf=4 \
; RUN:     -vplan-enable-peeling-hir -vplan-enable-general-peeling-hir \
; RUN:     -debug-only=vplan-value-tracking \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s
;

; REQUIRES: asserts
;
; This is a test for a complex known bits computation that takes into
; account alignment of several values plus a constant value.
;
;   void foo(int *dst_, int *src_, int x, int size) {
;     int *dst = __builtin_assume_aligned(dst_, 128, 32);
;     int *src = __builtin_assume_aligned(src_, 64, 24);
;     for (int i = 0; i < size; ++i)
;       dst[5*x + i + 7] = src[5*x + i + 2] + i * i;
;   }
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%size) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:        |   %2 = (%src)[i1 + sext.i32.i64((5 * %x))];
; CHECK-NEXT:        |   %3 = i1  *  i1;
; CHECK-NEXT:        |   %4 = trunc.i64.i32(%3);
; CHECK-NEXT:        |   (%dst)[i1 + sext.i32.i64((5 * %x)) + 7] = %2 + %4;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:  END REGION
;
; CHECK:       getKnownBits({(%src + 4 * sext.i32.i64((5 * %x))),+,0})
; CHECK-NEXT:    ??????????????????????????????????????????????????????????????00
;
; CHECK:       getKnownBits({(4 * sext.i32.i64((5 * %x)) + %dst + 28),+,0})
; CHECK-NEXT:    -> ??????????????????????????????????????????????????????????????00
;
; FIXME: Alignment assumption bundles with displacement have very
;        limited support in ValueTracking. Basically, ["align"(i8* %0, i64 128, i64 32)]
;        is processed as if it were ["align"(i8* %0, i64 32)]. That's why the
;        precision of the analysis is so limited here. If it were fully
;        supported, the result would be {Zero=27, One=36}.
; CHECK:       getKnownBits({(-1 * %src + %dst + 28),+,0})
; CHECK-NEXT:    -> ?????????????????????????????????????????????????????????????100
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias %dst, ptr noalias %src, i32 %x, i32 %size) {
entry:
  call void @llvm.assume(i1 true) [ "align"(ptr %dst, i64 128, i64 32) ]
  call void @llvm.assume(i1 true) [ "align"(ptr %src, i64 64, i64 24) ]
  %cmp17 = icmp sgt i32 %size, 0
  br i1 %cmp17, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %mul = mul nsw i32 %x, 5
  %0 = sext i32 %mul to i64
  %wide.trip.count22 = zext i32 %size to i64
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %1 = add nsw i64 %indvars.iv, %0
  %ptridx = getelementptr inbounds i32, ptr %src, i64 %1
  %2 = load i32, ptr %ptridx, align 4
  %3 = mul nsw i64 %indvars.iv, %indvars.iv
  %4 = trunc i64 %3 to i32
  %add2 = add nsw i32 %2, %4
  %5 = add nsw i64 %1, 7
  %ptridx7 = getelementptr inbounds i32, ptr %dst, i64 %5
  store i32 %add2, ptr %ptridx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count22
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare void @llvm.assume(i1 noundef) #0
attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }

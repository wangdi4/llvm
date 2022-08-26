; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -vplan-force-vf=4 \
; RUN:     -vplan-enable-peeling \
; RUN:     -debug-only=vplan-value-tracking \
; RUN:     -print-before=hir-vplan-vec -disable-output < %s 2>&1 \
; RUN: | FileCheck %s
;
; REQUIRES: asserts
;
;   void foo(int *dst_, int size) {
;     int *dst = __builtin_assume_aligned(dst_, 32);
;     for (int i = 0; i < size; ++i)
;       dst[i] = i + 7;
;   }
;
; CHECK:       BEGIN REGION { }
; CHECK-NEXT:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        + DO i1 = 0, zext.i32.i64(%size) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK-NEXT:        |   (%dst)[i1] = i1 + 7;
; CHECK-NEXT:        + END LOOP
; CHECK-NEXT:<{{.*}}>
; CHECK-NEXT:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:  END REGION
;
; CHECK:       getKnownBits({(%dst),+,0})
; CHECK-NEXT:    -> {Zero=31, One=0}
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* %dst, i32 %size) {
entry:
  %0 = bitcast i32* %dst to i8*
  call void @llvm.assume(i1 true) [ "align"(i8* %0, i64 32) ]
  %cmp6 = icmp sgt i32 %size, 0
  br i1 %cmp6, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count9 = zext i32 %size to i64
  br label %for.body

for.cond.cleanup.loopexit:
  br label %for.cond.cleanup

for.cond.cleanup:
  ret void

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %ptridx = getelementptr inbounds i32, i32* %dst, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  %2 = add i32 %1, 7
  store i32 %2, i32* %ptridx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count9
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body
}

declare void @llvm.assume(i1 noundef) #0
attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }

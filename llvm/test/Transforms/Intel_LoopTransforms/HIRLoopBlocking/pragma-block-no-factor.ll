; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-pragma-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -disable-output 2>&1 < %s | FileCheck %s

; Check that we properly handle block_loop pragma that has no factor specified by the user and
; instead use the default blocking factor.

; Source
;subroutine block(a,b,c,n)
;  integer n
;  real a(n,n),b(n,n),c(n,n)
;  integer i,j,k
;
;  !dir$ block_loop
;  do k=1,n
;    do j=1,n
;      do i=1,n
;        c(i,j) = c(i,j) + a(i,k)*b(k,j)
;      end do
;    end do
;  end do
;  return
;end

;Final LoopToPragma:
;LoopLevel: 1
;Level: 1, -1
;Level: 2, -1
;Level: 3, -1
;Final LoopMap:
;LoopLevel: 1
;BlockFactor: 64
;LoopLevel: 2
;BlockFactor: 64
;LoopLevel: 3
;BlockFactor: 64

; *** IR Dump After HIR Loop Blocking based on Pragma (hir-pragma-loop-blocking) ***
; Function: block_

; CHECK:  BEGIN REGION { modified }

; CHECK: DO i1
; CHECK: DO i2
; CHECK: DO i3
;               + DO i1 = 0, (zext.i32.i64(%"block_$N_fetch.1") + -1)/u64, 1   <DO_LOOP>
;               |   %min = (-64 * i1 + zext.i32.i64(%"block_$N_fetch.1") + -1 <= 63) ? -64 * i1 + zext.i32.i64(%"block_$N_fetch.1") + -1 : 63;
;               |
;               |   + DO i2 = 0, (zext.i32.i64(%"block_$N_fetch.1") + -1)/u64, 1   <DO_LOOP>
;               |   |   %min4 = (-64 * i2 + zext.i32.i64(%"block_$N_fetch.1") + -1 <= 63) ? -64 * i2 + zext.i32.i64(%"block_$N_fetch.1") + -1 : 63;
;               |   |
;               |   |   + DO i3 = 0, (zext.i32.i64(%"block_$N_fetch.1") + -1)/u64, 1   <DO_LOOP>
;               |   |   |   %min5 = (-64 * i3 + zext.i32.i64(%"block_$N_fetch.1") + -1 <= 63) ? -64 * i3 + zext.i32.i64(%"block_$N_fetch.1") + -1 : 63;
;               |   |   |
; CHECK:        |   |   |   + DO i4 = 0, %min, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; CHECK:        |   |   |   |   + DO i5 = 0, %min4, 1   <DO_LOOP>  <MAX_TC_EST = 64>
; CHECK:        |   |   |   |   |   + DO i6 = 0, %min5, 1   <DO_LOOP>  <MAX_TC_EST = 64>
;               |   |   |   |   |   |   %"block_$B[][]_fetch.28" = (%"block_$B")[64 * i2 + i5][64 * i1 + i4];
;               |   |   |   |   |   |   %mul.7 = %"block_$B[][]_fetch.28"  *  (%"block_$A")[64 * i1 + i4][64 * i3 + i6];
;               |   |   |   |   |   |   %add.4 = %mul.7  +  (%"block_$C")[64 * i2 + i5][64 * i3 + i6];
;               |   |   |   |   |   |   (%"block_$C")[64 * i2 + i5][64 * i3 + i6] = %add.4;
;               |   |   |   |   |   + END LOOP
;               |   |   |   |   + END LOOP
;               |   |   |   + END LOOP
;               |   |   + END LOOP
;               |   + END LOOP
;               + END LOOP
;         END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @block_(ptr noalias nocapture readonly dereferenceable(4) %"block_$A", ptr noalias nocapture readonly dereferenceable(4) %"block_$B", ptr noalias nocapture dereferenceable(4) %"block_$C", ptr noalias nocapture readonly dereferenceable(4) %"block_$N") local_unnamed_addr #0 {
DIR.PRAGMA.BLOCK_LOOP.151:
  %"block_$N_fetch.1" = load i32, ptr %"block_$N", align 1, !tbaa !0
  %int_sext = sext i32 %"block_$N_fetch.1" to i64
  %mul.1 = shl nsw i64 %int_sext, 2
  br label %DIR.PRAGMA.BLOCK_LOOP.1

DIR.PRAGMA.BLOCK_LOOP.1:                          ; preds = %DIR.PRAGMA.BLOCK_LOOP.151
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.BLOCK_LOOP"(), "QUAL.PRAGMA.LEVEL"(i32 -1), "QUAL.PRAGMA.FACTOR"(i64 -1) ]
  br label %DIR.PRAGMA.BLOCK_LOOP.2

DIR.PRAGMA.BLOCK_LOOP.2:                          ; preds = %DIR.PRAGMA.BLOCK_LOOP.1
  %rel.1 = icmp slt i32 %"block_$N_fetch.1", 1
  br i1 %rel.1, label %DIR.PRAGMA.END.BLOCK_LOOP.252, label %bb2.preheader

bb2.preheader:                                    ; preds = %DIR.PRAGMA.BLOCK_LOOP.2
  %1 = add nuw nsw i32 %"block_$N_fetch.1", 1
  %wide.trip.count4953 = zext i32 %1 to i64
  br label %bb10.preheader.preheader

bb10.preheader.preheader:                         ; preds = %bb7, %bb2.preheader
  %indvars.iv47 = phi i64 [ 1, %bb2.preheader ], [ %indvars.iv.next48, %bb7 ]
  %"block_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"block_$A", i64 %indvars.iv47)
  br label %bb10.preheader

bb10.preheader:                                   ; preds = %bb10.preheader.preheader, %bb11
  %indvars.iv43 = phi i64 [ 1, %bb10.preheader.preheader ], [ %indvars.iv.next44, %bb11 ]
  %"block_$C[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"block_$C", i64 %indvars.iv43)
  %"block_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %mul.1, ptr nonnull elementtype(float) %"block_$B", i64 %indvars.iv43)
  %"block_$B[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"block_$B[]", i64 %indvars.iv47)
  %"block_$B[][]_fetch.28" = load float, ptr %"block_$B[][]", align 1, !tbaa !4
  br label %bb10

bb10:                                             ; preds = %bb10.preheader, %bb10
  %indvars.iv = phi i64 [ 1, %bb10.preheader ], [ %indvars.iv.next, %bb10 ]
  %"block_$C[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"block_$C[]", i64 %indvars.iv)
  %"block_$C[][]_fetch.14" = load float, ptr %"block_$C[][]", align 1, !tbaa !6
  %"block_$A[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"block_$A[]", i64 %indvars.iv)
  %"block_$A[][]_fetch.21" = load float, ptr %"block_$A[][]", align 1, !tbaa !8
  %mul.7 = fmul fast float %"block_$B[][]_fetch.28", %"block_$A[][]_fetch.21"
  %add.4 = fadd fast float %mul.7, %"block_$C[][]_fetch.14"
  store float %add.4, ptr %"block_$C[][]", align 1, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count4953
  br i1 %exitcond, label %bb11, label %bb10

bb11:                                             ; preds = %bb10
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %exitcond46 = icmp eq i64 %indvars.iv.next44, %wide.trip.count4953
  br i1 %exitcond46, label %bb7, label %bb10.preheader

bb7:                                              ; preds = %bb11
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond50 = icmp eq i64 %indvars.iv.next48, %wide.trip.count4953
  br i1 %exitcond50, label %DIR.PRAGMA.END.BLOCK_LOOP.252.loopexit, label %bb10.preheader.preheader

DIR.PRAGMA.END.BLOCK_LOOP.252.loopexit:           ; preds = %bb7
  br label %DIR.PRAGMA.END.BLOCK_LOOP.252

DIR.PRAGMA.END.BLOCK_LOOP.252:                    ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.252.loopexit, %DIR.PRAGMA.BLOCK_LOOP.2
  br label %DIR.PRAGMA.END.BLOCK_LOOP.3

DIR.PRAGMA.END.BLOCK_LOOP.3:                      ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.252
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.BLOCK_LOOP"() ]
  br label %DIR.PRAGMA.END.BLOCK_LOOP.4

DIR.PRAGMA.END.BLOCK_LOOP.4:                      ; preds = %DIR.PRAGMA.END.BLOCK_LOOP.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$block_"}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$7", !2, i64 0}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$5", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$6", !2, i64 0}


; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s
;
;  Loop cannot be vectorized because of lexically backward output DEP
;  do i=1, n
;     do j=1,n
;          a(j)    =  b(j)  ! cannot be vectorized    
;          a(j+1)  = b(j) + 1.0
;       enddo
;    enddo

; CHECK-NOT: <auto-vectorized> 

;Module Before HIR
; ModuleID = 'singleDimMultiLvlA.f90'
source_filename = "singleDimMultiLvlA.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture writeonly dereferenceable(4) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$B", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr #0 {
alloca_0:
  %"sub_$N_fetch.1" = load i32, ptr %"sub_$N", align 1, !tbaa !0
  %rel.1 = icmp slt i32 %"sub_$N_fetch.1", 1
  br i1 %rel.1, label %do.end_do3, label %do.body2.preheader

do.body2.preheader:                               ; preds = %alloca_0
  %0 = add nuw nsw i32 %"sub_$N_fetch.1", 1
  %wide.trip.count = sext i32 %0 to i64
  br label %do.body2

do.body2:                                         ; preds = %do.body2.preheader, %do.end_do7
  %"sub_$I.0" = phi i32 [ %add.4, %do.end_do7 ], [ 1, %do.body2.preheader ]
  br label %do.body6

do.body6:                                         ; preds = %do.body2, %do.body6
  %indvars.iv = phi i64 [ 1, %do.body2 ], [ %indvars.iv.next, %do.body6 ]
  %"sub_$B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$B", i64 %indvars.iv), !llfort.type_idx !5
  %"sub_$B[]_fetch.6" = load float, ptr %"sub_$B[]", align 1, !tbaa !6
  %"sub_$A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv), !llfort.type_idx !5
  store float %"sub_$B[]_fetch.6", ptr %"sub_$A[]", align 1, !tbaa !8
  %add.1 = fadd reassoc ninf nsz arcp contract afn float %"sub_$B[]_fetch.6", 1.000000e+00
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"sub_$A[]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"sub_$A", i64 %indvars.iv.next), !llfort.type_idx !5
  store float %add.1, ptr %"sub_$A[]6", align 1, !tbaa !8
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %do.end_do7, label %do.body6

do.end_do7:                                       ; preds = %do.body6
  %add.4 = add nuw nsw i32 %"sub_$I.0", 1
  %exitcond15 = icmp eq i32 %add.4, %0
  br i1 %exitcond15, label %do.end_do3.loopexit, label %do.body2

do.end_do3.loopexit:                              ; preds = %do.end_do7
  br label %do.end_do3

do.end_do3:                                       ; preds = %do.end_do3.loopexit, %alloca_0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Fortran Data Symbol", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$sub_"}
!5 = !{i64 5}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$4", !2, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$5", !2, i64 0}

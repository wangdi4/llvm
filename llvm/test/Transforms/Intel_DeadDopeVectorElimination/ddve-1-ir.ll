; RUN: opt < %s -passes='deaddopevectorelimination,simplifycfg' -whole-program-assume -S 2>&1 | FileCheck %s

;
; The test checks that compiler is able to detect and remove dead dopevector
; in the following program.  hssTid - is a dead dopevector since it is just
; allocated, deallocated and never used.
;
;      MODULE mod_ncparam
;        integer, allocatable :: hssTid(:,:)   ! Hessian tracers IDs
;      CONTAINS
;
;        SUBROUTINE initialize_ncparam
;          allocate ( hssTid(3,100) )
;          deallocate ( hssTid )
;          RETURN
;        END SUBROUTINE initialize_ncparam
;
;      END MODULE mod_ncparam
;
;      PROGRAM TEST
;      USE mod_ncparam
;      call initialize_ncparam()
;      END PROGRAM TEST
;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$i32*$rank2$" = type { ptr, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

@llvm.compiler.used = appending global [2 x ptr] [ptr @__intel_new_feature_proc_init, ptr @__intel_new_feature_proc_init], section "llvm.metadata"
@mod_ncparam_mp_hsstid_ = internal global %"QNCA_a0$i32*$rank2$" { ptr null, i64 0, i64 0, i64 1342177408, i64 2, i64 0, [2 x { i64, i64, i64 }] zeroinitializer }
@anon.ceabb6bb8474018766b9b959de769cfa.0 = internal unnamed_addr constant i32 65536, align 4
@anon.ceabb6bb8474018766b9b959de769cfa.1 = internal unnamed_addr constant i32 2, align 4

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: nofree nounwind uwtable
define internal void @mod_ncparam_mp_initialize_ncparam_() #1 {
; CHECK-LABEL: define internal void @mod_ncparam_mp_initialize_ncparam_
; CHECK-NEXT:    ret void
  %1 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 3), align 8, !tbaa !3, !llfort.type_idx !9
  %2 = and i64 %1, 1030792151296
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 5), align 8, !tbaa !10
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 1), align 8, !tbaa !11
  store i64 2, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 4), align 16, !tbaa !12
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 2), align 16, !tbaa !13
  %3 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0, i32 2), i32 0), !llfort.type_idx !14
  store i64 1, ptr %3, align 1, !tbaa !15
  %4 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0), i32 0), !llfort.type_idx !16
  store i64 3, ptr %4, align 1, !tbaa !17
  %5 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0, i32 1), i32 0), !llfort.type_idx !18
  store i64 4, ptr %5, align 1, !tbaa !19
  %6 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0, i32 2), i32 1), !llfort.type_idx !14
  store i64 1, ptr %6, align 1, !tbaa !15
  %7 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0), i32 1), !llfort.type_idx !16
  store i64 100, ptr %7, align 1, !tbaa !17
  %8 = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 6, i64 0, i32 1), i32 1), !llfort.type_idx !18
  store i64 12, ptr %8, align 1, !tbaa !19
  %9 = or i64 %2, 1342177413
  store i64 %9, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 3), align 8, !tbaa !3
  %10 = lshr i64 %2, 15
  %11 = trunc i64 %10 to i32
  %12 = or i32 %11, 327682
  %13 = tail call i32 @for_alloc_allocatable_handle(i64 1200, ptr nonnull @mod_ncparam_mp_hsstid_, i32 %12, ptr null) #5, !llfort.type_idx !20
  %14 = load ptr, ptr @mod_ncparam_mp_hsstid_, align 16, !tbaa !21, !llfort.type_idx !20
  %15 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 3), align 8, !tbaa !3, !llfort.type_idx !9
  %16 = trunc i64 %15 to i32
  %17 = shl i32 %16, 1
  %18 = and i32 %17, 6
  %19 = lshr i32 %16, 3
  %20 = and i32 %19, 256
  %21 = lshr i64 %15, 15
  %22 = trunc i64 %21 to i32
  %23 = and i32 %22, 65011712
  %24 = or i32 %20, %18
  %25 = or i32 %24, %23
  %26 = or i32 %25, 327680
  %27 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 5), align 8, !tbaa !10, !llfort.type_idx !22
  %28 = inttoptr i64 %27 to ptr, !llfort.type_idx !23
  %29 = tail call i32 @for_dealloc_allocatable_handle(ptr %14, i32 %26, ptr %28) #5, !llfort.type_idx !20
  %30 = icmp eq i32 %29, 0
  br i1 %30, label %31, label %34

31:                                               ; preds = %0
  store ptr null, ptr @mod_ncparam_mp_hsstid_, align 16, !tbaa !21
  %32 = load i64, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 3), align 8, !tbaa !3, !llfort.type_idx !9
  %33 = and i64 %32, -1030792153090
  store i64 %33, ptr getelementptr inbounds (%"QNCA_a0$i32*$rank2$", ptr @mod_ncparam_mp_hsstid_, i64 0, i32 3), align 8, !tbaa !3
  br label %34

34:                                               ; preds = %31, %0
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nofree
declare !llfort.intrin_id !24 dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #3

; Function Attrs: nofree
declare !llfort.intrin_id !25 dso_local i32 @for_dealloc_allocatable_handle(ptr nocapture readonly, i32, ptr) local_unnamed_addr #3

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #4 {
  %1 = tail call i32 @for_set_fpe_(ptr nonnull @anon.ceabb6bb8474018766b9b959de769cfa.0) #5, !llfort.type_idx !20
  %2 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.ceabb6bb8474018766b9b959de769cfa.1) #5, !llfort.type_idx !20
  tail call void @mod_ncparam_mp_initialize_ncparam_() #5, !llfort.type_idx !26
  ret void
}

declare !llfort.intrin_id !27 dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !28 dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #3

attributes #0 = { nofree nounwind }
attributes #1 = { nofree nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt"  "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt"  "unsafe-fp-math"="true" }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}
!ifx.types.dv = !{!29}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4, !5, i64 24}
!4 = !{!"ifx$descr$1", !5, i64 0, !5, i64 8, !5, i64 16, !5, i64 24, !5, i64 32, !5, i64 40, !5, i64 48, !5, i64 56, !5, i64 64, !5, i64 72, !5, i64 80, !5, i64 88}
!5 = !{!"ifx$descr$field", !6, i64 0}
!6 = !{!"Fortran Dope Vector Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$mod_ncparam_mp_initialize_ncparam_"}
!9 = !{i64 29}
!10 = !{!4, !5, i64 40}
!11 = !{!4, !5, i64 8}
!12 = !{!4, !5, i64 32}
!13 = !{!4, !5, i64 16}
!14 = !{i64 20}
!15 = !{!4, !5, i64 64}
!16 = !{i64 18}
!17 = !{!4, !5, i64 48}
!18 = !{i64 19}
!19 = !{!4, !5, i64 56}
!20 = !{i64 2}
!21 = !{!4, !5, i64 0}
!22 = !{i64 31}
!23 = !{i64 11}
!24 = !{i32 94}
!25 = !{i32 95}
!26 = !{i64 16}
!27 = !{i32 97}
!28 = !{i32 98}
!29 = !{%"QNCA_a0$i32*$rank2$" zeroinitializer, i32 0}


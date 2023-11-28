; REQUIRES: asserts
; RUN: opt < %s -passes='deaddopevectorelimination' --debug-only=deaddopevectorelimination -whole-program-assume -disable-output 2>&1 | FileCheck %s
;
; The test checks that dead dopevector is not triggered in the following program.
;
;      program cq415310_memkind
;      use ifcore
;      implicit none
;      real, allocatable :: r(:)
;      integer(4)        :: i4_iostat
;
;      write(*, '("FOR_K_MEMKIND_DDR = ", I0, ", FOR_K_MEMKIND_HBW = ", I0)') FOR_K_MEMKIND_DDR, FOR_K_MEMKIND_HBW
;
;      !dir$ attributes memkind:hbw :: r
;      write(*, '("for_get_memkind(r) = ", I0, " (should be 1 for HBW)")') 1 !! ??? for_get_memkind(r)
;
;      !dir$ memkind:ddr
;
;      allocate(r(1000), stat=i4_iostat) !! will likely fail on linux because we need FASTMEM for it to work
;      write(*, '("for_get_memkind(r) = ", I0, " (should be 0 for DDR)")') 0 !! ??? for_get_memkind(r)
;      write(*, '("PASSED")')
;      end program
;

; CHECK: Start DeadDopeVectorElimination
; CHECK-NOT: Global Dope Vector{{.*}} was deleted
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$float*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"cq415310_memkind_$R" = internal global %"QNCA_a0$float*$rank1$" { ptr null, i64 0, i64 0, i64 69793218944, i64 1, i64 0, [1 x { i64, i64, i64 }] zeroinitializer }
@"cq415310_memkind_$format_pack" = internal unnamed_addr global [256 x i8] c"6\00\00\00\1C\00\14\00FOR_K_MEMKIND_DDR = $\00\00\00\01\00\00\00\00\00\00\00\1C\00\16\00, FOR_K_MEMKIND_HBW = \00\00$\00\00\00\01\00\00\00\00\00\00\007\00\00\006\00\00\00\1C\00\15\00for_get_memkind(r) = \00\00\00$\00\00\00\01\00\00\00\00\00\00\00\1C\00\16\00 (should be 1 for HBW)\00\007\00\00\006\00\00\00\1C\00\15\00for_get_memkind(r) = \00\00\00$\00\00\00\01\00\00\00\00\00\00\00\1C\00\16\00 (should be 0 for DDR)\00\007\00\00\006\00\00\00\1C\00\06\00PASSED\00\007\00\00\00", align 4
@anon.68ba48b9c6c80ce889c10c7426f57970.0 = internal unnamed_addr constant i32 65536, align 4
@anon.68ba48b9c6c80ce889c10c7426f57970.1 = internal unnamed_addr constant i32 2, align 4

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() #0 {
  %1 = alloca [8 x i64], align 16, !llfort.type_idx !3
  %2 = alloca [4 x i8], align 1, !llfort.type_idx !4
  %3 = alloca <{ i64 }>, align 8, !llfort.type_idx !5
  %4 = alloca [4 x i8], align 1, !llfort.type_idx !6
  %5 = alloca <{ i64 }>, align 8, !llfort.type_idx !5
  %6 = alloca [4 x i8], align 1, !llfort.type_idx !7
  %7 = alloca <{ i64 }>, align 8, !llfort.type_idx !5
  %8 = alloca [4 x i8], align 1, !llfort.type_idx !8
  %9 = alloca <{ i64 }>, align 8, !llfort.type_idx !5
  %10 = alloca [2 x i8], align 1, !llfort.type_idx !9
  %11 = tail call i32 @for_set_fpe_(ptr nonnull @anon.68ba48b9c6c80ce889c10c7426f57970.0) #3, !llfort.type_idx !10
  %12 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.68ba48b9c6c80ce889c10c7426f57970.1) #3, !llfort.type_idx !10
  store i64 0, ptr %1, align 16, !tbaa !11
  %13 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 0
  store i8 9, ptr %13, align 1, !tbaa !11
  %14 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 1
  store i8 1, ptr %14, align 1, !tbaa !11
  %15 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 2
  store i8 2, ptr %15, align 1, !tbaa !11
  %16 = getelementptr inbounds [4 x i8], ptr %2, i64 0, i64 3
  store i8 0, ptr %16, align 1, !tbaa !11
  %17 = getelementptr inbounds <{ i64 }>, ptr %3, i64 0, i32 0, !llfort.type_idx !15
  store i32 0, ptr %17, align 8, !tbaa !16
  %18 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %2, ptr nonnull %3, ptr nonnull @"cq415310_memkind_$format_pack") #3, !llfort.type_idx !10
  %19 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 0
  store i8 9, ptr %19, align 1, !tbaa !11
  %20 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 1
  store i8 1, ptr %20, align 1, !tbaa !11
  %21 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 2
  store i8 1, ptr %21, align 1, !tbaa !11
  %22 = getelementptr inbounds [4 x i8], ptr %4, i64 0, i64 3
  store i8 0, ptr %22, align 1, !tbaa !11
  %23 = getelementptr inbounds <{ i64 }>, ptr %5, i64 0, i32 0, !llfort.type_idx !18
  store i32 1, ptr %23, align 8, !tbaa !19
  %24 = call i32 @for_write_seq_fmt_xmit(ptr nonnull %1, ptr nonnull %4, ptr nonnull %5) #3, !llfort.type_idx !10
  store i64 0, ptr %1, align 16, !tbaa !11
  %25 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 0
  store i8 9, ptr %25, align 1, !tbaa !11
  %26 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 1
  store i8 1, ptr %26, align 1, !tbaa !11
  %27 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 2
  store i8 1, ptr %27, align 1, !tbaa !11
  %28 = getelementptr inbounds [4 x i8], ptr %6, i64 0, i64 3
  store i8 0, ptr %28, align 1, !tbaa !11
  %29 = getelementptr inbounds <{ i64 }>, ptr %7, i64 0, i32 0, !llfort.type_idx !21
  store i32 1, ptr %29, align 8, !tbaa !22
  %30 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %6, ptr nonnull %7, ptr nonnull getelementptr inbounds ([256 x i8], ptr @"cq415310_memkind_$format_pack", i64 0, i64 84)) #3, !llfort.type_idx !10
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 5), align 8, !tbaa !24
  %31 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 3), align 8, !tbaa !28, !llfort.type_idx !29
  %32 = and i64 %31, -1099243192321
  %33 = or i64 %32, 1073741824
  store i64 %33, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 3), align 8, !tbaa !28
  %34 = trunc i64 %31 to i32
  %35 = shl i32 %34, 1
  %36 = and i32 %35, 2
  %37 = lshr i64 %31, 15
  %38 = trunc i64 %37 to i32
  %39 = and i32 %38, 65011712
  %40 = or i32 %39, %36
  %41 = or i32 %40, 262145
  %42 = call i32 @for_alloc_allocatable_handle(i64 4000, ptr nonnull @"cq415310_memkind_$R", i32 %41, ptr null) #3, !llfort.type_idx !10
  %43 = icmp eq i32 %42, 0
  br i1 %43, label %44, label %51

44:                                               ; preds = %0
  %45 = load i64, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 3), align 8, !tbaa !28, !llfort.type_idx !29
  %46 = and i64 %45, 1030792151296
  %47 = or i64 %46, 133
  store i64 %47, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 3), align 8, !tbaa !28
  store i64 4, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 1), align 8, !tbaa !30
  store i64 1, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 4), align 16, !tbaa !31
  store i64 0, ptr getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 2), align 16, !tbaa !32
  %48 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 6, i64 0, i32 2), i32 0), !llfort.type_idx !33
  store i64 1, ptr %48, align 1, !tbaa !34
  %49 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 6, i64 0), i32 0), !llfort.type_idx !35
  store i64 1000, ptr %49, align 1, !tbaa !36
  %50 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) getelementptr inbounds (%"QNCA_a0$float*$rank1$", ptr @"cq415310_memkind_$R", i64 0, i32 6, i64 0, i32 1), i32 0), !llfort.type_idx !37
  store i64 4, ptr %50, align 1, !tbaa !38
  br label %51

51:                                               ; preds = %44, %0
  store i64 0, ptr %1, align 16, !tbaa !11
  %52 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 0
  store i8 9, ptr %52, align 1, !tbaa !11
  %53 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 1
  store i8 1, ptr %53, align 1, !tbaa !11
  %54 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 2
  store i8 1, ptr %54, align 1, !tbaa !11
  %55 = getelementptr inbounds [4 x i8], ptr %8, i64 0, i64 3
  store i8 0, ptr %55, align 1, !tbaa !11
  %56 = getelementptr inbounds <{ i64 }>, ptr %9, i64 0, i32 0, !llfort.type_idx !39
  store i32 0, ptr %56, align 8, !tbaa !40
  %57 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %8, ptr nonnull %9, ptr nonnull getelementptr inbounds ([256 x i8], ptr @"cq415310_memkind_$format_pack", i64 0, i64 160)) #3, !llfort.type_idx !10
  store i64 0, ptr %1, align 16, !tbaa !11
  %58 = getelementptr inbounds [2 x i8], ptr %10, i64 0, i64 0
  store i8 1, ptr %58, align 1, !tbaa !11
  %59 = getelementptr inbounds [2 x i8], ptr %10, i64 0, i64 1
  store i8 0, ptr %59, align 1, !tbaa !11
  %60 = call i32 (ptr, i32, i64, ptr, ptr, ptr, ...) @for_write_seq_fmt(ptr nonnull %1, i32 -1, i64 2253038970797824, ptr nonnull %10, ptr null, ptr nonnull getelementptr inbounds ([256 x i8], ptr @"cq415310_memkind_$format_pack", i64 0, i64 236)) #3, !llfort.type_idx !10
  ret void
}

declare !llfort.intrin_id !42 dso_local i32 @for_set_fpe_(ptr nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !43 dso_local i32 @for_set_reentrancy(ptr nocapture readonly) local_unnamed_addr #1

; Function Attrs: nofree
declare !llfort.intrin_id !44 dso_local i32 @for_write_seq_fmt(ptr, i32, i64, ptr, ptr, ptr, ...) local_unnamed_addr #1

; Function Attrs: nofree
declare !llfort.intrin_id !45 dso_local i32 @for_write_seq_fmt_xmit(ptr nocapture readonly, ptr nocapture readonly, ptr) local_unnamed_addr #1

; Function Attrs: nofree
declare !llfort.intrin_id !46 dso_local i32 @for_alloc_allocatable_handle(i64, ptr nocapture, i32, ptr) local_unnamed_addr #1

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nofree "intel-lang"="fortran" }
attributes #2 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}
!ifx.types.dv = !{!47}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{i64 41}
!4 = !{i64 65}
!5 = !{i64 69}
!6 = !{i64 72}
!7 = !{i64 76}
!8 = !{i64 83}
!9 = !{i64 87}
!10 = !{i64 2}
!11 = !{!12, !12, i64 0}
!12 = !{!"Fortran Data Symbol", !13, i64 0}
!13 = !{!"Generic Fortran Symbol", !14, i64 0}
!14 = !{!"ifx$root$1$MAIN__"}
!15 = !{i64 70}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$1", !12, i64 0}
!18 = !{i64 74}
!19 = !{!20, !20, i64 0}
!20 = !{!"ifx$unique_sym$2", !12, i64 0}
!21 = !{i64 77}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$3", !12, i64 0}
!24 = !{!25, !26, i64 40}
!25 = !{!"ifx$descr$1", !26, i64 0, !26, i64 8, !26, i64 16, !26, i64 24, !26, i64 32, !26, i64 40, !26, i64 48, !26, i64 56, !26, i64 64}
!26 = !{!"ifx$descr$field", !27, i64 0}
!27 = !{!"Fortran Dope Vector Symbol", !13, i64 0}
!28 = !{!25, !26, i64 24}
!29 = !{i64 29}
!30 = !{!25, !26, i64 8}
!31 = !{!25, !26, i64 32}
!32 = !{!25, !26, i64 16}
!33 = !{i64 20}
!34 = !{!25, !26, i64 64}
!35 = !{i64 18}
!36 = !{!25, !26, i64 48}
!37 = !{i64 19}
!38 = !{!25, !26, i64 56}
!39 = !{i64 84}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$5", !12, i64 0}
!42 = !{i32 97}
!43 = !{i32 98}
!44 = !{i32 334}
!45 = !{i32 335}
!46 = !{i32 94}
!47 = !{%"QNCA_a0$float*$rank1$" zeroinitializer, float 0.000000e+00}

; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -lto-inline-cost -inlining-dyn-alloca-adjustable=true -inlining-dyn-alloca-adjustable-max-count=1  -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -lto-inline-cost -inlining-dyn-alloca-adjustable=true -inlining-dyn-alloca-adjustable-max-count=1 -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; Check that a single callsite function can not be inlined on the link step of an
; -flto compilation if it contains too many local Fortran adjustable arrays.

; CHECK-LABEL: COMPILE FUNC: MAIN__
; CHECK: EXTERN: for_set_fpe_
; CHECK: EXTERN: for_set_reentrancy
; CHECK: foo_ {{.*}}Callee has dynamic alloca

target triple = "x86_64-unknown-linux-gnu"

@"main_$MYARRAY" = internal global [10 x [10 x i32]] zeroinitializer, align 16, !llfort.type_idx !0
@0 = internal unnamed_addr constant i32 65536
@1 = internal unnamed_addr constant i32 2
@2 = internal unnamed_addr constant i32 10

; Function Attrs: nounwind uwtable
define internal void @foo_(i32* noalias dereferenceable(4) %"foo_$MYARRAY", i32* noalias dereferenceable(4) %"foo_$MYN") local_unnamed_addr #0 !llfort.type_idx !3 {
alloca_0:
  %"foo_$MYN_fetch.3" = load i32, i32* %"foo_$MYN", align 1, !tbaa !4, !llfort.type_idx !9
  %int_sext = sext i32 %"foo_$MYN_fetch.3" to i64, !llfort.type_idx !10
  %0 = call i64 @llvm.smax.i64(i64 %int_sext, i64 0)
  %mul.1 = shl nsw i64 %0, 2
  %mul.2 = mul nsw i64 %mul.1, %0
  %div.148 = lshr exact i64 %mul.2, 2
  %"foo_$MYLOCALARRAY4" = alloca i32, i64 %div.148, align 4, !llfort.type_idx !11
  %"foo_$MYLOCALARRAY5" = alloca i32, i64 %div.148, align 4, !llfort.type_idx !11
  %"foo_$MYLOCALARRAY6" = alloca i32, i64 %div.148, align 4, !llfort.type_idx !11
  %mul.3 = shl nsw i64 %int_sext, 2
  %"foo_$MYN_fetch.4" = load i32, i32* %"foo_$MYN", align 1, !tbaa !4
  %rel.3 = icmp slt i32 %"foo_$MYN_fetch.4", 1
  br i1 %rel.3, label %bb10, label %bb1

bb1thread-pre-split:                              ; preds = %bb6
  %"foo_$MYN_fetch.6.pr" = load i32, i32* %"foo_$MYN", align 1, !tbaa !4
  br label %bb1

bb1:                                              ; preds = %bb1thread-pre-split, %alloca_0
  %"foo_$MYN_fetch.6" = phi i32 [ %"foo_$MYN_fetch.6.pr", %bb1thread-pre-split ], [ %"foo_$MYN_fetch.4", %alloca_0 ]
  %"foo_$I.0" = phi i32 [ 1, %alloca_0 ], [ %add.4, %bb1thread-pre-split ]
  %rel.4 = icmp slt i32 %"foo_$MYN_fetch.6", 1
  br i1 %rel.4, label %bb6, label %bb5

bb5:                                              ; preds = %bb1, %bb5
  %"foo_$J.0" = phi i32 [ 1, %bb1 ], [ %add.3, %bb5 ]
  %int_sext9 = zext i32 %"foo_$I.0" to i64
  %int_sext10 = zext i32 %"foo_$J.0" to i64
  %"foo_$MYARRAY[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.3, i32* nonnull elementtype(i32) %"foo_$MYARRAY", i64 %int_sext10), !llfort.type_idx !12
  %"foo_$MYARRAY[][]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"foo_$MYARRAY[]", i64 %int_sext9), !llfort.type_idx !13
  %"foo_$MYARRAY[][]_fetch.14" = load i32, i32* %"foo_$MYARRAY[][]", align 1, !tbaa !14, !llfort.type_idx !13
  %"foo_$MYLOCALARRAY4[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.3, i32* nonnull elementtype(i32) %"foo_$MYLOCALARRAY4", i64 %int_sext10), !llfort.type_idx !16
  %"foo_$MYLOCALARRAY4[][]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"foo_$MYLOCALARRAY4[]", i64 %int_sext9), !llfort.type_idx !17
  store i32 %"foo_$MYARRAY[][]_fetch.14", i32* %"foo_$MYLOCALARRAY4[][]", align 4, !tbaa !18
  %add.3 = add nuw nsw i32 %"foo_$J.0", 1
  %rel.5.not = icmp sgt i32 %add.3, %"foo_$MYN_fetch.6"
  br i1 %rel.5.not, label %bb6, label %bb5

bb6:                                              ; preds = %bb5, %bb1
  %add.4 = add nuw nsw i32 %"foo_$I.0", 1
  %rel.6.not = icmp sgt i32 %add.4, %"foo_$MYN_fetch.4"
  br i1 %rel.6.not, label %bb2, label %bb1thread-pre-split

bb2:                                              ; preds = %bb6
  %"foo_$MYN_fetch.27.pr" = load i32, i32* %"foo_$MYN", align 1, !tbaa !4
  %rel.7 = icmp slt i32 %"foo_$MYN_fetch.27.pr", 1
  br i1 %rel.7, label %bb10, label %bb9

bb9thread-pre-split:                              ; preds = %bb14
  %"foo_$MYN_fetch.29.pr" = load i32, i32* %"foo_$MYN", align 1, !tbaa !4
  br label %bb9

bb9:                                              ; preds = %bb9thread-pre-split, %bb2
  %"foo_$MYN_fetch.29" = phi i32 [ %"foo_$MYN_fetch.29.pr", %bb9thread-pre-split ], [ %"foo_$MYN_fetch.27.pr", %bb2 ]
  %"foo_$I.1" = phi i32 [ 1, %bb2 ], [ %add.6, %bb9thread-pre-split ]
  %rel.8 = icmp slt i32 %"foo_$MYN_fetch.29", 1
  br i1 %rel.8, label %bb14, label %bb13

bb13:                                             ; preds = %bb9, %bb13
  %"foo_$J.1" = phi i32 [ 1, %bb9 ], [ %add.5, %bb13 ]
  %int_sext13 = zext i32 %"foo_$I.1" to i64
  %int_sext14 = zext i32 %"foo_$J.1" to i64
  %"foo_$MYLOCALARRAY4[]15" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.3, i32* nonnull elementtype(i32) %"foo_$MYLOCALARRAY4", i64 %int_sext14), !llfort.type_idx !20
  %"foo_$MYLOCALARRAY4[][]16" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"foo_$MYLOCALARRAY4[]15", i64 %int_sext13), !llfort.type_idx !21
  %"foo_$MYLOCALARRAY4[][]_fetch.35" = load i32, i32* %"foo_$MYLOCALARRAY4[][]16", align 4, !tbaa !18, !llfort.type_idx !21
  %"foo_$MYARRAY[]19" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %mul.3, i32* nonnull elementtype(i32) %"foo_$MYARRAY", i64 %int_sext14), !llfort.type_idx !22
  %"foo_$MYARRAY[][]20" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"foo_$MYARRAY[]19", i64 %int_sext13), !llfort.type_idx !23
  store i32 %"foo_$MYLOCALARRAY4[][]_fetch.35", i32* %"foo_$MYARRAY[][]20", align 1, !tbaa !14
  %add.5 = add nuw nsw i32 %"foo_$J.1", 1
  %rel.9.not = icmp sgt i32 %add.5, %"foo_$MYN_fetch.29"
  br i1 %rel.9.not, label %bb14, label %bb13

bb14:                                             ; preds = %bb13, %bb9
  %add.6 = add nuw nsw i32 %"foo_$I.1", 1
  %rel.10.not = icmp sgt i32 %add.6, %"foo_$MYN_fetch.27.pr"
  br i1 %rel.10.not, label %bb10, label %bb9thread-pre-split

bb10:                                             ; preds = %alloca_0, %bb14, %bb2
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare !llfort.type_idx !24 i8* @llvm.stacksave() #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare !llfort.type_idx !25 void @llvm.stackrestore(i8*) #1

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 !llfort.type_idx !26 {
alloca_1:
  %func_result = call i32 @for_set_fpe_(i32* nonnull @0) #5, !llfort.type_idx !27
  %func_result2 = call i32 @for_set_reentrancy(i32* nonnull @1) #5, !llfort.type_idx !27
  call void @foo_(i32* nonnull getelementptr inbounds ([10 x [10 x i32]], [10 x [10 x i32]]* @"main_$MYARRAY", i64 0, i64 0, i64 0), i32* nonnull @2), !llfort.type_idx !28
  ret void
}

declare !llfort.intrin_id !29 !llfort.type_idx !30 i32 @for_set_fpe_(i32* nocapture readonly) local_unnamed_addr

; Function Attrs: nofree
declare !llfort.intrin_id !31 !llfort.type_idx !32 i32 @for_set_reentrancy(i32* nocapture readonly) local_unnamed_addr #3

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i64 @llvm.smax.i64(i64, i64) #4

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nofree nosync nounwind readnone speculatable }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #5 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!1, !2}

!0 = !{i64 58}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!3 = !{i64 31}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$1", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$foo_"}
!9 = !{i64 39}
!10 = !{i64 3}
!11 = !{i64 36}
!12 = !{i64 42}
!13 = !{i64 43}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$4", !6, i64 0}
!16 = !{i64 44}
!17 = !{i64 45}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$5", !6, i64 0}
!20 = !{i64 48}
!21 = !{i64 49}
!22 = !{i64 50}
!23 = !{i64 51}
!24 = !{i64 37}
!25 = !{i64 52}
!26 = !{i64 63}
!27 = !{i64 2}
!28 = !{i64 16}
!29 = !{i32 100}
!30 = !{i64 64}
!31 = !{i32 101}
!32 = !{i64 66}
; end INTEL_FEATURE_SW_ADVANCED


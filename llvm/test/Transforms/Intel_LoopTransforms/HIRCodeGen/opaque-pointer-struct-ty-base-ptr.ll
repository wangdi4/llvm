; RUN: opt < %s -enable-new-pm=0 -hir-ssa-deconstruction -hir-temp-cleanup -hir-cost-model-throttling=0 -hir-cg -force-hir-cg -print-after=hir-temp-cleanup,hir-cg -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-cg" -hir-cost-model-throttling=0 -force-hir-cg -print-after=hir-temp-cleanup,hir-cg -disable-output 2>&1 | FileCheck %s

; Verify that the code is generated with a crash. Notice that the input is by default using opaque pointers (i.e. "ptr").
; Option -opaque-pointers is not needed.

; CHECK:              + DO i1 = 0, zext.i32.i64(%0) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 441>  <LEGAL_MAX_TC = 2147483647>
; CHECK:              |   %1 = (%this)[0].5.0[i1];
; CHECK:              |   if (%1 != 3)
; CHECK:              |   {
; CHECK:              |      %res.020 = (@_ZN7Zobrist7zobristE)[0].0[%1].0[i1]  ^  %res.020;
; CHECK:              |   }
; CHECK:              + END LOOP

; CHECK: region.0:                                         ; preds = %for.body
; CHECK:   [[TMP5:%.*]] = load i64, ptr %i1.i64, align 8
; CHECK:   [[TMP6:%.*]] = getelementptr inbounds %class.FastBoard, ptr %this, i64 0, i32 5, i32 0, i64 [[TMP5]]
; CHECK:   %gepload = load i32, ptr [[TMP6]], align 4, !tbaa !27
; CHECK:   store i32 %gepload, ptr [[TMP9:%.*]], align 4
; CHECK:   [[LOAD1:%.*]] = load i32, ptr [[TMP9]], align 4
;  %hir.cmp.5 = icmp ne i32 %t9., 3
;  br i1 %hir.cmp.5, label %then.5, label %ifmerge.5

; CHECK:  [[TMP9LOAD:%.*]] = load i32, ptr [[TMP9]], align 4
; CHECK:  [[TMP7:%.*]] = zext i32 [[TMP9LOAD]] to i64
; CHECK:  [[TMP8:%.*]] = load i64, ptr %i1.i64, align 8
; CHECK:  [[T9:%.*]] = getelementptr inbounds %"class.boost::array.5", ptr @_ZN7Zobrist7zobristE, i64 0, i32 0, i64 [[TMP7]], i32 0, i64 [[TMP8]]
; CHECK:  [[Load:%.*]] = load i64, ptr [[T9]], align 8, !tbaa !28

; ModuleID = 'module'
source_filename = "FullBoard.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.boost::array.5" = type { [4 x %"class.boost::array.6"] }
%"class.boost::array.6" = type { [441 x i64] }
%class.FastBoard = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32, [4 x i8] }>
%"class.boost::array.0" = type { [441 x i32] }
%"class.boost::array.1" = type { [442 x i16] }
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.2" = type { [4 x i32] }
%"class.boost::array.3" = type { [8 x i32] }
%"class.boost::array.4" = type { [2 x i32] }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl" = type { %"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" }
%"struct.std::_Vector_base<int, std::allocator<int>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%class.FullBoard = type { %class.FastBoard.base, i64, i64 }
%class.FastBoard.base = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32 }>

@_ZN7Zobrist7zobristE = external dso_local local_unnamed_addr global %"class.boost::array.5", align 8

; Function Attrs: mustprogress nofree nosync nounwind uwtable
define dso_local noundef i64 @_ZN9FullBoard12calc_ko_hashEv(ptr nocapture noundef nonnull align 8 dereferenceable(8072) %this) local_unnamed_addr #0 align 2 {
entry:
  %m_maxsq = getelementptr inbounds %class.FastBoard, ptr %this, i64 0, i32 4, !intel-tbaa !3
  %0 = load i32, ptr %m_maxsq, align 4, !tbaa !3
  %cmp19 = icmp sgt i32 %0, 0
  br i1 %cmp19, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count22 = zext i32 %0 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %res.1.lcssa = phi i64 [ %res.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %res.0.lcssa = phi i64 [ 1311768467139281697, %entry ], [ %res.1.lcssa, %for.cond.cleanup.loopexit ]
  %ko_hash = getelementptr inbounds %class.FullBoard, ptr %this, i64 0, i32 2, !intel-tbaa !23
  store i64 %res.0.lcssa, ptr %ko_hash, align 8, !tbaa !23
  ret i64 %res.0.lcssa

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %res.020 = phi i64 [ 1311768467139281697, %for.body.preheader ], [ %res.1, %for.inc ]
  %arrayidx.i = getelementptr inbounds %class.FastBoard, ptr %this, i64 0, i32 5, i32 0, i64 %indvars.iv, !intel-tbaa !26
  %1 = load i32, ptr %arrayidx.i, align 4, !tbaa !26
  %cmp2.not = icmp eq i32 %1, 3
  br i1 %cmp2.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %conv6 = zext i32 %1 to i64
  %arrayidx.i16 = getelementptr inbounds %"class.boost::array.5", ptr @_ZN7Zobrist7zobristE, i64 0, i32 0, i64 %conv6, i32 0, i64 %indvars.iv, !intel-tbaa !27
  %2 = load i64, ptr %arrayidx.i16, align 8, !tbaa !27
  %xor = xor i64 %2, %res.020
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %res.1 = phi i64 [ %xor, %if.then ], [ %res.020, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count22
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !32
}

attributes #0 = { mustprogress nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !10, i64 1772}
!4 = !{!"struct@_ZTS9FastBoard", !5, i64 0, !5, i64 882, !10, i64 1764, !10, i64 1768, !10, i64 1772, !11, i64 1776, !14, i64 3540, !14, i64 4424, !14, i64 5308, !14, i64 6192, !5, i64 7076, !16, i64 7960, !18, i64 7976, !20, i64 8008, !20, i64 8016, !22, i64 8024, !10, i64 8048}
!5 = !{!"struct@_ZTSN5boost5arrayItLm441EEE", !6, i64 0}
!6 = !{!"array@_ZTSA441_t", !7, i64 0}
!7 = !{!"short", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!"int", !8, i64 0}
!11 = !{!"struct@_ZTSN5boost5arrayIN9FastBoard8square_tELm441EEE", !12, i64 0}
!12 = !{!"array@_ZTSA441_N9FastBoard8square_tE", !13, i64 0}
!13 = !{!"_ZTSN9FastBoard8square_tE", !8, i64 0}
!14 = !{!"struct@_ZTSN5boost5arrayItLm442EEE", !15, i64 0}
!15 = !{!"array@_ZTSA442_t", !7, i64 0}
!16 = !{!"struct@_ZTSN5boost5arrayIiLm4EEE", !17, i64 0}
!17 = !{!"array@_ZTSA4_i", !10, i64 0}
!18 = !{!"struct@_ZTSN5boost5arrayIiLm8EEE", !19, i64 0}
!19 = !{!"array@_ZTSA8_i", !10, i64 0}
!20 = !{!"struct@_ZTSN5boost5arrayIiLm2EEE", !21, i64 0}
!21 = !{!"array@_ZTSA2_i", !10, i64 0}
!22 = !{!"struct@_ZTSSt6vectorIiSaIiEE"}
!23 = !{!24, !25, i64 8064}
!24 = !{!"struct@_ZTS9FullBoard", !25, i64 8056, !25, i64 8064}
!25 = !{!"long long", !8, i64 0}
!26 = !{!4, !13, i64 1776}
!27 = !{!28, !25, i64 0}
!28 = !{!"struct@_ZTSN5boost5arrayINS0_IyLm441EEELm4EEE", !29, i64 0}
!29 = !{!"array@_ZTSA4_N5boost5arrayIyLm441EEE", !30, i64 0}
!30 = !{!"struct@_ZTSN5boost5arrayIyLm441EEE", !31, i64 0}
!31 = !{!"array@_ZTSA441_y", !25, i64 0}
!32 = distinct !{!32, !33}
!33 = !{!"llvm.loop.mustprogress"}

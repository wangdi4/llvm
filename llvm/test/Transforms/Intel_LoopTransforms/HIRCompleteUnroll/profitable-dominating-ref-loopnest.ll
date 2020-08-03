; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-post-vec-complete-unroll" -aa-pipeline="tbaa" 2>&1 < %s | FileCheck %s

; TODO: This test will be fixed with a patch to enable complete unroll for multi-exit loops. Currently, it is used to verify that we were able to cleanup redundant goto/label in the loopnest.

; Verify that the loopnest is profitable to unroll because of the load (%t9)[0].12.0[i1] which dominates the load (%t9)[0].12.0[i2].
; After unrolling the loopnest, many of the (%t9)[0].12.0[i2] loads will be eliminated as redundant.

; CHECK: Function

; CHECK:      + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK-NEXT: |   %t315 = (%t9)[0].12.0[i1];
; CHECK-NEXT: |   if ((%t9)[0].5.0[%t315 + %t305] == 2)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      + DO i2 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; CHECK-NEXT: |      |   %t328 = (%t9)[0].12.0[i2];
; CHECK-NEXT: |      |   if ((%t9)[0].7.0[%t315 + %t305 + %t328] == %t241)
; CHECK-NEXT: |      |   {
; CHECK-NEXT: |      |      goto earlyexit;
; CHECK-NEXT: |      |   }
; CHECK-NEXT: |      + END LOOP
; CHECK-NEXT: |
; CHECK-NEXT: |      %t335 = (%t9)[0].8.0[%t247];
; CHECK-NEXT: |      (%t9)[0].8.0[%t247] = %t335 + 1;
; CHECK-NEXT: |      earlyexit:
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP




%"class.boost::array.4" = type { [2 x i32] }
%"class.boost::array.3.338" = type { [5 x i64] }
%"class.boost::array.6" = type { [441 x i64] }
%"class.boost::array.8" = type { [882 x i64] }
%class.FastState = type <{ %class.FullBoard, float, i32, i32, i32, i32, i32, i32, %"class.boost::array.5.20", %"class.boost::array.6.21", [4 x i8] }>
%class.FullBoard = type { %class.FastBoard.base, i64, i64 }
%class.FastBoard.base = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32 }>
%"class.boost::array.0" = type { [441 x i32] }
%"class.boost::array.1" = type { [442 x i16] }
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.2" = type { [4 x i32] }
%"class.boost::array.3" = type { [8 x i32] }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { i32*, i32*, i32* }
%"class.boost::array.5.20" = type { [24 x i32] }
%"class.boost::array.6.21" = type { [24 x %"struct.std::pair"] }
%"struct.std::pair" = type { i32, i32 }
%class.FastBoard = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32, [4 x i8] }>

define void @foo(%class.FastBoard* %t9, i16 %t241, i64 %t247, i32 %t305) {
entry:
  %t302 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %t9, i64 0, i32 8, i32 0, i64 %t247
  br label %outerloop

outerloop:                                    ; preds = %outerbackedge, %entry
  %t313 = phi i64 [ 0, %entry ], [ %t339, %outerbackedge ]
  %t314 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %t9, i64 0, i32 12, i32 0, i64 %t313
  %t315 = load i32, i32* %t314, align 4, !tbaa !25
  %t316 = add nsw i32 %t315, %t305
  %t317 = sext i32 %t316 to i64
  %t318 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %t9, i64 0, i32 5, i32 0, i64 %t317
  %t319 = load i32, i32* %t318, align 4, !tbaa !1
  %t320 = icmp eq i32 %t319, 2
  br i1 %t320, label %if, label %outerbackedge

if:                                    ; preds = %outerloop
  br label %innerloop

innerloop:                                    ; preds = %backedge, %if
  %t326 = phi i64 [ %t323, %backedge ], [ 0, %if ]
  %t327 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %t9, i64 0, i32 12, i32 0, i64 %t326
  %t328 = load i32, i32* %t327, align 4, !tbaa !25
  %t329 = add nsw i32 %t328, %t316
  %t330 = sext i32 %t329 to i64
  %t331 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %t9, i64 0, i32 7, i32 0, i64 %t330
  %t332 = load i16, i16* %t331, align 2, !tbaa !14
  %t333 = icmp eq i16 %t332, %t241
  br i1 %t333, label %earlyexit, label %backedge

backedge:                                    ; preds = %innerloop
  %t323 = add nuw nsw i64 %t326, 1
  %t324 = icmp slt i64 %t326, 3
  br i1 %t324, label %innerloop, label %exit

exit:                                    ; preds = %backedge
  %t335 = load i16, i16* %t302, align 2, !tbaa !14
  %t336 = add i16 %t335, 1
  store i16 %t336, i16* %t302, align 2, !tbaa !14
  br label %outerbackedge

earlyexit:                                    ; preds = %innerloop
  br label %outerbackedge

outerbackedge:                                    ; preds = %earlyexit, %exit, %outerloop
  %t339 = add nuw nsw i64 %t313, 1
  %t340 = icmp eq i64 %t339, 4
  br i1 %t340, label %outerexit, label %outerloop

outerexit:
  ret void
}

!0 = !{!"clang version 5.0.0 (cfe/trunk)"}
!1 = !{!2, !4, i64 0}
!2 = !{!"struct@_ZTSN5boost5arrayIN9FastBoard8square_tELm441EEE", !3, i64 0}
!3 = !{!"array@_ZTSA441_N9FastBoard8square_tE", !4, i64 0}
!4 = !{!"_ZTSN9FastBoard8square_tE", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !10, i64 0}
!8 = !{!"struct@_ZTSN5boost5arrayIyLm441EEE", !9, i64 0}
!9 = !{!"array@_ZTSA441_y", !10, i64 0}
!10 = !{!"long long", !5, i64 0}
!11 = !{!12, !10, i64 8056}
!12 = !{!"struct@_ZTS9FullBoard", !10, i64 8056, !10, i64 8064}
!13 = !{!12, !10, i64 8064}
!14 = !{!15, !17, i64 0}
!15 = !{!"struct@_ZTSN5boost5arrayItLm442EEE", !16, i64 0}
!16 = !{!"array@_ZTSA442_t", !17, i64 0}
!17 = !{!"short", !5, i64 0}
!18 = !{!19, !17, i64 0}
!19 = !{!"struct@_ZTSN5boost5arrayItLm441EEE", !20, i64 0}
!20 = !{!"array@_ZTSA441_t", !17, i64 0}
!21 = !{!22, !24, i64 0}
!22 = !{!"struct@_ZTSN5boost5arrayIiLm2EEE", !23, i64 0}
!23 = !{!"array@_ZTSA2_i", !24, i64 0}
!24 = !{!"int", !5, i64 0}
!25 = !{!26, !24, i64 0}
!26 = !{!"struct@_ZTSN5boost5arrayIiLm4EEE", !27, i64 0}
!27 = !{!"array@_ZTSA4_i", !24, i64 0}
!28 = !{!29, !10, i64 0}
!29 = !{!"struct@_ZTSN5boost5arrayIyLm882EEE", !30, i64 0}
!30 = !{!"array@_ZTSA882_y", !10, i64 0}
!31 = !{!32, !24, i64 1764}
!32 = !{!"struct@_ZTS9FastBoard", !19, i64 0, !19, i64 882, !24, i64 1764, !24, i64 1768, !24, i64 1772, !2, i64 1776, !15, i64 3540, !15, i64 4424, !15, i64 5308, !15, i64 6192, !19, i64 7076, !26, i64 7960, !33, i64 7976, !22, i64 8008, !22, i64 8016, !35, i64 8024, !24, i64 8048}
!33 = !{!"struct@_ZTSN5boost5arrayIiLm8EEE", !34, i64 0}
!34 = !{!"array@_ZTSA8_i", !24, i64 0}
!35 = !{!"struct@_ZTSSt6vectorIiSaIiEE"}
!36 = !{!37, !24, i64 8084}
!37 = !{!"struct@_ZTS9FastState", !12, i64 0, !38, i64 8072, !24, i64 8076, !24, i64 8080, !24, i64 8084, !24, i64 8088, !24, i64 8092, !24, i64 8096, !39, i64 8100, !41, i64 8196}
!38 = !{!"float", !5, i64 0}
!39 = !{!"struct@_ZTSN5boost5arrayIiLm24EEE", !40, i64 0}
!40 = !{!"array@_ZTSA24_i", !24, i64 0}
!41 = !{!"struct@_ZTSN5boost5arrayISt4pairIiiELm24EEE", !5, i64 0}
!42 = !{!37, !24, i64 8092}
!43 = !{!37, !24, i64 8096}
!44 = !{!37, !24, i64 8088}
!45 = !{!32, !24, i64 1768}
!46 = !{!37, !10, i64 8056}
!47 = !{!37, !24, i64 8080}
!48 = !{!49, !10, i64 0}
!49 = !{!"struct@_ZTSN5boost5arrayIyLm5EEE", !50, i64 0}
!50 = !{!"array@_ZTSA5_y", !10, i64 0}


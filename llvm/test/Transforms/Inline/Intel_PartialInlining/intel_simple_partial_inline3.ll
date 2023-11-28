; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -intel-pi-test -passes=intel-partialinline -S 2>&1 | FileCheck %s

; This test checks the simple Intel partial inliner for a case where PGO was
; used on the original benchmark, which has resulted in needing to apply the
; transformation on a function that has more basic blocks and is a hot
; function marked by PGO with the 'inlinehint' attribute. (CMPLRLLVM-49898)
;
; The intent of this transformation for this case is that a cloned function
; will be created and called in the place of the original function to break
; a recursive function into a non-recursive part and a recursive part. The
; original called function is recursive, which prevents it from being inlined.
; The cloned function gets the loop body, which contained the recursive call,
; extracted to a separate function, that will then call the original recursive
; function. The hot part of the function is the portion that was extracted into
; a non-recursive routine, which can then be inlined into the caller, following
; this transformation.

; CHECK: define i32 @_ZN3povL16Inside_CSG_UnionEPdPNS_13Object_StructE
; CHECK: tail call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE.1

; CHECK: define zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE.1
; CHECK: codeRepl:
; CHECK: call i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE.1.bb8

; CHECK: define internal i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE.1.bb8
; CHECK: tail call fastcc noundef zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"struct._ZTSN3pov10CSG_StructE.pov::CSG_Struct" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"struct._ZTSN3pov19Bounding_Box_StructE.pov::Bounding_Box_Struct", ptr, ptr, float, i32, ptr, i32 }
%"struct._ZTSN3pov19Bounding_Box_StructE.pov::Bounding_Box_Struct" = type { [3 x float], [3 x float] }
%"struct._ZTSN3pov13Object_StructE.pov::Object_Struct" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"struct._ZTSN3pov19Bounding_Box_StructE.pov::Bounding_Box_Struct", ptr, ptr, float, i32 }
%"struct._ZTSN3pov19Light_Source_StructE.pov::Light_Source_Struct" = type { ptr, i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"struct._ZTSN3pov19Bounding_Box_StructE.pov::Bounding_Box_Struct", ptr, ptr, float, i32, ptr, [5 x float], [3 x double], [3 x double], [3 x double], [3 x double], [3 x double], double, double, double, double, double, ptr, i8, i8, i8, i8, i8, i8, i8, i8, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, [6 x ptr] }
%"struct._ZTSN3pov13Method_StructE.pov::Method_Struct" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }

; Function Attrs: inlinehint mustprogress uwtable
define i32 @_ZN3povL16Inside_CSG_UnionEPdPNS_13Object_StructE(ptr %arg, ptr %arg1) #0 !prof !35 !PGOFuncName !36 {
bb:
  %i = getelementptr inbounds %"struct._ZTSN3pov10CSG_StructE.pov::CSG_Struct", ptr %arg1, i64 0, i32 14, !intel-tbaa !37
  %i2 = load ptr, ptr %i, align 8, !tbaa !51
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb20, label %bb4, !prof !52

bb4:                                              ; preds = %bb16, %bb
  %i5 = phi ptr [ %i18, %bb16 ], [ %i2, %bb ]
  %i6 = getelementptr inbounds %"struct._ZTSN3pov13Object_StructE.pov::Object_Struct", ptr %i5, i64 0, i32 1, !intel-tbaa !53
  %i7 = load i32, ptr %i6, align 8, !tbaa !53
  %i8 = and i32 %i7, 32
  %i9 = icmp eq i32 %i8, 0
  br i1 %i9, label %bb14, label %bb10, !prof !55

bb10:                                             ; preds = %bb4
  %i11 = getelementptr inbounds %"struct._ZTSN3pov19Light_Source_StructE.pov::Light_Source_Struct", ptr %i5, i64 0, i32 14, !intel-tbaa !56
  %i12 = load ptr, ptr %i11, align 8, !tbaa !56
  %i13 = icmp eq ptr %i12, null
  br i1 %i13, label %bb16, label %bb14

bb14:                                             ; preds = %bb10, %bb4
  %i15 = tail call zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef nonnull %i5), !intel-profx !66
  br i1 %i15, label %bb20, label %bb16, !prof !67

bb16:                                             ; preds = %bb14, %bb10
  %i17 = getelementptr inbounds %"struct._ZTSN3pov13Object_StructE.pov::Object_Struct", ptr %i5, i64 0, i32 2, !intel-tbaa !68
  %i18 = load ptr, ptr %i17, align 8, !tbaa !51
  %i19 = icmp eq ptr %i18, null
  br i1 %i19, label %bb20, label %bb4, !prof !52, !llvm.loop !69

bb20:                                             ; preds = %bb16, %bb14, %bb
  %i21 = phi i32 [ 0, %bb ], [ 1, %bb14 ], [ 0, %bb16 ]
  ret i32 %i21
}

; Function Attrs: inlinehint mustprogress uwtable
define zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef %arg1) unnamed_addr #0 !prof !71 {
bb:
  %i = getelementptr inbounds %"struct._ZTSN3pov13Object_StructE.pov::Object_Struct", ptr %arg1, i64 0, i32 7, !intel-tbaa !72
  %i2 = load ptr, ptr %i, align 8, !tbaa !51
  %i3 = icmp eq ptr %i2, null
  br i1 %i3, label %bb11, label %bb8, !prof !73

bb4:                                              ; preds = %bb8
  %i5 = getelementptr inbounds %"struct._ZTSN3pov13Object_StructE.pov::Object_Struct", ptr %i9, i64 0, i32 2, !intel-tbaa !68
  %i6 = load ptr, ptr %i5, align 8, !tbaa !51
  %i7 = icmp eq ptr %i6, null
  br i1 %i7, label %bb11, label %bb8, !prof !73, !llvm.loop !74

bb8:                                              ; preds = %bb4, %bb
  %i9 = phi ptr [ %i6, %bb4 ], [ %i2, %bb ]
  %i10 = tail call fastcc noundef zeroext i1 @_ZN3pov13Inside_ObjectEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef nonnull %i9), !intel-profx !75
  br i1 %i10, label %bb4, label %bb32

bb11:                                             ; preds = %bb4, %bb
  %i12 = getelementptr inbounds %"struct._ZTSN3pov13Object_StructE.pov::Object_Struct", ptr %arg1, i64 0, i32 0, !intel-tbaa !76
  %i13 = load ptr, ptr %i12, align 8, !tbaa !76
  %i14 = getelementptr inbounds %"struct._ZTSN3pov13Method_StructE.pov::Method_Struct", ptr %i13, i64 0, i32 1, !intel-tbaa !77
  %i15 = load ptr, ptr %i14, align 8, !tbaa !77
  %i16 = icmp eq ptr %i15, @_ZN3povL12Inside_PlaneEPdPNS_13Object_StructE
  br i1 %i16, label %bb17, label %bb19, !prof !86

bb17:                                             ; preds = %bb11
  %i18 = tail call noundef i32 @_ZN3povL12Inside_PlaneEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef %arg1), !range !87, !intel-profx !88, !intel_dtrans_type !89
  br label %bb29

bb19:                                             ; preds = %bb11
  %i20 = icmp eq ptr %i15, @_ZN3povL14Inside_QuadricEPdPNS_13Object_StructE
  br i1 %i20, label %bb21, label %bb23, !prof !93

bb21:                                             ; preds = %bb19
  %i22 = tail call noundef i32 @_ZN3povL14Inside_QuadricEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef %arg1), !range !87, !intel-profx !94, !intel_dtrans_type !89
  br label %bb29

bb23:                                             ; preds = %bb19
  %i24 = icmp eq ptr %i15, @_ZN3povL13Inside_SphereEPdPNS_13Object_StructE
  br i1 %i24, label %bb25, label %bb27, !prof !95

bb25:                                             ; preds = %bb23
  %i26 = tail call noundef i32 @_ZN3povL13Inside_SphereEPdPNS_13Object_StructE(ptr noundef %arg, ptr noundef %arg1), !range !87, !intel-profx !96, !intel_dtrans_type !89
  br label %bb29

bb27:                                             ; preds = %bb23
  %i28 = tail call noundef i32 %i15(ptr noundef %arg, ptr noundef %arg1), !intel-profx !97
  br label %bb29

bb29:                                             ; preds = %bb27, %bb25, %bb21, %bb17
  %i30 = phi i32 [ %i18, %bb17 ], [ %i22, %bb21 ], [ %i28, %bb27 ], [ %i26, %bb25 ]
  %i31 = icmp ne i32 %i30, 0
  br label %bb32

bb32:                                             ; preds = %bb29, %bb8
  %i33 = phi i1 [ %i31, %bb29 ], [ false, %bb8 ]
  ret i1 %i33
}

; Function Attrs: inlinehint mustprogress nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
declare i32 @_ZN3povL12Inside_PlaneEPdPNS_13Object_StructE(ptr nocapture noundef readonly, ptr nocapture noundef readonly) #1

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
declare i32 @_ZN3povL14Inside_QuadricEPdPNS_13Object_StructE(ptr nocapture noundef readonly, ptr nocapture noundef readonly) #2

; Function Attrs: inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
declare i32 @_ZN3povL13Inside_SphereEPdPNS_13Object_StructE(ptr nocapture noundef readonly, ptr nocapture noundef readonly) #2

attributes #0 = { inlinehint mustprogress uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inlinehint mustprogress nofree nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { inlinehint mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5, !6}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"Virtual Function Elim", i32 0}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 1, !"ThinLTO", i32 0}
!5 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!6 = !{i32 1, !"ProfileSummary", !7}
!7 = !{!8, !9, !10, !11, !12, !13, !14, !15, !16, !17}
!8 = !{!"ProfileFormat", !"InstrProf"}
!9 = !{!"TotalCount", i64 2245597371}
!10 = !{!"MaxCount", i64 151865319}
!11 = !{!"MaxInternalCount", i64 151865319}
!12 = !{!"MaxFunctionCount", i64 73424739}
!13 = !{!"NumCounts", i64 15938}
!14 = !{!"NumFunctions", i64 1592}
!15 = !{!"IsPartialProfile", i64 0}
!16 = !{!"PartialProfileRatio", double 0.000000e+00}
!17 = !{!"DetailedSummary", !18}
!18 = !{!19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34}
!19 = !{i32 10000, i64 151865319, i32 1}
!20 = !{i32 100000, i64 133024055, i32 3}
!21 = !{i32 200000, i64 101801227, i32 4}
!22 = !{i32 300000, i64 72831699, i32 7}
!23 = !{i32 400000, i64 61399006, i32 10}
!24 = !{i32 500000, i64 46594080, i32 14}
!25 = !{i32 600000, i64 33242457, i32 20}
!26 = !{i32 700000, i64 16577853, i32 30}
!27 = !{i32 800000, i64 10347529, i32 46}
!28 = !{i32 900000, i64 4014765, i32 79}
!29 = !{i32 950000, i64 1242621, i32 134}
!30 = !{i32 990000, i64 312216, i32 267}
!31 = !{i32 999000, i64 39104, i32 443}
!32 = !{i32 999900, i64 789, i32 740}
!33 = !{i32 999990, i64 72, i32 1480}
!34 = !{i32 999999, i64 6, i32 2307}
!35 = !{!"function_entry_count", i64 814965}
!36 = !{!"csg.cpp;_ZN3povL16Inside_CSG_UnionEPdPNS_13Object_StructE"}
!37 = !{!38, !43, i64 120}
!38 = !{!"struct@_ZTSN3pov10CSG_StructE", !39, i64 0, !42, i64 8, !43, i64 16, !44, i64 24, !44, i64 32, !45, i64 40, !43, i64 48, !43, i64 56, !46, i64 64, !47, i64 72, !50, i64 96, !50, i64 104, !49, i64 112, !42, i64 116, !43, i64 120, !42, i64 128}
!39 = !{!"pointer@_ZTSPN3pov13Method_StructE", !40, i64 0}
!40 = !{!"omnipotent char", !41, i64 0}
!41 = !{!"Simple C++ TBAA"}
!42 = !{!"int", !40, i64 0}
!43 = !{!"pointer@_ZTSPN3pov13Object_StructE", !40, i64 0}
!44 = !{!"pointer@_ZTSPN3pov14Texture_StructE", !40, i64 0}
!45 = !{!"pointer@_ZTSPN3pov15Interior_StructE", !40, i64 0}
!46 = !{!"pointer@_ZTSPN3pov19Light_Source_StructE", !40, i64 0}
!47 = !{!"struct@_ZTSN3pov19Bounding_Box_StructE", !48, i64 0, !48, i64 12}
!48 = !{!"array@_ZTSA3_f", !49, i64 0}
!49 = !{!"float", !40, i64 0}
!50 = !{!"pointer@_ZTSPN3pov16Transform_StructE", !40, i64 0}
!51 = !{!43, !43, i64 0}
!52 = !{!"branch_weights", i32 87262, i32 1161267}
!53 = !{!54, !42, i64 8}
!54 = !{!"struct@_ZTSN3pov13Object_StructE", !39, i64 0, !42, i64 8, !43, i64 16, !44, i64 24, !44, i64 32, !45, i64 40, !43, i64 48, !43, i64 56, !46, i64 64, !47, i64 72, !50, i64 96, !50, i64 104, !49, i64 112, !42, i64 116}
!55 = !{!"branch_weights", i32 1161267, i32 0}
!56 = !{!57, !43, i64 120}
!57 = !{!"struct@_ZTSN3pov19Light_Source_StructE", !39, i64 0, !42, i64 8, !43, i64 16, !44, i64 24, !44, i64 32, !45, i64 40, !43, i64 48, !43, i64 56, !46, i64 64, !47, i64 72, !50, i64 96, !50, i64 104, !49, i64 112, !42, i64 116, !43, i64 120, !58, i64 128, !59, i64 152, !59, i64 176, !59, i64 200, !59, i64 224, !59, i64 248, !60, i64 272, !60, i64 280, !60, i64 288, !60, i64 296, !60, i64 304, !46, i64 312, !40, i64 320, !40, i64 321, !40, i64 322, !61, i64 323, !61, i64 324, !40, i64 325, !40, i64 326, !40, i64 327, !42, i64 328, !42, i64 332, !42, i64 336, !42, i64 340, !42, i64 344, !62, i64 352, !43, i64 360, !43, i64 368, !63, i64 376, !64, i64 384}
!58 = !{!"array@_ZTSA5_f", !49, i64 0}
!59 = !{!"array@_ZTSA3_d", !60, i64 0}
!60 = !{!"double", !40, i64 0}
!61 = !{!"bool", !40, i64 0}
!62 = !{!"pointer@_ZTSPPA5_f", !40, i64 0}
!63 = !{!"pointer@_ZTSPN3pov16Blend_Map_StructE", !40, i64 0}
!64 = !{!"array@_ZTSA6_PN3pov24Project_Tree_Node_StructE", !65, i64 0}
!65 = !{!"pointer@_ZTSPN3pov24Project_Tree_Node_StructE", !40, i64 0}
!66 = !{!"intel_profx", i64 1161267}
!67 = !{!"branch_weights", i32 727703, i32 433564}
!68 = !{!54, !43, i64 16}
!69 = distinct !{!69, !70}
!70 = !{!"llvm.loop.mustprogress"}
!71 = !{!"function_entry_count", i64 70245544}
!72 = !{!54, !43, i64 56}
!73 = !{!"branch_weights", i32 70245544, i32 0}
!74 = distinct !{!74, !70}
!75 = !{!"intel_profx", i64 0}
!76 = !{!54, !39, i64 0}
!77 = !{!78, !80, i64 8}
!78 = !{!"struct@_ZTSN3pov13Method_StructE", !79, i64 0, !80, i64 8, !81, i64 16, !81, i64 24, !82, i64 32, !83, i64 40, !83, i64 48, !83, i64 56, !84, i64 64, !85, i64 72, !85, i64 80}
!79 = !{!"pointer@_ZTSPFiPN3pov13Object_StructEPNS_10Ray_StructEPNS_13istack_structEE", !40, i64 0}
!80 = !{!"pointer@_ZTSPFiPdPN3pov13Object_StructEE", !40, i64 0}
!81 = !{!"pointer@_ZTSPFvPdPN3pov13Object_StructEPNS0_10istk_entryEE", !40, i64 0}
!82 = !{!"pointer@_ZTSPFPvPN3pov13Object_StructEE", !40, i64 0}
!83 = !{!"pointer@_ZTSPFvPN3pov13Object_StructEPdPNS_16Transform_StructEE", !40, i64 0}
!84 = !{!"pointer@_ZTSPFvPN3pov13Object_StructEPNS_16Transform_StructEE", !40, i64 0}
!85 = !{!"pointer@_ZTSPFvPN3pov13Object_StructEE", !40, i64 0}
!86 = !{!"branch_weights", i32 50237224, i32 20008320}
!87 = !{i32 0, i32 2}
!88 = !{!"intel_profx", i64 50237224}
!89 = !{!"F", i1 false, i32 2, !90, !91, !92}
!90 = !{i32 0, i32 0}
!91 = !{double 0.000000e+00, i32 1}
!92 = !{%"struct._ZTSN3pov13Object_StructE.pov::Object_Struct" zeroinitializer, i32 1}
!93 = !{!"branch_weights", i32 16310638, i32 3697682}
!94 = !{!"intel_profx", i64 16310638}
!95 = !{!"branch_weights", i32 2657422, i32 1040260}
!96 = !{!"intel_profx", i64 2657422}
!97 = !{!"intel_profx", i64 1040260}

; end INTEL_FEATURE_SW_ADVANCED

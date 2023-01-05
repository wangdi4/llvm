; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; This test is to verify that a load of a pointer from a location that the
; pointer points to does not result in treating a type as being many levels of
; pointer indirection. For example:
;   %x = load ptr, ptr %y
;
; If %x and %y use a common PHINode, then %x and %y are the same type, but the
; inference implies that %y is a pointer to the type of %x. Check that this does
; not cause type propagation that keeps creating additional levels of
; indirection.

%struct._ZTS8_Cluster._Cluster = type { ptr, %struct._ZTS13_ExtentPacket._ExtentPacket, %struct._ZTS13_ExtentPacket._ExtentPacket, %struct._ZTS13_ExtentPacket._ExtentPacket, i64, i64 }
%struct._ZTS13_ExtentPacket._ExtentPacket = type { float, i64, i64, i64 }
%struct._ZTS12_PixelPacket._PixelPacket = type { i16, i16, i16, i16 }
%struct._ZTS6_Image._Image = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct._ZTS12_PixelPacket._PixelPacket, %struct._ZTS12_PixelPacket._PixelPacket, %struct._ZTS12_PixelPacket._PixelPacket, double, %struct._ZTS17_ChromaticityInfo._ChromaticityInfo, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct._ZTS14_RectangleInfo._RectangleInfo, %struct._ZTS14_RectangleInfo._RectangleInfo, %struct._ZTS14_RectangleInfo._RectangleInfo, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct._ZTS10_ErrorInfo._ErrorInfo, %struct._ZTS10_TimerInfo._TimerInfo, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct._ZTS14_ExceptionInfo._ExceptionInfo, i32, i64, ptr, %struct._ZTS12_ProfileInfo._ProfileInfo, %struct._ZTS12_ProfileInfo._ProfileInfo, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct._ZTS12_PixelPacket._PixelPacket, ptr, %struct._ZTS14_RectangleInfo._RectangleInfo, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct._ZTS17_ChromaticityInfo._ChromaticityInfo = type { %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo, %struct._ZTS12_PrimaryInfo._PrimaryInfo }
%struct._ZTS12_PrimaryInfo._PrimaryInfo = type { double, double, double }
%struct._ZTS10_ErrorInfo._ErrorInfo = type { double, double, double }
%struct._ZTS10_TimerInfo._TimerInfo = type { %struct._ZTS6_Timer._Timer, %struct._ZTS6_Timer._Timer, i32, i64 }
%struct._ZTS6_Timer._Timer = type { double, double, double }
%struct._ZTS14_ExceptionInfo._ExceptionInfo = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct._ZTS12_ProfileInfo._ProfileInfo = type { ptr, i64, ptr, i64 }
%struct._ZTS14_RectangleInfo._RectangleInfo = type { i64, i64, i64, i64 }
%struct._ZTS12_Ascii85Info._Ascii85Info = type { i64, i64, [10 x i8] }
%struct._ZTS9_BlobInfo._BlobInfo = type { i64, i64, i64, i32, i32, i64, i64, i32, i32, i32, i32, i32, %union._ZTS8FileInfo.FileInfo, %struct._ZTS4stat.stat, ptr, ptr, i32, ptr, i64, i64 }
%union._ZTS8FileInfo.FileInfo = type { ptr }
%struct._ZTS4stat.stat = type { i64, i64, i64, i32, i32, i32, i32, i64, i64, i64, i64, %struct._ZTS8timespec.timespec, %struct._ZTS8timespec.timespec, %struct._ZTS8timespec.timespec, [3 x i64] }
%struct._ZTS8timespec.timespec = type { i64, i64 }
%struct._ZTS13SemaphoreInfo.SemaphoreInfo = type { i64, i32, i64, i64 }
%struct._ZTS8_IO_FILE._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._ZTS10_IO_marker._IO_marker = type opaque
%struct._ZTS11_IO_codecvt._IO_codecvt = type opaque
%struct._ZTS13_IO_wide_data._IO_wide_data = type opaque

; Function Attrs: mustprogress nofree
declare !intel.dtrans.func.type !62 ptr @AcquireMagickMemory(i64) #0

define hidden i32 @SegmentImage(ptr noundef "intel_dtrans_func_index"="1" %arg, i32 noundef %arg1, i32 noundef %arg2, double noundef %arg3, double noundef %arg4) #1 !intel.dtrans.func.type !63 {
bb:
  br label %bb5

bb5:                                              ; preds = %bb6, %bb
  br label %bb6

bb6:                                              ; preds = %bb5
  br i1 false, label %bb7, label %bb5

bb7:                                              ; preds = %bb6
  br i1 false, label %bb8, label %bb22

bb8:                                              ; preds = %bb7
  br label %bb9

bb9:                                              ; preds = %bb13, %bb8
  %i = phi ptr [ null, %bb8 ], [ %i20, %bb13 ]
  %i10 = phi ptr [ null, %bb8 ], [ %i20, %bb13 ]
  ret i32 0

bb11:                                             ; preds = %bb12
  br label %bb12

bb12:                                             ; preds = %bb11
  br i1 false, label %bb13, label %bb11

bb13:                                             ; preds = %bb12
  br i1 false, label %bb9, label %bb21

bb14:                                             ; preds = %bb15
  br i1 false, label %bb16, label %bb15

bb15:                                             ; preds = %bb14
  br label %bb14

bb16:                                             ; preds = %bb14
  br label %bb17

bb17:                                             ; preds = %bb16
  br label %bb18

bb18:                                             ; preds = %bb17
  br label %bb19

bb19:                                             ; preds = %bb18
  %i20 = call ptr @AcquireMagickMemory(i64 0)
  ret i32 0

bb21:                                             ; preds = %bb13
  br i1 false, label %bb22, label %bb25

bb22:                                             ; preds = %bb21, %bb7
  %i23 = call ptr @AcquireMagickMemory(i64 0)
  br label %bb24

bb24:                                             ; preds = %bb22
  br label %bb25

bb25:                                             ; preds = %bb24, %bb21
  %i26 = phi ptr [ %i23, %bb24 ], [ %i20, %bb21 ]
; CHECK:  %i26 = phi ptr [ %i23, %bb24 ], [ %i20, %bb21 ]
; CHECK:    LocalPointerInfo: CompletelyAnalyzed
; CHECK:      Aliased types:
; CHECK:        %struct._ZTS8_Cluster._Cluster*{{ *}}
; CHECK:        i8*{{ *}}

  br label %bb27

bb27:                                             ; preds = %bb25
  br label %bb28

bb28:                                             ; preds = %bb27
  br label %bb29

bb29:                                             ; preds = %bb28
  br label %bb30

bb30:                                             ; preds = %bb29
  br label %bb31

bb31:                                             ; preds = %bb34, %bb30
  %i32 = phi ptr [ %i26, %bb30 ], [ %i35, %bb34 ]
  %i33 = getelementptr inbounds %struct._ZTS8_Cluster._Cluster, ptr %i32, i64 0, i32 1
  br label %bb34

bb34:                                             ; preds = %bb31
  %i35 = load ptr, ptr %i32, align 8
  br label %bb31
}

attributes #0 = { mustprogress nofree }
attributes #1 = { "denormal-fp-math-f32"="ieee,ieee" }

!intel.dtrans.types = !{!0, !2, !24, !26, !27, !28, !29, !31, !32, !33, !34, !41, !46, !47, !50, !51, !53, !56, !58, !59, !60, !61}

!0 = !{!"S", %struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 4, !1, !1, !1, !1}
!1 = !{i16 0, i32 0}
!2 = !{!"S", %struct._ZTS6_Image._Image zeroinitializer, i32 85, !3, !3, !3, !4, !3, !3, !3, !4, !4, !4, !4, !5, !6, !6, !6, !7, !8, !3, !9, !3, !9, !9, !9, !4, !7, !7, !10, !10, !10, !7, !7, !7, !3, !3, !3, !3, !3, !3, !11, !4, !4, !4, !4, !4, !4, !12, !13, !14, !9, !9, !9, !16, !17, !18, !18, !18, !4, !4, !20, !3, !4, !21, !22, !22, !23, !4, !4, !11, !11, !11, !3, !3, !6, !11, !10, !9, !9, !3, !3, !4, !3, !4, !4, !3, !4}
!3 = !{i32 0, i32 0}
!4 = !{i64 0, i32 0}
!5 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 1}
!6 = !{%struct._ZTS12_PixelPacket._PixelPacket zeroinitializer, i32 0}
!7 = !{double 0.000000e+00, i32 0}
!8 = !{%struct._ZTS17_ChromaticityInfo._ChromaticityInfo zeroinitializer, i32 0}
!9 = !{i8 0, i32 1}
!10 = !{%struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 0}
!11 = !{%struct._ZTS6_Image._Image zeroinitializer, i32 1}
!12 = !{%struct._ZTS10_ErrorInfo._ErrorInfo zeroinitializer, i32 0}
!13 = !{%struct._ZTS10_TimerInfo._TimerInfo zeroinitializer, i32 0}
!14 = !{!15, i32 1}
!15 = !{!"F", i1 false, i32 4, !3, !9, !4, !4, !9}
!16 = !{%struct._ZTS12_Ascii85Info._Ascii85Info zeroinitializer, i32 1}
!17 = !{%struct._ZTS9_BlobInfo._BlobInfo zeroinitializer, i32 1}
!18 = !{!"A", i32 4096, !19}
!19 = !{i8 0, i32 0}
!20 = !{%struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 0}
!21 = !{%struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 1}
!22 = !{%struct._ZTS12_ProfileInfo._ProfileInfo zeroinitializer, i32 0}
!23 = !{%struct._ZTS12_ProfileInfo._ProfileInfo zeroinitializer, i32 1}
!24 = !{!"S", %struct._ZTS17_ChromaticityInfo._ChromaticityInfo zeroinitializer, i32 4, !25, !25, !25, !25}
!25 = !{%struct._ZTS12_PrimaryInfo._PrimaryInfo zeroinitializer, i32 0}
!26 = !{!"S", %struct._ZTS12_PrimaryInfo._PrimaryInfo zeroinitializer, i32 3, !7, !7, !7}
!27 = !{!"S", %struct._ZTS14_RectangleInfo._RectangleInfo zeroinitializer, i32 4, !4, !4, !4, !4}
!28 = !{!"S", %struct._ZTS10_ErrorInfo._ErrorInfo zeroinitializer, i32 3, !7, !7, !7}
!29 = !{!"S", %struct._ZTS10_TimerInfo._TimerInfo zeroinitializer, i32 4, !30, !30, !3, !4}
!30 = !{%struct._ZTS6_Timer._Timer zeroinitializer, i32 0}
!31 = !{!"S", %struct._ZTS6_Timer._Timer zeroinitializer, i32 3, !7, !7, !7}
!32 = !{!"S", %struct._ZTS14_ExceptionInfo._ExceptionInfo zeroinitializer, i32 8, !3, !3, !9, !9, !9, !3, !21, !4}
!33 = !{!"S", %struct._ZTS12_ProfileInfo._ProfileInfo zeroinitializer, i32 4, !9, !4, !9, !4}
!34 = !{!"S", %struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 29, !3, !9, !9, !9, !9, !9, !9, !9, !9, !9, !9, !9, !35, !36, !3, !3, !4, !1, !19, !37, !9, !4, !38, !39, !36, !9, !4, !3, !40}
!35 = !{%struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 1}
!36 = !{%struct._ZTS8_IO_FILE._IO_FILE zeroinitializer, i32 1}
!37 = !{!"A", i32 1, !19}
!38 = !{%struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 1}
!39 = !{%struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 1}
!40 = !{!"A", i32 20, !19}
!41 = !{!"S", %struct._ZTS9_BlobInfo._BlobInfo zeroinitializer, i32 20, !4, !4, !4, !3, !3, !4, !4, !3, !3, !3, !3, !3, !42, !43, !44, !9, !3, !21, !4, !4}
!42 = !{%union._ZTS8FileInfo.FileInfo zeroinitializer, i32 0}
!43 = !{%struct._ZTS4stat.stat zeroinitializer, i32 0}
!44 = !{!45, i32 1}
!45 = !{!"F", i1 false, i32 3, !4, !11, !9, !4}
!46 = !{!"S", %union._ZTS8FileInfo.FileInfo zeroinitializer, i32 1, !36}
!47 = !{!"S", %struct._ZTS4stat.stat zeroinitializer, i32 15, !4, !4, !4, !3, !3, !3, !3, !4, !4, !4, !4, !48, !48, !48, !49}
!48 = !{%struct._ZTS8timespec.timespec zeroinitializer, i32 0}
!49 = !{!"A", i32 3, !4}
!50 = !{!"S", %struct._ZTS8timespec.timespec zeroinitializer, i32 2, !4, !4}
!51 = !{!"S", %struct._ZTS12_Ascii85Info._Ascii85Info zeroinitializer, i32 3, !4, !4, !52}
!52 = !{!"A", i32 10, !19}
!53 = !{!"S", %struct._ZTS8_Cluster._Cluster zeroinitializer, i32 6, !54, !55, !55, !55, !4, !4}
!54 = !{%struct._ZTS8_Cluster._Cluster zeroinitializer, i32 1}
!55 = !{%struct._ZTS13_ExtentPacket._ExtentPacket zeroinitializer, i32 0}
!56 = !{!"S", %struct._ZTS13_ExtentPacket._ExtentPacket zeroinitializer, i32 4, !57, !4, !4, !4}
!57 = !{float 0.000000e+00, i32 0}
!58 = !{!"S", %struct._ZTS13SemaphoreInfo.SemaphoreInfo zeroinitializer, i32 4, !4, !3, !4, !4}
!59 = !{!"S", %struct._ZTS10_IO_marker._IO_marker zeroinitializer, i32 -1}
!60 = !{!"S", %struct._ZTS11_IO_codecvt._IO_codecvt zeroinitializer, i32 -1}
!61 = !{!"S", %struct._ZTS13_IO_wide_data._IO_wide_data zeroinitializer, i32 -1}
!62 = distinct !{!9}
!63 = distinct !{!11}

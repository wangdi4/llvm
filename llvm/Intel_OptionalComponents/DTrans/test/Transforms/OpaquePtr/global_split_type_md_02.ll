; This test verifies that intel_dtrans_type metadata is propagated
; to global variables that are created by GlobalSplit.
;
; RUN: opt < %s -S -passes=globalsplit 2>&1 | FileCheck %s

; CHECK: @_ZN11xercesc_2_7L12fgBlockNamesE.0 = private constant <{ [12 x i16], [81 x i16] }> <{ [12 x i16] [i16 73, i16 115, i16 66, i16 97, i16 115, i16 105, i16 99, i16 76, i16 97, i16 116, i16 105, i16 110], [81 x i16] zeroinitializer }>, !intel_dtrans_type ![[DT0:[0-9]+]]
; CHECK: @_ZN11xercesc_2_7L12fgBlockNamesE.1 = private constant <{ [19 x i16], [74 x i16] }> <{ [19 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 45, i16 49, i16 83, i16 117, i16 112, i16 112, i16 108, i16 101, i16 109, i16 101, i16 110, i16 116], [74 x i16] zeroinitializer }>, !intel_dtrans_type ![[DT5:[0-9]+]]
; CHECK: @_ZN11xercesc_2_7L12fgBlockNamesE.2 = private constant <{ [17 x i16], [76 x i16] }> <{ [17 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 69, i16 120, i16 116, i16 101, i16 110, i16 100, i16 101, i16 100, i16 45, i16 65], [76 x i16] zeroinitializer }>, !intel_dtrans_type ![[DT9:[0-9]+]]
; CHECK: @_ZN11xercesc_2_7L12fgBlockNamesE.3 = private constant <{ [17 x i16], [76 x i16] }> <{ [17 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 69, i16 120, i16 116, i16 101, i16 110, i16 100, i16 101, i16 100, i16 45, i16 66], [76 x i16] zeroinitializer }>, !intel_dtrans_type ![[DT9:[0-9]+]]
; CHECK: @_ZN11xercesc_2_7L12fgBlockNamesE.4 = private constant <{ [15 x i16], [78 x i16] }> <{ [15 x i16] [i16 73, i16 115, i16 73, i16 80, i16 65, i16 69, i16 120, i16 116, i16 101, i16 110, i16 115, i16 105, i16 111, i16 110, i16 115], [78 x i16] zeroinitializer }>, !intel_dtrans_type ![[DT13:[0-9]+]]

; CHECK: ![[DT0]] = !{![[DT1:[0-9]+]], i32 0}
; CHECK: ![[DT1]] = !{!"L", i32 2, ![[DT2:[0-9]+]], ![[DT4:[0-9]+]]}
; CHECK: ![[DT2]] = !{!"A", i32 12, ![[DT3:[0-9]+]]}
; CHECK: ![[DT3]] = !{i16 0, i32 0}
; CHECK: ![[DT4]] = !{!"A", i32 81, ![[DT3]]}
; CHECK: ![[DT5]] = !{![[DT6:[0-9]+]], i32 0}
; CHECK: ![[DT6]] = !{!"L", i32 2, ![[DT7:[0-9]+]], ![[DT8:[0-9]+]]}
; CHECK: ![[DT7]] = !{!"A", i32 19, ![[DT3]]}
; CHECK: ![[DT8]] = !{!"A", i32 74, ![[DT3]]}
; CHECK: ![[DT9]] = !{![[DT10:[0-9]+]], i32 0}
; CHECK: ![[DT10]] = !{!"L", i32 2, ![[DT11:[0-9]+]], ![[DT12:[0-9]+]]}
; CHECK: ![[DT11]] = !{!"A", i32 17, ![[DT3]]}
; CHECK: ![[DT12]] = !{!"A", i32 76, ![[DT3]]}
; CHECK: ![[DT13]] = !{![[DT14:[0-9]+]], i32 0}
; CHECK: ![[DT14]] = !{!"L", i32 2, ![[DT15:[0-9]+]], ![[DT16:[0-9]+]]}
; CHECK: ![[DT15]] = !{!"A", i32 15, ![[DT3]]}
; CHECK: ![[DT16]] = !{!"A", i32 78, ![[DT3]]}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZN11xercesc_2_7L12fgBlockNamesE = internal constant <{ <{ [12 x i16], [81 x i16] }>, <{ [19 x i16], [74 x i16] }>, <{ [17 x i16], [76 x i16] }>, <{ [17 x i16], [76 x i16] }>, <{ [15 x i16], [78 x i16] }> }> <{ <{ [12 x i16], [81 x i16] }> <{ [12 x i16] [i16 73, i16 115, i16 66, i16 97, i16 115, i16 105, i16 99, i16 76, i16 97, i16 116, i16 105, i16 110], [81 x i16] zeroinitializer }>, <{ [19 x i16], [74 x i16] }> <{ [19 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 45, i16 49, i16 83, i16 117, i16 112, i16 112, i16 108, i16 101, i16 109, i16 101, i16 110, i16 116], [74 x i16] zeroinitializer }>, <{ [17 x i16], [76 x i16] }> <{ [17 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 69, i16 120, i16 116, i16 101, i16 110, i16 100, i16 101, i16 100, i16 45, i16 65], [76 x i16] zeroinitializer }>, <{ [17 x i16], [76 x i16] }> <{ [17 x i16] [i16 73, i16 115, i16 76, i16 97, i16 116, i16 105, i16 110, i16 69, i16 120, i16 116, i16 101, i16 110, i16 100, i16 101, i16 100, i16 45, i16 66], [76 x i16] zeroinitializer }>, <{ [15 x i16], [78 x i16] }> <{ [15 x i16] [i16 73, i16 115, i16 73, i16 80, i16 65, i16 69, i16 120, i16 116, i16 101, i16 110, i16 115, i16 105, i16 111, i16 110, i16 115], [78 x i16] zeroinitializer }> }>, !intel_dtrans_type !0

define internal void @_ZN11xercesc_2_716XMLPlatformUtils5panicENS_12PanicHandler12PanicReasonsE() {
  %1 = tail call i1 @llvm.type.test(ptr null, metadata !"_ZTSN11xercesc_2_712PanicHandlerE")
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare i1 @llvm.type.test(ptr, metadata)

!0 = !{!1, i32 0}
!1 = !{!"L", i32 5, !2, !7, !11, !11, !15}
!2 = !{!3, i32 0}
!3 = !{!"L", i32 2, !4, !6}
!4 = !{!"A", i32 12, !5}
!5 = !{i16 0, i32 0}
!6 = !{!"A", i32 81, !5}
!7 = !{!8, i32 0}
!8 = !{!"L", i32 2, !9, !10}
!9 = !{!"A", i32 19, !5}
!10 = !{!"A", i32 74, !5}
!11 = !{!12, i32 0}
!12 = !{!"L", i32 2, !13, !14}
!13 = !{!"A", i32 17, !5}
!14 = !{!"A", i32 76, !5}
!15 = !{!16, i32 0}
!16 = !{!"L", i32 2, !17, !18}
!17 = !{!"A", i32 15, !5}
!18 = !{!"A", i32 78, !5}

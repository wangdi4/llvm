; RUN: opt -opaque-pointers -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-normalizeop < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; This test verifies that (GEP, 0, 0) is NOT generated for use of %i.

; CHECK: define void @foo(
; CHECK: %i = getelementptr inbounds %ValueVectorOf, ptr %arg, i64 0, i32 0
; CHECK-NOT: %dtnorm = getelementptr %ValueVectorOf, ptr %i, i64 0, i32 0
; CHECK: store i8 0, ptr %i, align 8

%"ValueVectorOf" = type { i8, i32, i32, ptr, ptr }
%"MemoryManager" = type { ptr }
%"XSerializable" = type { ptr }
%"QName" = type { %"XSerializable", i32, i32, i32, i32, ptr, ptr, ptr, ptr }
%"IdentityConstraint" = type <{ %"XSerializable", ptr, ptr, ptr, ptr, ptr, i32, [4 x i8] }>
%"IC_Selector" = type { %"XSerializable", ptr, ptr }
%"RefVectorOf" = type { %"BaseRefVectorOf" }
%"BaseRefVectorOf" = type { ptr, i8, i32, i32, ptr, ptr }
%"IC_Field" = type { %"XSerializable", ptr, ptr }
%"XercesXPath" = type { %"XSerializable", i32, ptr, ptr, ptr }
%"RefVectorOf1" = type { %"BaseRefVectorOf1" }
%"BaseRefVectorOf1" = type { ptr, i8, i32, i32, ptr, ptr }
%"MemoryManagerImpl" = type { %"MemoryManager" }
%"XercesLocationPath" = type { %"XSerializable", ptr }
%"XercesStep" = type { %"XSerializable", i16, ptr }
%"XercesNodeTest" = type { %"XSerializable", i16, ptr }
%"XalanDummyMemoryManager" = type { %"MemoryManager" }

define void @foo(ptr "intel_dtrans_func_index"="1" %arg) !intel.dtrans.func.type !43 {
  %i = getelementptr inbounds %"ValueVectorOf", ptr %arg, i64 0, i32 0
  store i8 0, ptr %i, align 8
  ret void
}

!intel.dtrans.types = !{!0, !4, !5, !9, !14, !16, !18, !21, !23, !24, !25, !27, !29, !31, !35, !38}

!0 = !{!"S", %"MemoryManager" zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %"XSerializable" zeroinitializer, i32 1, !1}
!5 = !{!"S", %"QName" zeroinitializer, i32 9, !6, !3, !3, !3, !3, !7, !7, !7, !8}
!6 = !{%"XSerializable" zeroinitializer, i32 0}
!7 = !{i16 0, i32 1}
!8 = !{%"MemoryManager" zeroinitializer, i32 1}
!9 = !{!"S", %"IdentityConstraint" zeroinitializer, i32 8, !6, !7, !7, !10, !11, !8, !3, !12}
!10 = !{%"IC_Selector" zeroinitializer, i32 1}
!11 = !{%"RefVectorOf" zeroinitializer, i32 1}
!12 = !{!"A", i32 4, !13}
!13 = !{i8 0, i32 0}
!14 = !{!"S", %"RefVectorOf" zeroinitializer, i32 1, !15}
!15 = !{%"BaseRefVectorOf" zeroinitializer, i32 0}
!16 = !{!"S", %"BaseRefVectorOf" zeroinitializer, i32 6, !1, !13, !3, !3, !17, !8}
!17 = !{%"IC_Field" zeroinitializer, i32 2}
!18 = !{!"S", %"IC_Field" zeroinitializer, i32 3, !6, !19, !20}
!19 = !{%"XercesXPath" zeroinitializer, i32 1}
!20 = !{%"IdentityConstraint" zeroinitializer, i32 1}
!21 = !{!"S", %"XercesXPath" zeroinitializer, i32 5, !6, !3, !7, !22, !8}
!22 = !{%"RefVectorOf1" zeroinitializer, i32 1}
!23 = !{!"S", %"ValueVectorOf" zeroinitializer, i32 5, !13, !3, !3, !17, !8}
!24 = !{!"S", %"IC_Selector" zeroinitializer, i32 3, !6, !19, !20}
!25 = !{!"S", %"MemoryManagerImpl" zeroinitializer, i32 1, !26}
!26 = !{%"MemoryManager" zeroinitializer, i32 0}
!27 = !{!"S", %"RefVectorOf1" zeroinitializer, i32 1, !28}
!28 = !{%"BaseRefVectorOf1" zeroinitializer, i32 0}
!29 = !{!"S", %"BaseRefVectorOf1" zeroinitializer, i32 6, !1, !13, !3, !3, !30, !8}
!30 = !{%"XercesLocationPath" zeroinitializer, i32 2}
!31 = !{!"S", %"XercesLocationPath" zeroinitializer, i32 2, !6, !32}
!32 = !{%"RefVectorOf" zeroinitializer, i32 1}
!34 = !{%"XercesStep" zeroinitializer, i32 2}
!35 = !{!"S", %"XercesStep" zeroinitializer, i32 3, !6, !36, !37}
!36 = !{i16 0, i32 0}
!37 = !{%"XercesNodeTest" zeroinitializer, i32 1}
!38 = !{!"S", %"XercesNodeTest" zeroinitializer, i32 3, !6, !36, !39}
!39 = !{%"QName" zeroinitializer, i32 1}
!43 = distinct !{!44, !8}
!44 = !{%"ValueVectorOf" zeroinitializer, i32 1}

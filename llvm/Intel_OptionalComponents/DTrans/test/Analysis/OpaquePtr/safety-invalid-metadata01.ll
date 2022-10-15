; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-safetyanalyzer %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify that the DTrans safety analyzer does not run when there
; are structure types that could not be uniquely resolved due to conflicting
; metadata descriptions for the structure fields. There are locations in the
; DTrans safety analyzer that expect to be able to get the field type of a
; DTransStructure object. Howeer, if the type metadata reader was unable to
; uniquely identify these, it could cause a failure while running the DTrans
; safety analyzer.

; CHECK: DTransSafetyInfo: Type metadata reader did not find structure type metadata or errors were detected in the metadata

%union.anon = type { %struct.anon, [8 x i8] }
%struct.anon = type { void (i32, i32, i32, i32, float)* }
%struct.IconImage = type { i32, i32, i32*, i8*, i32 }

; In the metadata descriptions below, there are conflicting descriptors for
; %struct.anon.

!intel.dtrans.types = !{!12, !13, !14, !15}

!1 = !{%struct.anon zeroinitializer, i32 0}  ; %struct.anon
!2 = !{!"A", i32 8, !3}  ; [8 x i8]
!3 = !{i8 0, i32 0}  ; i8
!4 = !{!"F", i1 false, i32 5, !5, !6, !6, !6, !6, !7}  ; void (i32, i32, i32, i32, float)
!5 = !{!"void", i32 0}  ; void
!6 = !{i32 0, i32 0}  ; i32
!7 = !{float 0.0e+00, i32 0}  ; float
!8 = !{!4, i32 1}  ; void (i32, i32, i32, i32, float)*
!9 = !{%struct.IconImage zeroinitializer, i32 1}  ; %struct.IconImage*
!10 = !{i32 0, i32 1}  ; i32*
!11 = !{i8 0, i32 1}  ; i8*
!12 = !{!"S", %union.anon zeroinitializer, i32 2, !1, !2} ; { %struct.anon, [8 x i8] }
!13 = !{!"S", %struct.anon zeroinitializer, i32 1, !8} ; { void (i32, i32, i32, i32, float)* }
!14 = !{!"S", %struct.anon zeroinitializer, i32 1, !9} ; { %struct.IconImage* }
!15 = !{!"S", %struct.IconImage zeroinitializer, i32 5, !6, !6, !10, !11, !6} ; { i32, i32, i32*, i8*, i32 }


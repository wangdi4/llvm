; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Check that the analysis of a global variable initializer handles a
; GEPOperator getting the address of an array element.

%struct._CoderMapInfo = type { ptr, ptr }

@.str.27.213 = private unnamed_addr constant [4 x i8] c"3FR\00", align 1
@.str.28.214 = private unnamed_addr constant [4 x i8] c"DNG\00", align 1
@.str.29.215 = private unnamed_addr constant [5 x i8] c"8BIM\00", align 1
@.str.30.216 = private unnamed_addr constant [5 x i8] c"META\00", align 1

@CoderMap = internal unnamed_addr constant [2 x %struct._CoderMapInfo] [
  %struct._CoderMapInfo { ptr @.str.27.213, ptr @.str.28.214 },
  %struct._CoderMapInfo { ptr @.str.29.215, ptr @.str.30.216 }]


; CHECK-LABEL: LLVMType: %struct._CoderMapInfo
; CHECK: Safety data: Global instance | Has initializer list | Global array{{ *}}
; CHECK: End LLVMType: %struct._CoderMapInfo

!intel.dtrans.types = !{!2}

!1 = !{i8 0, i32 1}  ; i8*
!2 = !{!"S", %struct._CoderMapInfo zeroinitializer, i32 2, !1, !1} ; { i8*, i8* }

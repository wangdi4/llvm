; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-safetyanalyzer-ir -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-safetyanalyzer-ir -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test of the -dtrans-print-types option of the DTransSafetyAnalyzer pass.
; This case is to verify structures are printed in the same lexical order that
; the original DTrans analysis generated when names with special non-alpha numeric
; characters caused the name to be placed in quotes.

%union._ZTSZ13ReadBlobFloatE3$_0.anon = type { i32 }
%struct._ZTSZ19AcquireResizeFilterE3$_0.anon = type { i32, i32 }
%struct._ZTSZ19AcquireResizeFilterE3$_0.anon.0 = type { float (float)*, double, i32 }
%struct._ZTS10_ErrorInfo._ErrorInfo = type { double, double }
%"struct.std::less" = type { i8 }

define void @test01() {
  %a1 = alloca %union._ZTSZ13ReadBlobFloatE3$_0.anon
  %a2 = alloca %struct._ZTSZ19AcquireResizeFilterE3$_0.anon
  %a3 = alloca %struct._ZTSZ19AcquireResizeFilterE3$_0.anon.0
  %a4 = alloca %struct._ZTS10_ErrorInfo._ErrorInfo
  %a5 = alloca %"struct.std::less"
  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %"struct._ZTSZ19AcquireResizeFilterE3$_0.anon"

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %"struct._ZTSZ19AcquireResizeFilterE3$_0.anon.0"

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %"struct.std::less"

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %"union._ZTSZ13ReadBlobFloatE3$_0.anon"

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS10_ErrorInfo._ErrorInfo

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !3}  ; float (float)
!3 = !{float 0.0e+00, i32 0}  ; float
!4 = !{!2, i32 1}  ; float (float)*
!5 = !{double 0.0e+00, i32 0}  ; double
!6 = !{i8 0, i32 0}  ; i8
!7 = !{!"S", %union._ZTSZ13ReadBlobFloatE3$_0.anon zeroinitializer, i32 1, !1} ; { i32 }
!8 = !{!"S", %struct._ZTSZ19AcquireResizeFilterE3$_0.anon zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct._ZTSZ19AcquireResizeFilterE3$_0.anon.0 zeroinitializer, i32 3, !4, !5, !1} ; { float (float)*, double, i32 }
!10 = !{!"S", %struct._ZTS10_ErrorInfo._ErrorInfo zeroinitializer, i32 2, !5, !5} ; { double, double }
!11 = !{!"S", %"struct.std::less" zeroinitializer, i32 1, !6} ; { i8 }

!intel.dtrans.types = !{!7, !8, !9, !10, !11}

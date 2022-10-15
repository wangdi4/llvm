; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function that forwards parameters to an indirect
; function call.

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.kmpc_loc.0.0.27 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.source.0.0.694 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; The parameters that are forwarded for the callback function cannot be
; checked because that target is unknown, so should be marked as "Unhandled use"
%struct.test01 = type { i32, i32, i32, i64, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %img, i64* "intel_dtrans_func_index"="2" %buf, void (i32*, i32*, i64, %struct.test01*, i64*)* "intel_dtrans_func_index"="3" %func) !intel.dtrans.func.type !11 {
  tail call void @broker(
    %struct.ident_t* @.kmpc_loc.0.0.27,
    i32 6,
    void (i32*, i32*, i64, %struct.test01*, i64*)* %func,
    i64 1,
    %struct.test01* %img,
    i64* %buf
  )
  ret void
}

declare !intel.dtrans.func.type !13 !callback !0 void @broker(%struct.ident_t* "intel_dtrans_func_index"="1" %0, i32 %1, void (i32*, i32*, i64, %struct.test01*, i64*)* "intel_dtrans_func_index"="2" %2, i64, %struct.test01* "intel_dtrans_func_index"="3", i64* "intel_dtrans_func_index"="4")


; This structure should get marked "Address taken" because it is passed to the
; external broker function.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.ident_t
; CHECK: Safety data: Global instance | Has initializer list | Address taken
; CHECK: End LLVMType: %struct.ident_t

; This structure gets marked as "Unhandled use" because the parameter passed to
; the broker function will be forwarded to an indirect call. The analysis
; currently does not support trying to resolve whether the indirect call will
; use the parameter as the expected type.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Unhandled use
; CHECK: End LLVMType: %struct.test01

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i64 3, i64 4, i64 5, i1 false }
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = !{i64 0, i32 1}  ; i64*
!7 = !{!"F", i1 false, i32 5, !8, !9, !9, !4, !5, !6}  ; void (i32*, i32*, i64, %struct.test01*, i64*)
!8 = !{!"void", i32 0}  ; void
!9 = !{i32 0, i32 1}  ; i32*
!10 = !{!7, i32 1}  ; void (i32*, i32*, i64, %struct.test01*, i64*)*
!11 = distinct !{!5, !6, !10}
!12 = !{%struct.ident_t zeroinitializer, i32 1}  ; %struct.ident_t*
!13 = distinct !{!12, !10, !5, !6}
!14 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !2, !2, !2, !2, !3} ; { i32, i32, i32, i32, i8* }
!15 = !{!"S", %struct.test01 zeroinitializer, i32 5, !2, !2, !2, !4, !2} ; { i32, i32, i32, i64, i32 }

!intel.dtrans.types = !{!14, !15}

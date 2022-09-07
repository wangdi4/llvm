; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback function that forwards parameters to an indirect
; function call.

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.kmpc_loc.0.0.27 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.source.0.0.694 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

; The parameters that are forwarded for the callback function cannot be
; checked because that target is unknown, so should be marked as "Unhandled use"
%struct.test01 = type { i32, i32, i32, i64, i32 }
define void @test01(%struct.test01* %img, i64* %buf, void (i32*, i32*, i64, %struct.test01*, i64*)* %func) {
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

declare !callback !0 void @broker(%struct.ident_t* %0, i32 %1, void (i32*, i32*, i64, %struct.test01*, i64*)* %2, i64, %struct.test01*, i64*)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i64 3, i64 4, i64 5, i1 false }

; This structure should get marked "Address taken" because it is passed to the
; external broker function.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.ident_t = type { i32, i32, i32, i32, i8* }
; CHECK: Safety data: Global instance | Has initializer list | Address taken

; This structure gets marked as "Unhandled use" because the parameter passed to
; the broker function will be forwarded to an indirect call. The analysis
; currently does not support trying to resolve whether the indirect call will
; use the parameter as the expected type.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: Unhandled use



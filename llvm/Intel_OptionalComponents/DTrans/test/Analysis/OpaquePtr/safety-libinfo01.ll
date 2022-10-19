; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that the safety analyzer can resolve the safety information for
; functions that are defined using the DTransLibraryInfo class, instead
; of setting "Unhandled use" for the passed structure type.

%struct.ident_t = type { i32, i32, i32, i32, i8* }

@.source.0.0.694 = private constant [22 x i8] c";unknown;unknown;0;0;;"
@.kmpc_loc.0.0 = private global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.gomp_critical_user.var = internal global [8 x i32] zeroinitializer

define void @test_libfunc() {
  %kmpc_global_thread_num = call i32 @__kmpc_global_thread_num(%struct.ident_t* @.kmpc_loc.0.0)
  ret void
}

; This function is intentionally declared without DTrans metadata to force the
; information to come from the DTransLibraryInfo class.
declare i32 @__kmpc_global_thread_num(%struct.ident_t*)

; CHECK: LLVMType: %struct.ident_t
; CHECK: Safety data: Global instance | Has initializer list | Address taken | System object{{ *}}
; CHECK: End LLVMType: %struct.ident_t

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !1, !1, !1, !1, !2} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!3}

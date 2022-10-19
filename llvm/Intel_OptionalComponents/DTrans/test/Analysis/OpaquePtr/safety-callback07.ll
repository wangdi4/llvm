; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test callback used for a LibFunc for OpenMP.

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.test01 = type { i32, i32, i32, i64, i32 }

@.kmpc_loc.0.0.27 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.source.0.0.694 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %img ,i8* "intel_dtrans_func_index"="2" %buf) !intel.dtrans.func.type !6 {
  tail call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(
    %struct.ident_t* @.kmpc_loc.0.0.27, i32 6, void (i32*, i32*, ...)* bitcast
      (void (i32*, i32*, i64, %struct.test01*, i64, i64*, i64, i64)* @GetImageChannelDepth.DIR.OMP.PARALLEL.LOOP.2.split552 to void (i32*, i32*, ...)*),
    i64 1,
    %struct.test01* %img,
    i64 75,
    i8* %buf,
    i64 0,
    i64 48
  )
  ret void
}

define void @GetImageChannelDepth.DIR.OMP.PARALLEL.LOOP.2.split552(i32* "intel_dtrans_func_index"="1" %0, i32* "intel_dtrans_func_index"="2" %1, i64 %2, %struct.test01* "intel_dtrans_func_index"="3" %3, i64 %4, i64* "intel_dtrans_func_index"="4" %5, i64 %6, i64 %7) !intel.dtrans.func.type !9 {
  %load0 = load i32, i32* %0
  %load1 = load i32, i32* %1
  %load3 = load i64, i64* %5
  %use1 = getelementptr %struct.test01, %struct.test01* %3, i64 0, i32 1
  ret void
}

declare !intel.dtrans.func.type !14 !callback !0 void @__kmpc_fork_call(%struct.ident_t* "intel_dtrans_func_index"="1" %0, i32 %1, void (i32*, i32*, ...)* "intel_dtrans_func_index"="2" %2, ...)

; This structure is used by the LibFunc instead of being forwarded to the
; callback function, so should be marked as a "System object."

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.ident_t
; CHECK: Safety data: Global instance | Has initializer list | Address taken | System object
; CHECK: End LLVMType: %struct.ident_t

; This structure is forwarded to the callback function with the expected type,
; and should not get a safety flag.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!6 = distinct !{!5, !3}
!7 = !{i32 0, i32 1}  ; i32*
!8 = !{i64 0, i32 1}  ; i64*
!9 = distinct !{!7, !7, !5, !8}
!10 = !{%struct.ident_t zeroinitializer, i32 1}  ; %struct.ident_t*
!11 = !{!"F", i1 true, i32 2, !12, !7, !7}  ; void (i32*, i32*, ...)
!12 = !{!"void", i32 0}  ; void
!13 = !{!11, i32 1}  ; void (i32*, i32*, ...)*
!14 = distinct !{!10, !13}
!15 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !2, !2, !2, !2, !3} ; { i32, i32, i32, i32, i8* }
!16 = !{!"S", %struct.test01 zeroinitializer, i32 5, !2, !2, !2, !4, !2} ; { i32, i32, i32, i64, i32 }

!intel.dtrans.types = !{!15, !16}

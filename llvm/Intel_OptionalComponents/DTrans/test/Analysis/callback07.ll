; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test callback used for a LibFunc for OpenMP.

; A safe case where types match the expected types.
%struct.ident_t = type { i32, i32, i32, i32, i8* }
%struct.test01 = type { i32, i32, i32, i64, i32 }

@.kmpc_loc.0.0.27 = private unnamed_addr global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.source.0.0.694 = private unnamed_addr constant [22 x i8] c";unknown;unknown;0;0;;"

define void @test01(%struct.test01* %img ,i8* %buf) {
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

define void @GetImageChannelDepth.DIR.OMP.PARALLEL.LOOP.2.split552(i32* %0, i32* %1, i64 %2, %struct.test01* %3, i64 %4, i64* %5, i64 %6, i64 %7) {
  %load0 = load i32, i32* %0
  %load1 = load i32, i32* %1
  %load3 = load i64, i64* %5
  %use1 = getelementptr %struct.test01, %struct.test01* %3, i64 0, i32 1
  ret void
}

declare !callback !0 void @__kmpc_fork_call(%struct.ident_t* %0, i32 %1, void (i32*, i32*, ...)* %2, ...)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}

; This structure is used by the LibFunc instead of being forwarded to the
; callback function, so should be marked as a "System object."

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.ident_t = type { i32, i32, i32, i32, i8* }
; CHECK: Safety data: Global instance | Has initializer list | Address taken | System object

; This structure is forwarded to the callback function with the expected type,
; and should not get a safety flag.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01 = type { i32, i32, i32, i64, i32 }
; CHECK: Safety data: No issues found

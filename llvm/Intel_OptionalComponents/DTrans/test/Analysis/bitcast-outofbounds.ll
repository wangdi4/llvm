; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output 2>&1 | FileCheck %s

; Check that no Bad casting is flagged for %struct.S3, as the GEP accesses
; only a field of the struct.

%struct.S3 = type { i8* }
define dso_local i32 @foo_1(i32* %int_ptr, %struct.S3* nocapture %s3) local_unnamed_addr {
entry:
  %ptr3 = getelementptr inbounds %struct.S3, %struct.S3* %s3, i64 0, i32 0
  %tmp = bitcast i8** %ptr3 to i32**
  store i32* %int_ptr, i32** %tmp, align 8
  ret i32 0
}
; CHECK-LABEL:  LLVMType: %struct.S3 = type { i8* }
; CHECK-NOT:  Safety data:{{.*}}Bad casting{{.*}}

; Check that Bad Casting is flagged for %struct.S5, because the pointer to
; it is bitcast, and %struct.S4, because it is nested inside %struct.S5,
; but not on %struct.S6.

%struct.S4 = type { i32, float }
%struct.S5 = type { i32, i32, %struct.S4 }
%struct.S6 = type { i32, i32, %struct.S5 }

define dso_local i32** @foo_2(%struct.S6* nocapture %s6) {
  %ptr1 = getelementptr inbounds %struct.S6, %struct.S6* %s6, i64 0, i32 2
  %tmp = bitcast %struct.S5* %ptr1 to i32**
  ret i32** %tmp
}

; CHECK-LABEL: LLVMType: %struct.S4 = type { i32, float }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}
; CHECK: LLVMType: %struct.S5 = type { i32, i32, %struct.S4 }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}
; CHECK: LLVMType: %struct.S6 = type { i32, i32, %struct.S5 }
; CHECK-NOT:  Safety data:{{.*}}Bad casting{{.*}}

; Cast of arbitrary i8* to struct pointer.
; Check that only %struct.S8 is flagged as Bad casting, and not
; %struct.S7, as %struct.S7 has only a field accessed.

%struct.S7 = type { i32, i8 }
%struct.S8 = type { i32, i32 }
@p.foo_3 = internal unnamed_addr global %struct.S7 zeroinitializer

define %struct.S8* @foo_3() {
  %s = bitcast i8* getelementptr( %struct.S7, %struct.S7* @p.foo_3,
                                  i64 0, i32 1) to %struct.S8*
  ret %struct.S8* %s
}

; CHECK-LABEL: LLVMType: %struct.S7 = type { i32, i8 }
; CHECK-NOT: Safety data:{{.*}}Bad casting
; CHECK: LLVMType: %struct.S8 = type { i32, i32 }
; CHECK: Safety data: Bad casting

%struct.S9 = type { i32, float }
%struct.SA = type { i32, i32, %struct.S9 }
@p.foo_4 = internal unnamed_addr global %struct.S7 zeroinitializer

; Check that Bad Casting is flagged for %struct.SA, because it is the
; target of a bitcast, and %struct.S9, because it is nested inside %struct.SA.

define dso_local %struct.SA* @foo_4() {
  %s = bitcast i8* getelementptr( %struct.S7, %struct.S7* @p.foo_4,
                                  i64 0, i32 1) to %struct.SA*
  ret %struct.SA* %s
}

; CHECK-LABEL: LLVMType: %struct.S9 = type { i32, float }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}
; CHECK: LLVMType: %struct.SA = type { i32, i32, %struct.S9 }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}

; Check that %struct.SB and %struct.SD get flagged as Bad casting because
; they are the source and destination of a Bad casting bitcast. But
; %struct.SD is not, as it is only enclosing %struct.SC.

%struct.SB = type { i32, float }
%struct.SC = type { i32, %struct.SB, i32 }
%struct.SD = type { float, i32 }
@p.foo_5 = internal unnamed_addr global %struct.SC zeroinitializer

define dso_local %struct.SD* @foo_5() {
  %s = bitcast %struct.SB* getelementptr( %struct.SC, %struct.SC* @p.foo_5,
                                  i64 0, i32 1) to %struct.SD*
  ret %struct.SD* %s
}

; CHECK-LABEL: LLVMType: %struct.SB = type { i32, float }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}
; CHECK: LLVMType: %struct.SC = type { i32, %struct.SB, i32 }
; CHECK-NOT:  Safety data:{{.*}}Bad casting{{.*}}
; CHECK: LLVMType: %struct.SD = type { float, i32 }
; CHECK:  Safety data:{{.*}}Bad casting{{.*}}

; Load from a GEP-based pointer with the incorrect type
; Check that no Bad casting is flagged for %struct.test08, as only a field
; of the struct is accessed.

%struct.SE = type { i32, i32 }
define void @foo_6(%struct.SE* %p) {
  %py = getelementptr %struct.SE, %struct.SE* %p, i64 0, i32 1
  %p2 = bitcast i32* %py to i16*
  %y2 = load i16, i16* %p2
  ret void
}

; CHECK-LABEL: LLVMType: %struct.SE = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}


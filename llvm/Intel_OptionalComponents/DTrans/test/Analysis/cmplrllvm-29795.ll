; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

%struct.op = type { %struct.op*, %struct.op*, i64, i16, i8, i8 }

define void @test01(%struct.op** %in) {
    %v = load %struct.op*, %struct.op** %in
    %f = freeze %struct.op* %v
    ret void
}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.op = type { %struct.op*, %struct.op*, i64, i16, i8, i8 }
; CHECK: Safety data: Unhandled use

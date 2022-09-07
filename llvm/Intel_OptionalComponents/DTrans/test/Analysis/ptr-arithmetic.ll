; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test validates correct handling of various pointer arithmetic idioms
; by the DTrans analysis.

; Subtracting two pointers of like types yields a safe offset.
%struct.test01 = type { i32, i32 }
define void @test01(%struct.test01* %p1, %struct.test01* %p2) {
  %t1 = ptrtoint %struct.test01* %p1 to i64
  %t2 = ptrtoint %struct.test01* %p2 to i64
  %offset = sub i64 %t1, %t2
  ret void
}

; CHECK: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Subtracting two pointers of unlike types is an unhandled use.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i32, i32, i32 }
define void @test02(%struct.test02.a* %p1, %struct.test02.b* %p2) {
  %t1 = ptrtoint %struct.test02.a* %p1 to i64
  %t2 = ptrtoint %struct.test02.b* %p2 to i64
  %offset = sub i64 %t1, %t2
  ret void
}

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test02.b = type { i32, i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a scalar from a pointer is an unhandled use.
%struct.test03 = type { i32, i32 }
define void @test03(%struct.test03* %p, i64 %n) {
  %t = ptrtoint %struct.test03* %p to i64
  %offset = sub i64 %t, %n
  ret void
}

; CHECK: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a pointer from a scalar is an unhandled use.
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %p, i64 %n) {
  %t = ptrtoint %struct.test04* %p to i64
  %offset = sub i64 %n, %t
  ret void
}

; CHECK: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Unhandled use

; Subtracting a pointers to elements is bad pointer manipulation.
%struct.test05 = type { i32, i32, i32, i32 }
define void @test05(%struct.test05* %p) {
  %p1 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 1
  %p2 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 3
  %t1 = ptrtoint i32* %p1 to i64
  %t2 = ptrtoint i32* %p2 to i64
  %offset = sub i64 %t2, %t1
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; The expected use of a pointer sub is to divide by the size of the struct.
%struct.test06 = type { i32, i32, i32, i32 }
define void @test06(%struct.test06* %p1, %struct.test06* %p2) {
  %t1 = ptrtoint %struct.test06* %p1 to i64
  %t2 = ptrtoint %struct.test06* %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = sdiv i64 %offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test06 = type { i32, i32, i32, i32 }
; CHECK: Safety data: No issues found

; Dividing using an udiv should also be acceptable.
%struct.test07 = type { i32, i32, i32, i32 }
define void @test07(%struct.test07* %p1, %struct.test07* %p2) {
  %t1 = ptrtoint %struct.test07* %p1 to i64
  %t2 = ptrtoint %struct.test07* %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = udiv i64 %offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test07 = type { i32, i32, i32, i32 }
; CHECK: Safety data: No issues found

; Dividing by some value that is not the size of the struct should be
; flagged as bad pointer manipulation.
%struct.test08 = type { i32, i32, i32, i32 }
define void @test08(%struct.test08* %p1, %struct.test08* %p2) {
  %t1 = ptrtoint %struct.test08* %p1 to i64
  %t2 = ptrtoint %struct.test08* %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = sdiv i64 %offset, 24
  ret void
}

; CHECK: LLVMType: %struct.test08 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Non-div uses of a pointer sub should be flagged as bad pointer manipulation.
%struct.test09 = type { i32, i32, i32, i32 }
define void @test09(%struct.test09* %p1, %struct.test09* %p2) {
  %t1 = ptrtoint %struct.test09* %p1 to i64
  %t2 = ptrtoint %struct.test09* %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = mul i64 %offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test09 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Test the case where the divide is separated from the sub by a PHI node.
%struct.test10 = type { i32, i32, i32, i32 }
define void @test10(%struct.test10* %p1, %struct.test10* %p2, i64 %v) {
entry:
  br i1 undef, label %sub, label %div

sub:
  %t1 = ptrtoint %struct.test10* %p1 to i64
  %t2 = ptrtoint %struct.test10* %p2 to i64
  %offset = sub i64 %t2, %t1
  br label %div

div:
  %pn_offset = phi i64 [%v, %entry], [%offset, %sub]
  %idx_offset = sdiv i64 %pn_offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test10 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Test the case where the divide is separated from the sub by a select.
%struct.test11 = type { i32, i32, i32, i32 }
define void @test11(%struct.test11* %p1, %struct.test11* %p2, i64 %v) {
  %t1 = ptrtoint %struct.test11* %p1 to i64
  %t2 = ptrtoint %struct.test11* %p2 to i64
  %offset = sub i64 %t2, %t1
  %sel_offset = select i1 undef, i64 %v, i64 %offset
  %idx_offset = sdiv i64 %sel_offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test11 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Subtraction of pointers-to-pointers doesn't depend on the structure size.
%struct.test12 = type { i32, i32, i32, i32 }
define void @test12(%struct.test12** %p1, %struct.test12** %p2) {
  %t1 = ptrtoint %struct.test12** %p1 to i64
  %t2 = ptrtoint %struct.test12** %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = sdiv i64 %offset, 24
  ret void
}

; CHECK: LLVMType: %struct.test12 = type { i32, i32, i32, i32 }
; CHECK: Safety data: No issues found

; Test that bad pointer manipulation is set for subtraction results that
; have both a div and a non-div use.
%struct.test13 = type { i32, i32, i32, i32 }
define void @test13(%struct.test13* %p1, %struct.test13* %p2, i64 %v) {
  %t1 = ptrtoint %struct.test13* %p1 to i64
  %t2 = ptrtoint %struct.test13* %p2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = sdiv i64 %offset, 16
  %sum = add i64 %offset, 8
  ret void
}

; CHECK: LLVMType: %struct.test13 = type { i32, i32, i32, i32 }
; CHECK: Safety data: Bad pointer manipulation

; Our subtraction analysis skips this case on the assumption that it is
; handled by the PHI analysis.
%struct.test14.a = type { i32, i32, i32, i32 }
%struct.test14.b = type { i64, i64 }
define void @test14(%struct.test14.a* %p1, %struct.test14.b* %p2,
                    %struct.test14.a* %p3, %struct.test14.b* %p4) {
entry:
  br i1 undef, label %true, label %false

true:
  %p5 = bitcast %struct.test14.a* %p1 to i8*
  %p6 = bitcast %struct.test14.a* %p3 to i8*
  br label %exit

false:
  %p7 = bitcast %struct.test14.b* %p2 to i8*
  %p8 = bitcast %struct.test14.b* %p4 to i8*
  br label %exit

exit:
  %phi1 = phi i8* [%p5, %true], [%p7, %false]
  %phi2 = phi i8* [%p6, %true], [%p8, %false]
  %t1 = ptrtoint i8* %phi1 to i64
  %t2 = ptrtoint i8* %phi2 to i64
  %offset = sub i64 %t2, %t1
  %idx_offset = sdiv i64 %offset, 16
  ret void
}

; CHECK: LLVMType: %struct.test14.a = type { i32, i32, i32, i32 }
; CHECK: Safety data: Unsafe pointer merge
; CHECK: LLVMType: %struct.test14.b = type { i64, i64 }
; CHECK: Safety data: Unsafe pointer merge

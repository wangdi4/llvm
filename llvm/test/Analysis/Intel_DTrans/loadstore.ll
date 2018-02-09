; RUN: opt < %s -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies correct handling of load and store instructions by the
; DTransAnalysis and verifies that real legality checks are correctly
; identified while patterns which do not present any legality concerns are not
; incorrectly marked as unsafe.

; Load and store analysis is not yet implemented, so most of these tests are
; reporting Unhandled use. The actual safety data should be something else.

; Store of pointer to struct cast as an pointer-sized integer.
%struct.test01.a = type { i32, i32 }
%struct.test01.b = type { i32, i32, %struct.test01.a* }
define void @test1( %struct.test01.a* %sa, %struct.test01.b* %sb ) {
  %a.as.i = ptrtoint %struct.test01.a* %sa to i64
  %pb.a = getelementptr %struct.test01.b, %struct.test01.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test01.a** %pb.a to i64*
  store i64 %a.as.i, i64* %pb.a.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test01.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test01.b = type { i32, i32, %struct.test01.a* }
; CHECK: Safety data: Unhandled use

; Mismatched store of pointer to struct cast as an pointer-sized integer.
%struct.test02.a = type { i32, i32 }
%struct.test02.b = type { i32, i32, %struct.test02.a* }
define void @test2( %struct.test02.b* %sb ) {
  %b.as.i = ptrtoint %struct.test02.b* %sb to i64
  %pb.a = getelementptr %struct.test02.b, %struct.test02.b* %sb, i64 0, i32 2
  %pb.a.as.pi = bitcast %struct.test02.a** %pb.a to i64*
  store i64 %b.as.i, i64* %pb.a.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test02.a = type { i32, i32 }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test02.b = type { i32, i32, %struct.test02.a* }
; CHECK: Safety data: Unhandled use

; Copy of pointer to struct from one aggregate to another via GEP, bitcast and
; load/store.
%struct.test03.a = type { i32, %struct.test03.a* }
%struct.test03.b = type { i32, i32, %struct.test03.a* }
define void @test3( %struct.test03.a* %sa, %struct.test03.b* %sb ) {
  %ppaa = getelementptr %struct.test03.a, %struct.test03.a* %sa, i64 0, i32 1
  %ppaa.as.pi = bitcast %struct.test03.a** %ppaa to i64*
  %paa.as.i = load i64, i64* %ppaa.as.pi
  %ppba = getelementptr %struct.test03.b, %struct.test03.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test03.a** %ppba to i64*
  store i64 %paa.as.i, i64* %ppba.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test03.a = type { i32, %struct.test03.a* }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test03.b = type { i32, i32, %struct.test03.a* }
; CHECK: Safety data: Unhandled use

; Copy of pointer to struct from one aggregate to another via GEP, bitcast and
; load/store.
%struct.test04.a = type { i32, %struct.test04.b* }
%struct.test04.b = type { i32, i32, %struct.test04.a* }
define void @test4( %struct.test04.a* %sa, %struct.test04.b* %sb ) {
  %ppab = getelementptr %struct.test04.a, %struct.test04.a* %sa, i64 0, i32 1
  %ppab.as.pi = bitcast %struct.test04.b** %ppab to i64*
  %pab.as.i = load i64, i64* %ppab.as.pi
  %ppba = getelementptr %struct.test04.b, %struct.test04.b* %sb, i64 0, i32 2
  %ppba.as.pi = bitcast %struct.test04.a** %ppba to i64*
  store i64 %pab.as.i, i64* %ppba.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test04.a = type { i32, %struct.test04.b* }
; CHECK: Safety data: Unhandled use
; CHECK: LLVMType: %struct.test04.b = type { i32, i32, %struct.test04.a* }
; CHECK: Safety data: Unhandled use

; Cast of non-instruction to struct pointer.
; This case currently gets flagged because it uses an operator GEP.
; This use actually meets the pointer copy idiom and so is safe.
%struct.test05 = type { i32, %struct.test05* }
@p.test05 = external global %struct.test05
define void @test5(%struct.test05* %pa.arg) {
  %ppa.as.pi = bitcast %struct.test05** getelementptr(
                                          %struct.test05,
                                          %struct.test05* @p.test05,
                                          i64 0, i32 1) to i64*
  %pa.as.i = load i64, i64* %ppa.as.pi
  %ppa.arg = getelementptr %struct.test05, %struct.test05* %pa.arg, i64 0, i32 1
  %ppa.arg.as.pi = bitcast %struct.test05** %ppa.arg to i64*
  store i64 %pa.as.i, i64* %ppa.arg.as.pi
  ret void
}

; CHECK: LLVMType: %struct.test05 = type { i32, %struct.test05* }
; CHECK: Safety data: Unhandled use

; Load from aliased pointer passed across PHI and Select
%struct.test06 = type { i32, i32 }
define void @test31(%struct.test06* %p1, i32* %p2) {
entry:
  %t1 = bitcast %struct.test06* %p1 to i8*
  br label %loop

loop:
  %t2 = phi i8* [%t1, %entry], [%t4, %back]
  br i1 undef, label %end, label %back

back:
  %t3 = bitcast i32* %p2 to i8*
  %t4 = select i1 undef, i8* %t2, i8* %t3
  br label %loop

end:
  %t5 = load i8, i8* %t2
  ret void
}

; CHECK: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Unhandled use


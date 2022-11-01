; RUN: opt < %s -passes='require<anders-aa>,function(gvn)' -aa-pipeline=basic-aa,anders-aa -S | FileCheck %s

; Test where static global is address taken, but can be determined to not be modified by the routine
; doesnotmodX, even though that routine calls a routine which modifies memory.
; This test is to check set propagation across routines.

@X = internal global i32 4		; <i32*>

define i32 @test(i32* %P) {
; CHECK:      @test
; CHECK-NEXT:  %V = alloca i32, align 4
; CHECK-NEXT: store i32 12, i32* @X
; CHECK-NEXT: call void @doesnotmodX(i32* %V)
; CHECK-NEXT: ret i32 12
    %V = alloca i32, align 4
	store i32 12, i32* @X
	call void @doesnotmodX(i32* %V)
	%1 = load i32, i32* @X		; <i32>
	ret i32 %1
}

define void @doesnotmodX(i32* %P) {
entry:
  call void @stilldoesnotmodX(i32* %P)
  ret void
}

define void @stilldoesnotmodX(i32* %P) {
entry:
  %P.addr = alloca i32*, align 8
  store i32* %P, i32** %P.addr, align 8
  store i32* null, i32** %P.addr, align 8
  ret void
}

define void @doesModX(i32* %anX) {
  %anX.addr = alloca i32*, align 8
  store i32* %anX, i32** %anX.addr, align 8
  %1 = load i32*, i32** %anX.addr, align 8
  store i32 1, i32* %1, align 4
  ret void
}

define void @test_addr_of() {
entry:
  call void @doesModX(i32* @X)
  ret void
}

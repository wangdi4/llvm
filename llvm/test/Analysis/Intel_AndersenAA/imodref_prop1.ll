; RUN: opt < %s -passes='require<anders-aa>,function(gvn)' -aa-pipeline=basic-aa,anders-aa -S | FileCheck %s

; Test where static global is address taken, but can be determined to not be modified by the routine
; doesnotmodX, even though that routine calls a routine which modifies memory.
; This test is to check set propagation across routines.

@X = internal global i32 4		; <ptr>

define i32 @test(ptr %P) {
; CHECK:      @test
; CHECK-NEXT:  %V = alloca i32, align 4
; CHECK-NEXT: store i32 12, ptr @X
; CHECK-NEXT: call void @doesnotmodX(ptr %V)
; CHECK-NEXT: ret i32 12
    %V = alloca i32, align 4
	store i32 12, ptr @X
	call void @doesnotmodX(ptr %V)
	%1 = load i32, ptr @X		; <i32>
	ret i32 %1
}

define void @doesnotmodX(ptr %P) {
entry:
  call void @stilldoesnotmodX(ptr %P)
  ret void
}

define void @stilldoesnotmodX(ptr %P) {
entry:
  %P.addr = alloca ptr, align 8
  store ptr %P, ptr %P.addr, align 8
  store ptr null, ptr %P.addr, align 8
  ret void
}

define void @doesModX(ptr %anX) {
  %anX.addr = alloca ptr, align 8
  store ptr %anX, ptr %anX.addr, align 8
  %1 = load ptr, ptr %anX.addr, align 8
  store i32 1, ptr %1, align 4
  ret void
}

define void @test_addr_of() {
entry:
  call void @doesModX(ptr @X)
  ret void
}

; RUN: opt < %s -basic-aa -anders-aa -gvn -S | FileCheck %s

; Test where static global is address taken, but can be determined to not be modified by the routine
; doesnotmodX, even though that routine modifies memory based on a pointer passed in that does not
; alias X.

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
	%1 = load i32, i32* @X
	ret i32 %1
}

define internal void @doesnotmodX(i32* %P) {
  %P.addr = alloca i32*, align 8
  store i32* %P, i32** %P.addr, align 8
  store i32* null, i32** %P.addr, align 8
  ret void
}


define internal void @doesModX(i32* %anX) {
  %anX.addr = alloca i32*, align 8
  store i32* %anX, i32** %anX.addr, align 8
  %1 = load i32*, i32** %anX.addr, align 8
  store i32 1, i32* %1, align 4
  ret void
}

define void @test_addr_of() {
  call void @doesModX(i32* @X)
  ret void
}

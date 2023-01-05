; RUN: opt < %s -passes='require<anders-aa>,function(gvn)' -aa-pipeline=basic-aa,anders-aa -S | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(gvn)' -aa-pipeline=basic-aa,anders-aa -anders-mod-ref-before-inl=true -S | FileCheck %s --check-prefix=CHECK-BEFOREINL

; Test where static global is address taken, but can be determined to not be modified by the routine
; doesnotmodX, even though that routine modifies memory.

@X = internal global i32 4		; <i32*>
@Y = internal global i32 4		; <i32*>

define i32 @test(i32* %P) {
; CHECK:      @test
; CHECK-NEXT:  %V = alloca i32, align 4
; CHECK-NEXT: store i32 12, i32* @X
; CHECK-NEXT: call void @doesnotmodX()
; CHECK-NEXT: ret i32 12
; CHECK-BEFOREINL:      @test
; CHECK-BEFOREINL-NEXT:  %V = alloca i32, align 4
; CHECK-BEFOREINL-NEXT: store i32 12, i32* @X
; CHECK-BEFOREINL-NEXT: call void @doesnotmodX()
; CHECK-BEFOREINL-NEXT:  %1 = load i32, i32* @X
; CHECK-BEFOREINL-NEXT: ret i32 %1
    %V = alloca i32, align 4
	store i32 12, i32* @X
	call void @doesnotmodX()
	%1 = load i32, i32* @X		; <i32>
	ret i32 %1
}

define void @doesnotmodX() {
  store i32 99, i32* @Y
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
  call void @doesModX(i32* @X)
  ret void
}

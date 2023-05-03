; RUN: opt < %s -passes='require<anders-aa>,function(gvn)' -aa-pipeline=basic-aa,anders-aa -S | FileCheck %s

; Test where external global could be modified by call to verify mod/ref set
; collection treats the set as BOTTOM. This case should not get transformed.

@X = internal global i32 4		; <ptr>

define i32 @test(ptr %P) {
; CHECK:      @test
; CHECK-NEXT:  %V = alloca i32, align 4
; CHECK-NEXT: store i32 12, ptr @X
; CHECK-NEXT: call void @mayModX()
; CHECK-NEXT: %1 = load i32, ptr @X
    %V = alloca i32, align 4
	store i32 12, ptr @X
	call void @mayModX()
	%1 = load i32, ptr @X
	ret i32 %1
}

declare void @mayModX()


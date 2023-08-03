; This test verifies that GlobalModRef shouldn't be conservative
; when an internal function is called through callback calls.
; GlobalModRef should be able to tell that @doesnotmodX() call
; doesn't change value of @X.

; RUN: opt < %s -enable-unsafe-globalsmodref-alias-results=true -aa-pipeline=basic-aa,globals-aa -passes="require<globals-aa>,function(gvn)" -S | FileCheck %s

@X = internal global i32 4
@Y = internal global i32 4

define i32 @test(ptr %P) {
; CHECK:      @test
; CHECK-NEXT: store i32 12, ptr @X
; CHECK-NEXT: call void @doesnotmodX()
; CHECK-NEXT: ret i32 12
  store i32 12, ptr @X
  call void @doesnotmodX()
  %V = load i32, ptr @X
  ret i32 %V
}

define internal void @doesnotmodX() {
  store i32 10, ptr @Y
  ret void
}

define internal void @foo() {
  call void (i32, ptr, ...) @broker(i32 3, ptr @bar, i32 10)
  ret void
}

define internal void @bar(i32 %i) {
  ret void
}

declare !callback !0 void @broker(i32, ptr, ...)

!0 = !{!1}
!1 = !{i64 1, i64 -1, i1 true}

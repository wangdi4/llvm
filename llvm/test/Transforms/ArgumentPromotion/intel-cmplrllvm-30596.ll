; RUN: opt < %s -argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s
; RUN: opt < %s -passes=argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s

; CHECK-30596: Check that the argument of @foo was promoted even though
; its value was homed.

; CHECK-LABEL: define internal i32 @foo(i32 %grid.addr.0.val) {
; CHECK-NEXT: ret i32 %grid.addr.0.val
; CHECK-NEXT: }

; CHECK-LABEL: define dso_local i32 @b() {
; CHECK-NEXT: %mylocal.addr = alloca i32, align 8
; CHECK-NEXT: store i32 5, i32* %mylocal.addr, align 8
; CHECK-NEXT: %mylocal.addr.val = load i32, i32* %mylocal.addr, align 8
; CHECK-NEXT: %t0 = call i32 @foo(i32 %mylocal.addr.val)
; CHECK-NEXT: ret i32 %t0
; CHECK-NEXT: }

define internal i32 @foo(i32* %grid.addr) {
  %grid.addr.addr = alloca i32*, align 8
  store i32* %grid.addr, i32** %grid.addr.addr, align 8
  %grid.addr.value = load i32*, i32** %grid.addr.addr, align 8
  %grid.addr.rv = load i32, i32* %grid.addr.value, align 8
  ret i32 %grid.addr.rv;
}

define dso_local i32 @b() {
  %mylocal.addr = alloca i32, align 8;
  store i32 5, i32* %mylocal.addr, align 8
  %t0 = call i32 @foo(i32* %mylocal.addr)
  ret i32 %t0
}

; RUN: opt < %s -argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s
; RUN: opt < %s -passes=argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s

; Check that the argument of @foo was not promoted because the unique load is bitcast to
; a different type.

; CHECK-LABEL: define internal i32 @foo(i32* %grid.addr) {
; CHECK: %grid.addr.addr = alloca i32*, align 8
; CHECK: store i32* %grid.addr, i32** %grid.addr.addr, align 8
; CHECK: %bc = bitcast i32** %grid.addr.addr to i32* 
; CHECK: %grid.addr.value = load i32, i32* %bc, align 8
; CHECK: ret i32 %grid.addr.value
; CHECK: }

; CHECK-LABEL: define dso_local i32 @b() {
; CHECK: %mylocal.addr = alloca i32, align 8
; CHECK:   store i32 5, i32* %mylocal.addr, align 8
; CHECK:  %t0 = call i32 @foo(i32* %mylocal.addr)
; CHECK:  ret i32 %t0
; CHECK: }

define internal i32 @foo(i32* %grid.addr) {
  %grid.addr.addr = alloca i32*, align 8
  store i32* %grid.addr, i32** %grid.addr.addr, align 8
  %bc = bitcast i32** %grid.addr.addr to i32* 
  %grid.addr.value = load i32, i32* %bc, align 8
  ret i32 %grid.addr.value
}

define dso_local i32 @b() {
  %mylocal.addr = alloca i32, align 8
  store i32 5, i32* %mylocal.addr, align 8
  %t0 = call i32 @foo(i32* %mylocal.addr)
  ret i32 %t0
}


; RUN: opt < %s -opaque-pointers -argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=argpromotion -argpro-force-remove-homed-arguments -S | FileCheck %s

; Check that the argument of @foo was not promoted because the unique load is virtually bitcast
; to a different type.

; CHECK-LABEL: define internal i32 @foo(ptr %grid.addr) {
; CHECK: %grid.addr.addr = alloca ptr, align 8
; CHECK: store ptr %grid.addr, ptr %grid.addr.addr, align 8
; CHECK: %grid.addr.value = load i32, ptr %grid.addr.addr, align 8
; CHECK: ret i32 %grid.addr.value
; CHECK: }

; CHECK-LABEL: define dso_local i32 @b() {
; CHECK: %mylocal.addr = alloca i32, align 8
; CHECK: store i32 5, ptr %mylocal.addr, align 8
; CHECK: %t0 = call i32 @foo(ptr %mylocal.addr)
; CHECK: ret i32 %t0
; CHECK: }

define internal i32 @foo(ptr %grid.addr) {
  %grid.addr.addr = alloca ptr, align 8
  store ptr %grid.addr, ptr %grid.addr.addr, align 8
  %grid.addr.value = load i32, ptr %grid.addr.addr, align 8
  ret i32 %grid.addr.value
}

define dso_local i32 @b() {
  %mylocal.addr = alloca i32, align 8
  store i32 5, ptr %mylocal.addr, align 8
  %t0 = call i32 @foo(ptr %mylocal.addr)
  ret i32 %t0
}


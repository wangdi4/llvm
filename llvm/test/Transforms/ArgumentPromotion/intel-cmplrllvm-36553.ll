; RUN: opt < %s -opaque-pointers -argpromotion -S | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes=argpromotion -S | FileCheck %s

; Check that the first argument of @foo was not promoted because there was a
; mismatch between the formal and actual second arguments, but that the first
; argument of @goo was promoted, because there was no such mismatch.

; CHECK: define internal i32 @goo(i32 %grid.addr.0.val, i1 %mybool)
; CHECK: define internal i32 @foo(ptr %grid.addr, i1 %mybool)

@global0 = internal global i32 zeroinitializer, align 8;
@global1 = internal global i32 zeroinitializer, align 8;

define internal i32 @goo(ptr %grid.addr, i1 %mybool) {
  %grid.addr.value = load i32, ptr %grid.addr, align 8
  %rv = add i32 %grid.addr.value, 5
  %sel = select i1 %mybool, i32 %rv, i32 7
  ret i32 %sel;
}

define dso_local i32 @c() {
  %mylocal.addr = alloca i1, align 1
  store i8 1, ptr %mylocal.addr, align 1
  %t0 = call i32 @goo(ptr @global0, i1 0)
  %myi1val = load i1, ptr %mylocal.addr, align 1
  %t1 = call i32 @goo(ptr @global1, i1 %myi1val)
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}

define internal i32 @foo(ptr %grid.addr, i1 %mybool) {
  %grid.addr.value = load i32, ptr %grid.addr, align 8
  %rv = add i32 %grid.addr.value, 5
  %sel = select i1 %mybool, i32 %rv, i32 7
  ret i32 %sel;
}

define dso_local i32 @b() {
  %mylocal.addr = alloca i8, align 8
  store i8 1, ptr %mylocal.addr, align 8
  %t0 = call i32 @foo(ptr @global0, i8 0)
  %myi8val = load i8, ptr %mylocal.addr, align 1
  %t1 = call i32 @foo(ptr @global1, i8 %myi8val)
  %t2 = add i32 %t0, %t1
  ret i32 %t2
}


